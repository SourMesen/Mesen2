using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using Be.Windows.Forms;
using FastColoredTextBoxNS;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmAssembler : BaseForm
	{
		private CpuType _cpuType;
		private int _startAddress;
		private int _blockLength;
		private bool _hasParsingErrors = false;
		private bool _isEditMode = false;
		private bool _startAddressValid = true;
		private string _initialCode = "";
		private int _textVersion = 0;
		private bool _updating = false;

		public frmAssembler(CpuType? cpuType = null, string code = "", int startAddress = 0, int blockLength = 0)
		{
			if(cpuType != null) {
				_cpuType = cpuType.Value;
			} else {
				_cpuType = EmuApi.GetRomInfo().CoprocessorType == CoprocessorType.Gameboy ? CpuType.Gameboy : CpuType.Cpu;
			}

			if(_cpuType == CpuType.Sa1) {
				_cpuType = CpuType.Cpu;
			}

			InitializeComponent();
			
			txtCode.ForeColor = Color.Black;

			AssemblerConfig cfg = ConfigManager.Config.Debug.Assembler;
			RestoreLocation(cfg.WindowLocation, cfg.WindowSize);
			txtCode.Font = new Font(cfg.FontFamily, cfg.FontSize, cfg.FontStyle);
			txtCode.Zoom = cfg.Zoom;

			UpdateCodeHighlighting();

			_initialCode = code;
			_startAddress = startAddress;
			_blockLength = blockLength;

			ctrlHexBox.Font = new Font(BaseControl.MonospaceFontFamily, 10, FontStyle.Regular);
			ctrlHexBox.SelectionForeColor = Color.White;
			ctrlHexBox.SelectionBackColor = Color.FromArgb(31, 123, 205);
			ctrlHexBox.InfoBackColor = Color.FromArgb(235, 235, 235);
			ctrlHexBox.InfoForeColor = Color.Gray;
			ctrlHexBox.Width = ctrlHexBox.RequiredWidth;
			ctrlHexBox.ByteProvider = new StaticByteProvider(new byte[0]);

			txtStartAddress.Text = _startAddress.ToString("X6");
		}

		private void InitShortcuts()
		{
			mnuIncreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.IncreaseFontSize));
			mnuDecreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.DecreaseFontSize));
			mnuResetFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.ResetFontSize));

			mnuPaste.InitShortcut(this, nameof(DebuggerShortcutsConfig.Paste));
			mnuCopy.InitShortcut(this, nameof(DebuggerShortcutsConfig.Copy));
			mnuCut.InitShortcut(this, nameof(DebuggerShortcutsConfig.Cut));
			mnuSelectAll.InitShortcut(this, nameof(DebuggerShortcutsConfig.SelectAll));
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);
			if(!string.IsNullOrWhiteSpace(_initialCode)) {
				_isEditMode = true;
				txtCode.Text = _initialCode;
			} else {
				_initialCode = "";
				txtCode.Text = _initialCode;
			}

			txtCode.ClearUndo();

			toolTip.SetToolTip(picSizeWarning, "Warning: The new code exceeds the original code's length." + Environment.NewLine + "Applying this modification will overwrite other portions of the code and potentially cause problems.");
			toolTip.SetToolTip(picStartAddressWarning, "Warning: Start address is invalid.  Must be a valid hexadecimal string.");

			UpdateWindow();
			InitShortcuts();
		}

		private void UpdateCodeHighlighting()
		{
			if(txtCode.SyntaxDescriptor != null) {
				SyntaxDescriptor desc = txtCode.SyntaxDescriptor;
				txtCode.SyntaxDescriptor = null;
				txtCode.ClearStylesBuffer();
				desc.Dispose();			
			}

			DebuggerInfo cfg = ConfigManager.Config.Debug.Debugger;

			SyntaxDescriptor syntax = new SyntaxDescriptor();
			syntax.styles.Add(new TextStyle(new SolidBrush(cfg.CodeOpcodeColor), Brushes.Transparent, FontStyle.Regular));
			syntax.styles.Add(new TextStyle(new SolidBrush(cfg.CodeLabelDefinitionColor), Brushes.Transparent, FontStyle.Regular));
			syntax.styles.Add(new TextStyle(new SolidBrush(cfg.CodeImmediateColor), Brushes.Transparent, FontStyle.Regular));
			syntax.styles.Add(new TextStyle(new SolidBrush(cfg.CodeAddressColor), Brushes.Transparent, FontStyle.Regular));
			syntax.styles.Add(new TextStyle(new SolidBrush(cfg.CodeCommentColor), Brushes.Transparent, FontStyle.Regular));
			syntax.rules.Add(new RuleDesc() { style = syntax.styles[0], pattern = @"(\n|^)[ \t]*(?<range>[a-zA-Z]{3}[*]{0,1})( |[^:a-zA-Z])" });
			syntax.rules.Add(new RuleDesc() { style = syntax.styles[1], pattern = @"(\n|^)[ \t]*(?<range>[@_a-zA-Z]+[@_a-zA-Z0-9]*):" });
			syntax.rules.Add(new RuleDesc() { style = syntax.styles[1], pattern = @"(\n|^)[ \t]*[a-zA-Z]{3}[ \t]+[(]{0,1}(?<range>[@_a-zA-Z]([@_a-zA-Z0-9])+)" });
			syntax.rules.Add(new RuleDesc() { style = syntax.styles[2], pattern = @"(\n|^)[ \t]*[a-zA-Z]{3}[ \t]+(?<range>#[$]{0,1}([A-Fa-f0-9])+)" });
			syntax.rules.Add(new RuleDesc() { style = syntax.styles[3], pattern = @"(\n|^)[ \t]*[a-zA-Z]{3}[ \t]+[\[(]{0,1}(?<range>([$][A-Fa-f0-9]+)|([0-9]+))[\])]{0,1}[ \t]*(,X|,Y|,x|,y|,s|,s\),y){0,1}[\])]{0,1}" });
			syntax.rules.Add(new RuleDesc() { style = syntax.styles[4], pattern = @"(\n|^)[^\n;]*(?<range>;[^\n]*)" });
			txtCode.SyntaxDescriptor = syntax;
			txtCode.OnTextChanged();
		}

		private bool SizeExceeded
		{
			get { return ctrlHexBox.ByteProvider.Length > _blockLength; }
		}

		private bool IsIdentical
		{
			get
			{
				if(ctrlHexBox.ByteProvider.Length != _blockLength) {
					return false;
				} else {
					for(int i = 0; i < ctrlHexBox.ByteProvider.Length; i++) {
						if(DebugApi.GetMemoryValue(_cpuType.ToMemoryType(), (UInt32)(_startAddress + i)) != ctrlHexBox.ByteProvider.ReadByte(i)) {
							return false;
						}
					}
					return true;
				}
			}
		}

		private void UpdateWindow()
		{
			if(!this.IsHandleCreated) {
				return;
			}

			btnOk.Enabled = false;

			_textVersion++;
			if(_updating) {
				return;
			}

			_updating = true;
			string text = txtCode.Text;
			int version = _textVersion;
			
			Task.Run(() => {
				short[] byteCode = DebugApi.AssembleCode(_cpuType, text, (UInt32)_startAddress);

				this.BeginInvoke((Action)(() => {
					_updating = false;
					if(_textVersion != version) {
						UpdateWindow();
					}

					List<byte> convertedByteCode = new List<byte>();
					List<ErrorDetail> errorList = new List<ErrorDetail>();
					string[] codeLines = text.Replace("\r", "").Split('\n');
					int line = 1;
					foreach(short s in byteCode) {
						if(s >= 0) {
							convertedByteCode.Add((byte)s);
						} else if(s == (int)AssemblerSpecialCodes.EndOfLine) {
							line++;
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
							errorList.Add(new ErrorDetail() { Message = message + " - " + codeLines[line-1], LineNumber = line });
							line++;
						}
					}

					_hasParsingErrors = errorList.Count > 0;

					lstErrors.BeginUpdate();
					lstErrors.Items.Clear();
					lstErrors.Items.AddRange(errorList.ToArray());
					lstErrors.EndUpdate();

					ctrlHexBox.ByteProvider = new StaticByteProvider(convertedByteCode.ToArray());

					UpdateButtons();
				}));
			});
		}

		private void UpdateButtons()
		{
			if(_isEditMode) {
				lblByteUsage.Text = ctrlHexBox.ByteProvider.Length.ToString() + " / " + _blockLength.ToString();
				lblByteUsage.ForeColor = SizeExceeded ? Color.Red : Color.Black;
				picSizeWarning.Visible = SizeExceeded;

				bool isIdentical = IsIdentical;
				lblNoChanges.Visible = isIdentical;
				btnOk.Image = _hasParsingErrors || SizeExceeded ? Properties.Resources.Warning : null;
				btnOk.Enabled = !isIdentical && _startAddressValid && ctrlHexBox.ByteProvider.Length > 0;
			} else {
				lblNoChanges.Visible = false;
				btnOk.Image = _hasParsingErrors ? Properties.Resources.Warning : null;
				btnOk.Enabled = _startAddressValid && ctrlHexBox.ByteProvider.Length > 0;

				lblByteUsage.Text = ctrlHexBox.ByteProvider.Length.ToString();
			}

			picStartAddressWarning.Visible = !_startAddressValid;
		}
		
		private void txtCode_TextChanged(object sender, FastColoredTextBoxNS.TextChangedEventArgs e)
		{
			UpdateWindow();
		}

		private void txtStartAddress_TextChanged(object sender, EventArgs e)
		{
			try {
				_startAddress = int.Parse(txtStartAddress.Text, System.Globalization.NumberStyles.HexNumber);
				_startAddressValid = true;
			} catch {
				_startAddressValid = false;
			}
			UpdateWindow();
		}

		private void btnOk_Click(object sender, EventArgs e)
		{
			List<string> warningMessages = new List<string>();
			if(_hasParsingErrors) {
				warningMessages.Add("Warning: The code contains parsing errors - lines with errors will be ignored.");
			}
			if(_isEditMode) {
				if(SizeExceeded) {
					warningMessages.Add("Warning: The new code exceeds the original code's length." + Environment.NewLine + "Applying this modification will overwrite other portions of the code and potentially cause problems.");
				}
			} else {
				warningMessages.Add($"Warning: The contents currently mapped to CPU memory addresses ${_startAddress.ToString("X6")} to ${(_startAddress+ctrlHexBox.ByteProvider.Length).ToString("X6")} will be overridden.");
			}

			if(warningMessages.Count == 0 || MessageBox.Show(string.Join(Environment.NewLine+Environment.NewLine, warningMessages.ToArray()) + Environment.NewLine + Environment.NewLine + "OK?", "Warning", MessageBoxButtons.OKCancel) == DialogResult.OK) {
				List<byte> bytes = new List<byte>(((StaticByteProvider)ctrlHexBox.ByteProvider).Bytes);
				int byteGap = (int)(_blockLength - ctrlHexBox.ByteProvider.Length);
				for(int i = 0; i < byteGap; i++) {
					//Pad data with NOPs as needed
					bytes.Add(0xEA);
				}

				SnesMemoryType memType;
				SnesMemoryType prgType;
				if(_cpuType == CpuType.Gameboy) {
					memType = SnesMemoryType.GameboyMemory;
					prgType = SnesMemoryType.GbPrgRom;
				} else {
					memType = SnesMemoryType.CpuMemory;
					prgType = SnesMemoryType.PrgRom;
				}

				DebugApi.SetMemoryValues(memType, (UInt32)_startAddress, bytes.ToArray(), bytes.Count);
				AddressInfo absStart = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = _startAddress, Type = memType });
				AddressInfo absEnd = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = _startAddress + bytes.Count, Type = memType });
				if(absStart.Type == prgType && absEnd.Type == prgType && (absEnd.Address - absStart.Address) == bytes.Count) {
					DebugApi.MarkBytesAs((uint)absStart.Address, (uint)absEnd.Address, CdlFlags.Code);
				}

				frmDebugger debugger = DebugWindowManager.OpenDebugger(_cpuType);
				if(debugger != null) {
					debugger.RefreshDisassembly();
				}

				this.DialogResult = DialogResult.OK;
				this.Close();
			}
		}

		private void btnCancel_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			if(_updating) {
				e.Cancel = true;
				return;
			}

			base.OnClosing(e);
			AssemblerConfig cfg = ConfigManager.Config.Debug.Assembler;
			cfg.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			cfg.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			cfg.Zoom = txtCode.Zoom;
			cfg.FontFamily = txtCode.OriginalFont.FontFamily.Name;
			cfg.FontSize = txtCode.OriginalFont.Size;
			cfg.FontStyle = txtCode.OriginalFont.Style;
			ConfigManager.ApplyChanges();
		}

		private void mnuClose_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void mnuConfigureColors_Click(object sender, EventArgs e)
		{
			using(frmDebuggerColors frm = new frmDebuggerColors()) {
				if(frm.ShowDialog(this, this) == DialogResult.OK) {
					UpdateCodeHighlighting();
				}
			}
		}

		private void mnuIncreaseFontSize_Click(object sender, EventArgs e)
		{
			txtCode.Zoom += 10;
		}

		private void mnuDecreaseFontSize_Click(object sender, EventArgs e)
		{
			txtCode.Zoom -= 10;
		}

		private void mnuResetFontSize_Click(object sender, EventArgs e)
		{
			txtCode.Zoom = 100;
		}

		private void mnuSelectFont_Click(object sender, EventArgs e)
		{
			txtCode.Font = FontDialogHelper.SelectFont(txtCode.OriginalFont);
			txtCode.Zoom = txtCode.Zoom;
		}

		private void mnuCopy_Click(object sender, EventArgs e)
		{
			txtCode.Copy();
		}

		private void mnuCut_Click(object sender, EventArgs e)
		{
			txtCode.Cut();
		}

		private void mnuPaste_Click(object sender, EventArgs e)
		{
			txtCode.Paste();
		}

		private void mnuSelectAll_Click(object sender, EventArgs e)
		{
			txtCode.SelectAll();
		}

		enum AssemblerSpecialCodes
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

		private void lstErrors_DoubleClick(object sender, EventArgs e)
		{
			if(lstErrors.SelectedItem != null) {
				int lineNumber = (lstErrors.SelectedItem as ErrorDetail).LineNumber;
				txtCode.Selection = txtCode.GetLine(lineNumber - 1);
				txtCode.SelectionLength = 0;
				txtCode.DoCaretVisible();
				txtCode.Focus();
			}
		}

		private class ErrorDetail
		{
			public string Message { get; set; }
			public int LineNumber { get; set; }

			public override string ToString()
			{
				return "Line " + LineNumber.ToString() + ": " + this.Message;
			}
		}
	}
}
