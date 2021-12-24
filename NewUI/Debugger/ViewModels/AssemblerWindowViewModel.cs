using Avalonia.Controls;
using Avalonia.Media;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reactive.Linq;
using System.Text;

namespace Mesen.Debugger.ViewModels
{
	public class AssemblerWindowViewModel : ViewModelBase
	{
		[Reactive] public string Code { get; set; } = "";
		[Reactive] public string ByteCodeView { get; set; } = "";
		[Reactive] public UInt32 StartAddress { get; set; }
		[Reactive] public UInt32 BytesUsed { get; set; }
		[Reactive] public bool OkEnabled { get; set; } = false;
		[Reactive] public List<AssemblerError> Errors { get; set; } = new List<AssemblerError>();
		public FontConfig Font { get; } = ConfigManager.Config.Debug.Font;

		private CpuType _cpuType;

		//For designer
		public AssemblerWindowViewModel() : this(CpuType.Cpu, "") { }

		public AssemblerWindowViewModel(CpuType cpuType, string code)
		{
			Code = code;
			_cpuType = cpuType;

			if(Design.IsDesignMode) {
				return;
			}

			this.WhenAnyValue(x => x.Code).Subscribe(code => {
				string[] codeLines = code.Replace("\r", "").Split('\n');
				short[] byteCode = DebugApi.AssembleCode(_cpuType, code, StartAddress);
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

				BytesUsed = (uint)convertedByteCode.Count;
				OkEnabled = BytesUsed > 0;
				ByteCodeView = sb.ToString();
				Errors = errorList;
			});
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
