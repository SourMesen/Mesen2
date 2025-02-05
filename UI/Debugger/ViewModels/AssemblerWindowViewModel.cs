using Avalonia.Controls;
using Mesen.Config;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.Windows;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels
{
	public class AssemblerWindowViewModel : DisposableViewModel
	{
		public AssemblerConfig Config { get; }

		[Reactive] public string Code { get; set; } = "";
		[Reactive] public string ByteCodeView { get; set; } = "";
		[Reactive] public int StartAddress { get; set; }
		[Reactive] public int BytesUsed { get; set; }
		
		[Reactive] public bool HasWarning { get; set; }
		[Reactive] public bool IsIdentical { get; set; }
		[Reactive] public bool OriginalSizeExceeded { get; set; }
		[Reactive] public bool MaxSizeExceeded { get; set; }

		[Reactive] public bool OkEnabled { get; set; } = false;
		[Reactive] public List<AssemblerError> Errors { get; set; } = new List<AssemblerError>();

		[Reactive] public List<ContextMenuAction> FileMenuActions { get; private set; } = new();
		[Reactive] public List<ContextMenuAction> OptionsMenuActions { get; private set; } = new();

		public CpuType CpuType { get; }
		private List<byte> _bytes = new();

		public int? MaxAddress { get; }

		private int _originalAddress = -1;
		private byte[] _originalCode = Array.Empty<byte>();
		[Reactive] public int OriginalByteCount { get; private set; } = 0;

		[Obsolete("For designer only")]
		public AssemblerWindowViewModel() : this(CpuType.Snes) { }

		public AssemblerWindowViewModel(CpuType cpuType)
		{
			Config = ConfigManager.Config.Debug.Assembler;
			CpuType = cpuType;

			if(Design.IsDesignMode) {
				return;
			}

			MaxAddress = DebugApi.GetMemorySize(CpuType.ToMemoryType()) - 1;

			AddDisposable(this.WhenAnyValue(x => x.Code, x => x.StartAddress).Subscribe(_ => {
				UpdateAssembly(Code);
			}));
		}

		public void InitMenu(Window wnd)
		{
			FileMenuActions = AddDisposables(new List<ContextMenuAction>() {
				SaveRomActionHelper.GetSaveRomAction(wnd),
				SaveRomActionHelper.GetSaveRomAsAction(wnd),
				SaveRomActionHelper.GetSaveEditsAsIpsAction(wnd),
				new ContextMenuSeparator(),
				new ContextMenuAction() {
					ActionType = ActionType.Exit,
					OnClick = () => wnd.Close()
				}
			});

			OptionsMenuActions = AddDisposables(new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.OpenDebugSettings,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.OpenDebugSettings),
					OnClick = () => DebuggerConfigWindow.Open(DebugConfigWindowTab.FontAndColors, wnd)
				}
			});
		}

		public void InitEditCode(int address, string code, int byteCount)
		{
			OriginalByteCount = byteCount;
			_originalAddress = address;

			using var delayNotifs = DelayChangeNotifications();
			StartAddress = address;
			Code = code;

			if(OriginalByteCount > 0) {
				_originalCode = DebugApi.GetMemoryValues(CpuType.ToMemoryType(), (uint)StartAddress, (uint)(StartAddress + OriginalByteCount - 1));
			}
		}

		private void UpdateAssembly(string code)
		{
			string[] codeLines = code.Replace("\r", "").Split('\n').Select(x => x.Trim()).ToArray();
			short[] byteCode = DebugApi.AssembleCode(CpuType, string.Join('\n', codeLines), (uint)StartAddress);
			List<AssemblerError> errorList = new List<AssemblerError>();
			List<byte> convertedByteCode = new List<byte>(byteCode.Length);
			StringBuilder sb = new StringBuilder();
			int line = 1;
			for(int i = 0; i < byteCode.Length; i++) {
				short s = byteCode[i];
				if(s >= 0) {
					convertedByteCode.Add((byte)s);
					sb.Append(s.ToString("X2") + " ");
				} else if(s == (int)AssemblerSpecialCodes.EndOfLine) {
					line++;
					if(line <= codeLines.Length) {
						sb.Append(Environment.NewLine);
					}
				} else if(s < (int)AssemblerSpecialCodes.EndOfLine) {
					string message = "unknown error";
					switch((AssemblerSpecialCodes)s) {
						case AssemblerSpecialCodes.ParsingError: message = "Invalid syntax"; break;
						case AssemblerSpecialCodes.OutOfRangeJump: message = "Relative jump is out of range (-128 to 127)"; break;
						case AssemblerSpecialCodes.LabelRedefinition: message = "Cannot redefine an existing label"; break;
						case AssemblerSpecialCodes.MissingOperand: message = "Operand is missing"; break;
						case AssemblerSpecialCodes.OperandOutOfRange: message = "Operand is out of range (invalid value)"; break;
						case AssemblerSpecialCodes.InvalidHex: message = "Hexadecimal string is invalid"; break;
						case AssemblerSpecialCodes.InvalidSpaces: message = "Operand cannot contain spaces"; break;
						case AssemblerSpecialCodes.TrailingText: message = "Invalid text trailing at the end of line"; break;
						case AssemblerSpecialCodes.UnknownLabel: message = "Unknown label"; break;
						case AssemblerSpecialCodes.InvalidInstruction: message = "Invalid instruction"; break;
						case AssemblerSpecialCodes.InvalidBinaryValue: message = "Invalid binary value"; break;
						case AssemblerSpecialCodes.InvalidOperands: message = "Invalid operands for instruction"; break;
						case AssemblerSpecialCodes.InvalidLabel: message = "Invalid label name"; break;
					}
					errorList.Add(new AssemblerError() { Message = message + " - " + codeLines[line - 1], LineNumber = line });

					sb.Append("<error: " + message + ">");
					if(i + 1 < byteCode.Length) {
						sb.Append(Environment.NewLine);
					}

					line++;
				}
			}

			BytesUsed = convertedByteCode.Count;
			IsIdentical = MatchesOriginalCode(convertedByteCode);
			OriginalSizeExceeded = BytesUsed > OriginalByteCount;
			MaxSizeExceeded = StartAddress + BytesUsed - 1 > MaxAddress;
			OkEnabled = BytesUsed > 0 && !IsIdentical && !MaxSizeExceeded;

			HasWarning = errorList.Count > 0 || OriginalSizeExceeded || MaxSizeExceeded;

			ByteCodeView = sb.ToString();
			Errors = errorList;
			_bytes = convertedByteCode;
		}

		private bool MatchesOriginalCode(List<byte> convertedByteCode)
		{
			bool isIdentical = false;
			if(_originalCode.Length > 0 && convertedByteCode.Count == _originalCode.Length) {
				isIdentical = true;
				for(int i = 0; i < _originalCode.Length; i++) {
					if(_originalCode[i] != convertedByteCode[i]) {
						isIdentical = false;
						break;
					}
				}
			}

			return isIdentical;
		}

		public async Task<bool> ApplyChanges(Window assemblerWindow)
		{
			MemoryType memType = CpuType.ToMemoryType();

			List<byte> bytes = new List<byte>(_bytes);
			if(OriginalByteCount > 0) {
				byte nopOpCode = CpuType.GetNopOpCode();
				while(OriginalByteCount > bytes.Count) {
					//Pad data with NOPs as needed
					bytes.Add(nopOpCode);
				}
			}

			string addrFormat = memType.GetFormatString();
			UInt32 endAddress = (uint)(StartAddress + bytes.Count - 1);

			List<string> warningMessages = new List<string>();
			if(Errors.Count > 0) {
				warningMessages.Add("Warning: The code contains errors - lines with errors will be ignored.");
			}

			if(_originalAddress >= 0) {
				if(OriginalSizeExceeded) {
					warningMessages.Add(ResourceHelper.GetViewLabel(nameof(AssemblerWindow), "lblByteCountExceeded"));
				}
			} else {
				warningMessages.Add($"Warning: The code currently mapped to CPU memory addresses ${StartAddress.ToString(addrFormat)} to ${endAddress.ToString(addrFormat)} will be overridden.");
			}

			if(warningMessages.Count > 0 && await MesenMsgBox.Show(assemblerWindow, "AssemblerConfirmation", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, string.Join(Environment.NewLine + Environment.NewLine, warningMessages)) != DialogResult.OK) {
				return false;
			}


			DebugApi.SetMemoryValues(memType, (uint)StartAddress, bytes.ToArray(), bytes.Count);
			if(OriginalByteCount > 0) {
				_originalCode = bytes.ToArray();
				IsIdentical = MatchesOriginalCode(bytes);
			}

			AddressInfo absStart = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = StartAddress, Type = memType });
			AddressInfo absEnd = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)endAddress, Type = memType });
			if(absStart.Type == absEnd.Type && (absEnd.Address - absStart.Address + 1) == bytes.Count) {
				DebugApi.MarkBytesAs(absStart.Type, (uint)absStart.Address, (uint)absEnd.Address, CdlFlags.Code);
			}

			DebuggerWindow? wnd = DebugWindowManager.GetDebugWindow<DebuggerWindow>(wnd => wnd.CpuType == CpuType);
			if(wnd != null) {
				wnd.RefreshDisassembly();
			}

			return true;
		}

		private enum AssemblerSpecialCodes
		{
			OK = 0,
			EndOfLine = -1,
			ParsingError = -2,
			OutOfRangeJump = -3,
			LabelRedefinition = -4,
			MissingOperand = -5,
			OperandOutOfRange = -6,
			InvalidHex = -7,
			InvalidSpaces = -8,
			TrailingText = -9,
			UnknownLabel = -10,
			InvalidInstruction = -11,
			InvalidBinaryValue = -12,
			InvalidOperands = -13,
			InvalidLabel = -14,
		}
	}

	public class AssemblerError
	{
		public string Message { get; set; } = "";
		public int LineNumber { get; set; }

		public override string ToString()
		{
			return "Line " + LineNumber.ToString() + ": " + this.Message;
		}
	}
}
