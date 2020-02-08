using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using Mesen.GUI.Debugger.Code;
using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmTraceLogger : BaseForm
	{
		private int _lineCount;
		private bool _loggingEnabled = false;
		private string _lastFilename;
		private EntityBinder _entityBinder = new EntityBinder();
		private string _previousTrace;
		private UInt64 _previousCycleCount;
		private volatile bool _refreshRunning;
		private bool _initialized;
		private NotificationListener _notifListener;
		private InteropTraceLoggerOptions _interopOptions;

		public frmTraceLogger()
		{
			InitializeComponent();

			TraceLoggerInfo config = ConfigManager.Config.Debug.TraceLogger;
			RestoreLocation(config.WindowLocation, config.WindowSize);

			txtTraceLog.BaseFont = new Font(config.FontFamily, config.FontSize, config.FontStyle);
			txtTraceLog.TextZoom = config.TextZoom;

			mnuAutoRefresh.Checked = config.AutoRefresh;
			_lineCount = config.LineCount;

			_entityBinder.Entity = config.LogOptions;

			_entityBinder.AddBinding(nameof(TraceLoggerOptions.LogCpu), chkLogCpu);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.LogSpc), chkLogSpc);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.LogNecDsp), chkLogNecDsp);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.LogSa1), chkLogSa1);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.LogGsu), chkLogGsu);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.LogCx4), chkLogCx4);

			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowByteCode), chkShowByteCode);
			//_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowCpuCycles), chkShowCpuCycles);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowEffectiveAddresses), chkShowEffectiveAddresses);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowMemoryValues), chkShowMemoryValues);
			//_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowExtraInfo), chkShowExtraInfo);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowPpuFrames), chkShowFrameCount);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowPpuCycles), chkShowPpuCycles);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowPpuScanline), chkShowPpuScanline);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ShowRegisters), chkShowRegisters);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.IndentCode), chkIndentCode);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.UseLabels), chkUseLabels);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.ExtendZeroPage), chkExtendZeroPage);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.UseWindowsEol), chkUseWindowsEol);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.StatusFormat), cboStatusFlagFormat);
			_entityBinder.AddBinding(nameof(TraceLoggerOptions.OverrideFormat), chkOverrideFormat);
			_entityBinder.UpdateUI();

			this.toolTip.SetToolTip(this.picExpressionWarning, "Condition contains invalid syntax or symbols.");
			//this.toolTip.SetToolTip(this.picHelp, "When a condition is given, instructions will only be logged by the trace logger if the condition returns a value not equal to 0 or false." + Environment.NewLine + Environment.NewLine + frmBreakpoint.GetConditionTooltip(false));
			this.toolTip.SetToolTip(this.picFormatHelp,
				"You can customize the trace logger's output by enabling the 'Override' option and altering the format." + Environment.NewLine + Environment.NewLine +
				"The following tags are available: " + Environment.NewLine +
				"[ByteCode]: The byte code for the instruction (1 to 3 bytes)." + Environment.NewLine +
				"[Disassembly]: The disassembly for the current instruction." + Environment.NewLine +
				"[EffectiveAddress]: The effective address used for indirect addressing modes." + Environment.NewLine +
				"[MemoryValue]: The value stored at the memory location referred to by the instruction." + Environment.NewLine +
				"[PC]: Program Counter" + Environment.NewLine +
				"[A]: A register" + Environment.NewLine +
				"[X]: X register" + Environment.NewLine +
				"[Y]: Y register" + Environment.NewLine +
				"[SP]: Stack Pointer" + Environment.NewLine +
				"[P]: Processor Flags" + Environment.NewLine +
				"[Cycle]: The current PPU cycle (H)" + Environment.NewLine +
				"[HClock]: The PPU's current H-clock position (H, in terms of master clocks)" + Environment.NewLine +
				"[Scanline]: The current PPU scanline (V)" + Environment.NewLine +
				"[FrameCount]: The current PPU frame." + Environment.NewLine +
				"[CycleCount]: The current CPU cycle (32-bit signed value, resets to 0 at power on)" + Environment.NewLine + Environment.NewLine +
				"You can also specify some options by using a comma. e.g:" + Environment.NewLine +
				"[Cycle,3] will display the cycle and pad out the output to always be 3 characters wide." + Environment.NewLine +
				"[Scanline,h] will display the scanline in hexadecimal." + Environment.NewLine +
				"[Align,50]: Align is a special tag that is useful when trying to align some content. [Align,50] will make the next tag start on column 50."
			);

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			this._initialized = true;
		}

		private void InitShortcuts()
		{
			mnuIncreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.IncreaseFontSize));
			mnuDecreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.DecreaseFontSize));
			mnuResetFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.ResetFontSize));
			mnuRefresh.InitShortcut(this, nameof(DebuggerShortcutsConfig.Refresh));
			mnuCopy.InitShortcut(this, nameof(DebuggerShortcutsConfig.Copy));
			mnuSelectAll.InitShortcut(this, nameof(DebuggerShortcutsConfig.SelectAll));

			//TODO
			//mnuEditInMemoryViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.CodeWindow_EditInMemoryViewer));
			//mnuViewInDisassembly.InitShortcut(this, nameof(DebuggerShortcutsConfig.MemoryViewer_ViewInDisassembly));
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			UpdateMenu();
			tmrUpdateLog.Start();
			RefreshLog(true, true);

			InitShortcuts();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			_notifListener?.Dispose();

			tmrUpdateLog.Stop();
			while(_refreshRunning) {
				System.Threading.Thread.Sleep(50);
			}

			base.OnFormClosing(e);

			TraceLoggerInfo config = ConfigManager.Config.Debug.TraceLogger;
			config.AutoRefresh = mnuAutoRefresh.Checked;
			config.LineCount = _lineCount;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;

			config.FontFamily = txtTraceLog.BaseFont.FontFamily.Name;
			config.FontSize = txtTraceLog.BaseFont.Size;
			config.FontStyle = txtTraceLog.BaseFont.Style;
			config.TextZoom = txtTraceLog.TextZoom;

			_entityBinder.UpdateObject();

			//Disable logging when we close the trace logger
			SetOptions(true);

			ConfigManager.ApplyChanges();

			if(_loggingEnabled) {
				DebugApi.StopTraceLogger();
			}
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					//Configuration is lost when debugger is restarted (when switching game or power cycling)
					SetCoreOptions();
					break;
			}
		}

		protected void UpdateFormatOptions()
		{
			if(!chkOverrideFormat.Checked) {
				string format = "[PC,6h] ";
				if(chkShowByteCode.Checked) {
					format += "[ByteCode,15h] ";
				}
				format += "[Disassembly]";
				int alignValue = 40;
				if(chkShowEffectiveAddresses.Checked) {
					format += "[EffectiveAddress]";
					alignValue += 8;
				}
				if(chkShowMemoryValues.Checked) {
					format += " [MemoryValue,h]";
					alignValue += 6;
				}
				format += "[Align," + alignValue.ToString() + "] ";

				if(chkShowRegisters.Checked) {
					format += "A:[A,4h] X:[X,4h] Y:[Y,4h] S:[SP,4h] D:[D,4h] DB:[DB,2h] ";
					switch(cboStatusFlagFormat.GetEnumValue<StatusFlagFormat>()) {
						case StatusFlagFormat.Hexadecimal: format += "P:[P,h] "; break;
						case StatusFlagFormat.CompactText: format += "P:[P] "; break;
						case StatusFlagFormat.Text: format += "P:[P,8] "; break;
					}
				}
				if(chkShowPpuScanline.Checked) {
					format += "V:[Scanline,3] ";
				}
				if(chkShowPpuCycles.Checked) {
					format += "H:[Cycle,3] ";
				}
				if(chkShowFrameCount.Checked) {
					format += "FC:[FrameCount] ";
				}
				if(chkShowCpuCycles.Checked) {
					format += "CPU Cycle:[CycleCount]";
				}
				txtFormat.Text = format.Trim();
			}

			txtFormat.ReadOnly = !chkOverrideFormat.Checked;
			chkShowByteCode.Enabled = !chkOverrideFormat.Checked;
			chkShowRegisters.Enabled = !chkOverrideFormat.Checked;
			chkShowPpuCycles.Enabled = !chkOverrideFormat.Checked;
			chkShowPpuScanline.Enabled = !chkOverrideFormat.Checked;
			chkShowFrameCount.Enabled = !chkOverrideFormat.Checked;
			chkShowCpuCycles.Enabled = !chkOverrideFormat.Checked;
			chkShowEffectiveAddresses.Enabled = !chkOverrideFormat.Checked;
			chkShowMemoryValues.Enabled = !chkOverrideFormat.Checked;
			cboStatusFlagFormat.Enabled = !chkOverrideFormat.Checked;

			if(_initialized) {
				RefreshLog(false, true);
			}
		}

		private void SetOptions(bool disableLogging = false)
		{
			_entityBinder.UpdateObject();
			TraceLoggerOptions options = (TraceLoggerOptions)_entityBinder.Entity;

			_interopOptions = new InteropTraceLoggerOptions();
			_interopOptions.LogCpu = !disableLogging && options.LogCpu;
			_interopOptions.LogSpc = !disableLogging && options.LogSpc;
			_interopOptions.LogNecDsp = !disableLogging && options.LogNecDsp;
			_interopOptions.LogSa1 = !disableLogging && options.LogSa1;
			_interopOptions.LogGsu = !disableLogging && options.LogGsu;
			_interopOptions.LogCx4 = !disableLogging && options.LogCx4;
			_interopOptions.IndentCode = options.IndentCode;
			_interopOptions.ShowExtraInfo = options.ShowExtraInfo;
			_interopOptions.UseLabels = options.UseLabels;
			_interopOptions.UseWindowsEol = options.UseWindowsEol;
			_interopOptions.ExtendZeroPage = options.ExtendZeroPage;

			_interopOptions.Condition = Encoding.UTF8.GetBytes(txtCondition.Text);
			Array.Resize(ref _interopOptions.Condition, 1000);

			_interopOptions.Format = Encoding.UTF8.GetBytes(txtFormat.Text.Replace("\t", "\\t"));
			Array.Resize(ref _interopOptions.Format, 1000);

			DebugApi.SetTraceOptions(_interopOptions);
		}

		private void SetCoreOptions()
		{
			DebugApi.SetTraceOptions(_interopOptions);
		}

		private void btnStartLogging_Click(object sender, EventArgs e)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter("Trace logs (*.txt)|*.txt");
				sfd.FileName = "Trace.txt";
				sfd.InitialDirectory = ConfigManager.DebuggerFolder;
				if(sfd.ShowDialog() == DialogResult.OK) {
					_lastFilename = sfd.FileName;
					SetOptions();
					DebugApi.StartTraceLogger(sfd.FileName);

					btnStartLogging.Enabled = false;
					btnStopLogging.Enabled = true;
					btnOpenTrace.Enabled = false;

					_loggingEnabled = true;
				}
			}
		}

		private void btnStopLogging_Click(object sender, EventArgs e)
		{
			DebugApi.StopTraceLogger();
			btnStartLogging.Enabled = true;
			btnStopLogging.Enabled = false;
			btnOpenTrace.Enabled = true;
		}

		private void btnOpenTrace_Click(object sender, EventArgs e)
		{
			try {
				System.Diagnostics.Process.Start(_lastFilename);
			} catch { }
		}

		private void RefreshLog(bool scrollToBottom, bool forceUpdate)
		{
			if(_refreshRunning) {
				return;
			}

			//TODO
			//Make sure labels are up to date
			//DebugWorkspaceManager.GetWorkspace();

			_refreshRunning = true;
			SetOptions();
			Task.Run(() => {
				//Update trace log in another thread for performance
				DebugState state = DebugApi.GetState();
				if(_previousCycleCount != state.Cpu.CycleCount || forceUpdate) {
					string newTrace = DebugApi.GetExecutionTrace((UInt32)_lineCount);
					_previousCycleCount = state.Cpu.CycleCount;
					_previousTrace = newTrace;

					int index = 0;
					string line = null;
					Func<bool> readLine = () => {
						if(index < newTrace.Length) {
							int endOfLineIndex = newTrace.IndexOf('\n', index);
							line = newTrace.Substring(index, endOfLineIndex - index);
							index = endOfLineIndex + 1;
							return true;
						} else {
							return false;
						}
					};

					List<int> lineFlags = new List<int>(30000);
					List<int> programCounter = new List<int>(30000);
					List<string> byteCode = new List<string>(30000);
					List<string> lineContent = new List<string>(30000);
					List<int> indent = new List<int>(30000);

					bool showByteCode = false;
					while(readLine()) {
						string[] parts = line.Split('\x1');
						lineFlags.Add((int)parts[0][0]);
						programCounter.Add(Int32.Parse(parts[1], System.Globalization.NumberStyles.HexNumber));
						byteCode.Add(parts[2]);

						string content = parts[3];
						while(true) {
							string str = content.TrimStart();
							if(str.StartsWith(parts[1])) {
								content = str.Substring(parts[1].Length);
							} else if(str.StartsWith(parts[2])) {
								content = str.Substring(parts[2].Length);
								showByteCode = true;
							} else if(str.StartsWith(parts[3].Replace("$", ""))) {
								content = str.Substring(8);
								showByteCode = true;
							} else {
								break;
							}
						}
						lineContent.Add(content.TrimStart());
					}
					this.BeginInvoke((Action)(() => {
						txtTraceLog.ShowContentNotes = showByteCode;
						txtTraceLog.ShowSingleContentLineNotes = showByteCode;

						txtTraceLog.DataProvider = new TraceLoggerCodeDataProvider(lineContent, programCounter, byteCode, lineFlags);
						txtTraceLog.StyleProvider = new TraceLoggerStyleProvider(lineFlags);
						
						if(scrollToBottom) {
							txtTraceLog.ScrollToLineIndex(txtTraceLog.LineCount - 1);
						}
					}));
				}
				_refreshRunning = false;
			});
		}

		private void UpdateMenu()
		{
			mnu30000Lines.Checked = _lineCount == 30000;
			mnu15000Lines.Checked = _lineCount == 15000;
			mnu10000Lines.Checked = _lineCount == 10000;
			mnu5000Lines.Checked = _lineCount == 5000;
			mnu1000Lines.Checked = _lineCount == 1000;
			mnu100Lines.Checked = _lineCount == 100;

			if(_lineCount >= 1000) {
				tmrUpdateLog.Interval = 250;
			} else {
				tmrUpdateLog.Interval = 150;
			}
		}

		private void tmrUpdateLog_Tick(object sender, EventArgs e)
		{
			if(txtCondition.Text.Length > 0) {
				//TODO
				/*EvalResultType resultType;
				InteropEmu.DebugEvaluateExpression(txtCondition.Text, out resultType, false);
				picExpressionWarning.Visible = (resultType == EvalResultType.Invalid);*/
			} else {
				picExpressionWarning.Visible = false;
			}

			if(mnuAutoRefresh.Checked) {
				RefreshLog(true, false);
			}
		}

		private void SetLineCount(int count)
		{
			_lineCount = count;
			UpdateMenu();
			RefreshLog(true, true);
		}

		private void mnu30000Lines_Click(object sender, EventArgs e)
		{
			SetLineCount(30000);
		}

		private void mnu15000Lines_Click(object sender, EventArgs e)
		{
			SetLineCount(15000);
		}

		private void mnu10000Lines_Click(object sender, EventArgs e)
		{
			SetLineCount(10000);
		}

		private void mnu5000Lines_Click(object sender, EventArgs e)
		{
			SetLineCount(5000);
		}

		private void mnu1000Lines_Click(object sender, EventArgs e)
		{
			SetLineCount(1000);
		}

		private void mnu100Lines_Click(object sender, EventArgs e)
		{
			SetLineCount(100);
		}

		private void mnuRefresh_Click(object sender, EventArgs e)
		{
			RefreshLog(false, true);
		}

		private void mnuIncreaseFontSize_Click(object sender, EventArgs e)
		{
			txtTraceLog.TextZoom += 10;
		}

		private void mnuDecreaseFontSize_Click(object sender, EventArgs e)
		{
			txtTraceLog.TextZoom -= 10;
		}

		private void mnuResetFontSize_Click(object sender, EventArgs e)
		{
			txtTraceLog.TextZoom = 100;
		}

		private void mnuSelectFont_Click(object sender, EventArgs e)
		{
			txtTraceLog.BaseFont = FontDialogHelper.SelectFont(txtTraceLog.BaseFont);
		}

		private void chkOptions_CheckedChanged(object sender, EventArgs e)
		{
			UpdateFormatOptions();
		}

		private void cboStatusFlagFormat_SelectedIndexChanged(object sender, EventArgs e)
		{
			UpdateFormatOptions();
		}

		private void txtFormat_TextChanged(object sender, EventArgs e)
		{
			if(chkOverrideFormat.Checked) {
				//Only save format string when override flag is set
				((TraceLoggerOptions)_entityBinder.Entity).Format = txtFormat.Text;
			}
		}

		private void chkOverrideFormat_CheckedChanged(object sender, EventArgs e)
		{
			if(chkOverrideFormat.Checked) {
				string format = ((TraceLoggerOptions)_entityBinder.Entity).Format;
				if(string.IsNullOrWhiteSpace(format)) {
					format = txtFormat.Text;
				}
				txtFormat.Text = format;
			}
			UpdateFormatOptions();
		}

		private void mnuCopy_Click(object sender, EventArgs e)
		{
			string[] lines = _previousTrace.Split('\n');
			StringBuilder sb = new StringBuilder();
			for(int i = txtTraceLog.SelectionStart, end = txtTraceLog.SelectionStart + txtTraceLog.SelectionLength; i <= end && i < lines.Length; i++) {
				sb.Append(lines[i]);
			}
			Clipboard.SetText(sb.ToString());

			txtTraceLog.CopySelection(true, true, false);
		}

		private void mnuSelectAll_Click(object sender, EventArgs e)
		{
			txtTraceLog.SelectAll();
		}

		private void btnClearLog_Click(object sender, EventArgs e)
		{
			DebugApi.ClearTraceLog();
			RefreshLog(true, true);
		}
	}

	public class TraceLoggerCodeDataProvider : ICodeDataProvider
	{
		private List<string> _textLines;
		private List<Int32> _addresses;
		private List<string> _byteCode;
		private List<int> _flags;

		public TraceLoggerCodeDataProvider(List<string> lineContent, List<int> programCounter, List<string> byteCode, List<int> lineFlags)
		{
			_addresses = programCounter;
			_byteCode = byteCode;
			_textLines = lineContent;
			_flags = lineFlags;
		}

		public CodeLineData GetCodeLineData(int lineIndex)
		{
			int count = _textLines.Count - 1;
			return new CodeLineData(_flags[count - lineIndex] == 3 ? CpuType.Spc : CpuType.Cpu) {
				Address = _addresses[count - lineIndex],
				Text = _textLines[count - lineIndex],
				ByteCode = _byteCode[count - lineIndex],
				EffectiveAddress = -1
			};
		}

		public bool UseOptimizedSearch { get { return false; } }

		public int GetLineAddress(int lineIndex)
		{
			return _addresses[lineIndex];
		}

		public int GetLineCount()
		{
			return _textLines.Count;
		}

		public int GetLineIndex(uint address)
		{
			throw new NotImplementedException();
		}

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			throw new NotImplementedException();
		}
	}

	public class TraceLoggerStyleProvider : ILineStyleProvider
	{
		private Color _spcColor = Color.FromArgb(30, 145, 30);
		private Color _spcBgColor = Color.FromArgb(230, 245, 230);
		private Color _dspColor = Color.FromArgb(30, 30, 145);
		private Color _dspBgColor = Color.FromArgb(230, 230, 245);

		private List<int> _flags;

		public TraceLoggerStyleProvider(List<int> lineFlags)
		{
			_flags = lineFlags;
		}

		public List<CodeColor> GetCodeColors(CodeLineData lineData, bool highlightCode, string addressFormat, Color? textColor, bool showMemoryValues)
		{
			return CodeHighlighting.GetCpuHighlights(lineData, highlightCode, addressFormat, textColor, showMemoryValues);
		}

		public string GetLineComment(int lineIndex)
		{
			return null;
		}

		public LineProperties GetLineStyle(CodeLineData lineData, int lineIndex)
		{
			int count = _flags.Count - 1;
			int cpuType = _flags[count - lineIndex];
			if(cpuType == 3) {
				//SPC
				return new LineProperties() { AddressColor = _spcColor, LineBgColor = _spcBgColor };
			} else if(cpuType == 4) {
				//DSP
				return new LineProperties() { AddressColor = _dspColor, LineBgColor = _dspBgColor };
			} else {
				//CPU
				return new LineProperties() { AddressColor = null, LineBgColor = null };
			}
		}
	}
}
