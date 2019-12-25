using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using FastColoredTextBoxNS;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;
using Mesen.GUI.Properties;

namespace Mesen.GUI.Debugger
{
	public partial class frmScript : BaseForm
	{
		private NotificationListener _notifListener;
		private int _scriptId = -1;
		private string _filePath = null;
		private DateTime _lastTimestamp = DateTime.MinValue;
		private AutocompleteMenu _popupMenu;
		private string _originalText = "";
		private string _builtInScriptName = null;

		public frmScript()
		{
			InitializeComponent();
			txtScriptContent.ForeColor = Color.Black;

			List<string> builtInScripts = new List<string> { "DrawMode.lua", "Example.lua", "Grid.lua", "NtscSafeArea.lua" };
			foreach(string script in builtInScripts) {
				ToolStripItem item = mnuBuiltInScripts.DropDownItems.Add(script);
				item.Click += (s, e) => {
					LoadBuiltInScript(item.Text);
				};
			}

			tsToolbar.AddItemsToToolbar(
				mnuOpen, mnuSave, null,
				mnuRun, mnuStop, null,
				mnuBuiltInScripts
			);

			ScriptWindowConfig cfg = ConfigManager.Config.Debug.ScriptWindow;

			_popupMenu = new AutocompleteMenu(txtScriptContent, this);
			_popupMenu.ImageList = new ImageList();
			_popupMenu.ImageList.Images.Add(Resources.Enum);
			_popupMenu.ImageList.Images.Add(Resources.Function);
			_popupMenu.SelectedColor = Color.LightBlue;
			_popupMenu.SearchPattern = @"[\w\.]";

			List<AutocompleteItem> items = new List<AutocompleteItem>();
			_availableFunctions.Sort((a, b) => {
				int type = a[0].CompareTo(b[0]);
				if(type == 0) {
					return a[1].CompareTo(b[1]);
				} else {
					return -type;
				}
			});

			foreach(List<string> item in _availableFunctions) {
				MethodAutocompleteItem autocompleteItem = new MethodAutocompleteItem(item[1]);
				autocompleteItem.ImageIndex = item[0] == "func" ? 1 : 0;
				autocompleteItem.ToolTipTitle = item[2];
				if(!string.IsNullOrWhiteSpace(item[3])) {
					autocompleteItem.ToolTipText = "Parameters" + Environment.NewLine + item[3] + Environment.NewLine + Environment.NewLine;
				}
				if(!string.IsNullOrWhiteSpace(item[4])) {
					autocompleteItem.ToolTipText += "Return Value" + Environment.NewLine + item[4] + Environment.NewLine + Environment.NewLine;
				}
				if(!string.IsNullOrWhiteSpace(item[5])) {
					autocompleteItem.ToolTipText += "Description" + Environment.NewLine + item[5] + Environment.NewLine + Environment.NewLine;
				}
				items.Add(autocompleteItem);
			}

			_popupMenu.Items.SetAutocompleteItems(items);

			UpdateRecentScripts();
			
			mnuTutorialScript.Checked = cfg.ScriptStartupBehavior == ScriptStartupBehavior.ShowTutorial;
			mnuBlankWindow.Checked = cfg.ScriptStartupBehavior == ScriptStartupBehavior.ShowBlankWindow;
			mnuAutoLoadLastScript.Checked = cfg.ScriptStartupBehavior == ScriptStartupBehavior.LoadLastScript;

			if(cfg.AutoLoadLastScript && cfg.RecentScripts.Count > 0) {
				string scriptToLoad = cfg.RecentScripts.Where((s) => File.Exists(s)).FirstOrDefault();
				if(scriptToLoad != null) {
					LoadScriptFile(scriptToLoad, false);
				}
			} else if(mnuTutorialScript.Checked) {
				LoadBuiltInScript("Example.lua");
			}

			RestoreLocation(cfg.WindowLocation, cfg.WindowSize);
			mnuSaveBeforeRun.Checked = cfg.SaveScriptBeforeRun;

			if(cfg.CodeWindowHeight >= ctrlSplit.Panel1MinSize) {
				if(cfg.CodeWindowHeight == Int32.MaxValue) {
					ctrlSplit.CollapsePanel();
				} else {
					ctrlSplit.SplitterDistance = cfg.CodeWindowHeight;
				}
			}

			txtScriptContent.Font = new Font(cfg.FontFamily, cfg.FontSize, cfg.FontStyle);
			txtScriptContent.Zoom = cfg.Zoom;
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += this._notifListener_OnNotification;

			this.InitShortcuts();
		}

		private void InitShortcuts()
		{
			mnuOpen.InitShortcut(this, nameof(DebuggerShortcutsConfig.ScriptWindow_OpenScript));
			mnuSave.InitShortcut(this, nameof(DebuggerShortcutsConfig.ScriptWindow_SaveScript));
			mnuNewScript.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenScriptWindow));
			mnuRun.InitShortcut(this, nameof(DebuggerShortcutsConfig.ScriptWindow_RunScript));
			mnuStop.InitShortcut(this, nameof(DebuggerShortcutsConfig.ScriptWindow_StopScript));

			mnuIncreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.IncreaseFontSize));
			mnuDecreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.DecreaseFontSize));
			mnuResetFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.ResetFontSize));

			mnuPaste.InitShortcut(this, nameof(DebuggerShortcutsConfig.Paste));
			mnuCopy.InitShortcut(this, nameof(DebuggerShortcutsConfig.Copy));
			mnuCut.InitShortcut(this, nameof(DebuggerShortcutsConfig.Cut));
			mnuSelectAll.InitShortcut(this, nameof(DebuggerShortcutsConfig.SelectAll));
		}

		private void _notifListener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.BeforeGameUnload:
				case ConsoleNotificationType.GameLoaded:
					this._scriptId = -1;
					this.BeginInvoke((Action)(() => {
						lblScriptActive.Visible = false;
					}));
					break;
			}
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			if(!SavePrompt()) {
				e.Cancel = true;
				return;
			}
			
			if(_scriptId >= 0) {
				DebugApi.RemoveScript(_scriptId);
			}

			ScriptWindowConfig cfg = ConfigManager.Config.Debug.ScriptWindow;
			cfg.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			cfg.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			cfg.SaveScriptBeforeRun = mnuSaveBeforeRun.Checked;
			if(mnuAutoLoadLastScript.Checked) {
				cfg.ScriptStartupBehavior = ScriptStartupBehavior.LoadLastScript;
			} else if(mnuTutorialScript.Checked) {
				cfg.ScriptStartupBehavior = ScriptStartupBehavior.ShowTutorial;
			} else {
				cfg.ScriptStartupBehavior = ScriptStartupBehavior.ShowBlankWindow;
			}
			cfg.CodeWindowHeight = ctrlSplit.Panel2.Height <= 2 ? Int32.MaxValue : ctrlSplit.SplitterDistance;
			cfg.Zoom = txtScriptContent.Zoom;
			cfg.FontFamily = txtScriptContent.OriginalFont.FontFamily.Name;
			cfg.FontStyle = txtScriptContent.OriginalFont.Style;
			cfg.FontSize = txtScriptContent.OriginalFont.Size;
			cfg.AutoLoadLastScript = mnuAutoLoadLastScript.Checked;
			ConfigManager.ApplyChanges();

			base.OnClosing(e);
		}

		private void LoadScript()
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.SetFilter("Lua scripts (*.lua)|*.lua");
				if(ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count > 0) {
					ofd.InitialDirectory = Path.GetDirectoryName(ConfigManager.Config.Debug.ScriptWindow.RecentScripts[0]);
				}
				if(ofd.ShowDialog() == DialogResult.OK) {
					LoadScriptFile(ofd.FileName);
				}
			}
		}

		private bool SavePrompt()
		{
			if(_originalText != txtScriptContent.Text) {
				DialogResult result = MessageBox.Show("You have unsaved changes for this script - would you like to save them?", "Script Window", MessageBoxButtons.YesNoCancel);
				if((result == DialogResult.Yes && !SaveScript()) || result == DialogResult.Cancel) {
					return false;
				}
			}
			return true;
		}
		
		private void LoadBuiltInScript(string name)
		{
			if(!SavePrompt()) {
				return;
			}

			this.Text = $"{name} - Script Window";
			txtScriptContent.Text = ResourceExtractor.ReadZippedResource(name);
			_originalText = txtScriptContent.Text;
			_builtInScriptName = name;
			txtScriptContent.ClearUndo();
		}

		public void LoadScriptFile(string filepath, bool runScript = true, bool disableSavePrompt = false)
		{
			if(File.Exists(filepath)) {
				if(!disableSavePrompt && !SavePrompt()) {
					return;
				}
				string content = File.ReadAllText(filepath);
				SetFilePath(filepath);
				txtScriptContent.Text = content;
				txtScriptContent.ClearUndo();
				ConfigManager.Config.Debug.ScriptWindow.AddRecentScript(filepath);
				UpdateRecentScripts();
				if(runScript) {
					RunScript();
				}

				_originalText = txtScriptContent.Text;
				_lastTimestamp = File.GetLastWriteTime(filepath);
			}
		}

		private void UpdateRecentScripts()
		{
			mnuRecentScripts.DropDownItems.Clear();
			foreach(string recentScript in ConfigManager.Config.Debug.ScriptWindow.RecentScripts) {
				if(File.Exists(recentScript)) {
					ToolStripMenuItem tsmi = new ToolStripMenuItem();
					tsmi.Text = Path.GetFileName(recentScript);
					tsmi.Click += (object sender, EventArgs args) => {
						LoadScriptFile(recentScript);
					};
					mnuRecentScripts.DropDownItems.Add(tsmi);
				}
			}

			mnuRecentScripts.Enabled = mnuRecentScripts.DropDownItems.Count > 0;
		}

		private string ScriptName
		{
			get
			{
				if(_filePath != null) {
					return Path.GetFileName(_filePath);
				} else if(_builtInScriptName != null) {
					return _builtInScriptName;
				} else {
					return "unnamed.lua";
				}
			}
		}

		private void RunScript()
		{
			if(_filePath != null && mnuSaveBeforeRun.Checked && txtScriptContent.UndoEnabled) {
				txtScriptContent.SaveToFile(_filePath, Encoding.UTF8);
			}

			_scriptId = DebugApi.LoadScript(ScriptName, txtScriptContent.Text, _scriptId);
			if(_scriptId < 0) {
				MessageBox.Show("Error while loading script.");
			} else {
				lblScriptActive.Visible = true;
			}
		}

		private void StopScript()
		{
			if(_scriptId >= 0) {
				DebugApi.RemoveScript(_scriptId);
				lblScriptActive.Visible = false;
				_scriptId = -1;
			}
		}

		private bool SaveScript()
		{
			if(_filePath != null && txtScriptContent.UndoEnabled) {
				txtScriptContent.SaveToFile(_filePath, Encoding.UTF8);
				_originalText = txtScriptContent.Text;
				return true;
			} else {
				return SaveAs("NewScript.lua");
			}
		}

		private bool SaveAs(string newName)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.FileName = newName;
				if(ConfigManager.Config.Debug.ScriptWindow.RecentScripts.Count > 0) {
					sfd.InitialDirectory = Path.GetDirectoryName(ConfigManager.Config.Debug.ScriptWindow.RecentScripts[0]);
				}
				sfd.SetFilter("Lua scripts (*.lua)|*.lua");
				if(sfd.ShowDialog() == DialogResult.OK) {
					SetFilePath(sfd.FileName);
					txtScriptContent.SaveToFile(_filePath, Encoding.UTF8);
					ConfigManager.Config.Debug.ScriptWindow.AddRecentScript(sfd.FileName);
					UpdateRecentScripts();
					_originalText = txtScriptContent.Text;
					_builtInScriptName = null;
					return true;
				}
			}
			return false;
		}

		private void SetFilePath(string filePath)
		{
			_filePath = filePath;
			this.Text = $"{Path.GetFileName(_filePath)} - Script Window";
		}

		private void mnuOpen_Click(object sender, EventArgs e)
		{
			LoadScript();
		}

		private void mnuClose_Click(object sender, EventArgs e)
		{
			this.Close();
		}

		private void mnuRun_Click(object sender, EventArgs e)
		{
			RunScript();
		}

		private void mnuSave_Click(object sender, EventArgs e)
		{
			SaveScript();
		}

		private void mnuSaveAs_Click(object sender, EventArgs e)
		{
			SaveAs(Path.GetFileName(this._filePath));
		}

		private void mnuStop_Click(object sender, EventArgs e)
		{
			StopScript();
		}

		private void tmrLog_Tick(object sender, EventArgs e)
		{
			tmrLog.Stop();
			if(_scriptId >= 0) {
				string newLog = DebugApi.GetScriptLog(_scriptId);
				if(txtLog.Text != newLog) {
					txtLog.SuspendLayout();
					txtLog.Text = newLog;
					txtLog.SelectionStart = newLog.Length;
					txtLog.ScrollToCaret();
					txtLog.ResumeLayout();
				}
			}

			mnuSave.Enabled = txtScriptContent.Text != _originalText;

			if(_filePath != null && File.Exists(_filePath) && mnuAutoReload.Checked) {
				if(_lastTimestamp < File.GetLastWriteTime(_filePath)) {
					if(_originalText != txtScriptContent.Text) {
						DialogResult result = MessageBox.Show("You have unsaved changes for this script and the file has been modified by another process - would you like to discard the changes and reload the script from the disk?", "Script Window", MessageBoxButtons.YesNo);
						if(result == DialogResult.Yes) {
							LoadScriptFile(_filePath, true, true);
						}
					} else {
						LoadScriptFile(_filePath, true, true);
					}
					_lastTimestamp = File.GetLastWriteTime(_filePath);
				}
			}
			tmrLog.Start();
		}

		private void mnuView_DropDownOpening(object sender, EventArgs e)
		{
			mnuShowLogWindow.Checked = ctrlSplit.Panel2.Height > 5;
		}

		private void mnuShowLogWindow_Click(object sender, EventArgs e)
		{
			if(mnuShowLogWindow.Checked) {
				ctrlSplit.ExpandPanel();
			} else {
				ctrlSplit.CollapsePanel();
			}
		}

		private void mnuCopy_Click(object sender, EventArgs e)
		{
			txtScriptContent.Copy();
		}

		private void mnuCut_Click(object sender, EventArgs e)
		{
			txtScriptContent.Cut();
		}

		private void mnuPaste_Click(object sender, EventArgs e)
		{
			txtScriptContent.Paste();
		}
		
		private void mnuSelectAll_Click(object sender, EventArgs e)
		{
			txtScriptContent.SelectAll();
		}

		private void mnuIncreaseFontSize_Click(object sender, EventArgs e)
		{
			txtScriptContent.Zoom += 10;
		}

		private void mnuDecreaseFontSize_Click(object sender, EventArgs e)
		{
			txtScriptContent.Zoom -= 10;
		}

		private void mnuResetFontSize_Click(object sender, EventArgs e)
		{
			txtScriptContent.Zoom = 100;
		}
		
		private void mnuSelectFont_Click(object sender, EventArgs e)
		{
			txtScriptContent.Font = FontDialogHelper.SelectFont(txtScriptContent.OriginalFont);
			txtScriptContent.Zoom = txtScriptContent.Zoom;
		}

		private void mnuNewScript_Click(object sender, EventArgs e)
		{
			frmScript frm = DebugWindowManager.OpenDebugWindow(DebugWindow.ScriptWindow) as frmScript;
			frm.LoadBuiltInScript("");
		}

		private void mnuAutoLoadLastScript_Click(object sender, EventArgs e)
		{
			mnuAutoLoadLastScript.Checked = true;
			mnuTutorialScript.Checked = false;
			mnuBlankWindow.Checked = false;
		}

		private void mnuTutorialScript_Click(object sender, EventArgs e)
		{
			mnuAutoLoadLastScript.Checked = false;
			mnuTutorialScript.Checked = true;
			mnuBlankWindow.Checked = false;
		}

		private void mnuBlankWindow_Click(object sender, EventArgs e)
		{
			mnuAutoLoadLastScript.Checked = false;
			mnuTutorialScript.Checked = false;
			mnuBlankWindow.Checked = true;
		}
		
		private void mnuApiReference_Click(object sender, EventArgs e)
		{
			Process.Start("https://www.mesen.ca/ApiReference.php");
		}

		private void mnuSetScriptTimeout_Click(object sender, EventArgs e)
		{
			//TODO
			/*using(frmSetScriptTimeout frm = new frmSetScriptTimeout()) {
				frm.ShowDialog();
			}*/
		}

		static readonly List<List<string>> _availableFunctions = new List<List<string>>() {
			new List<string> {"enum", "emu", "", "", "", "", "" },
			new List<string> {"func","emu.addEventCallback","emu.addEventCallback(function, type)","function - A Lua function.\ntype - *Enum* See eventType.","Returns an integer value that can be used to remove the callback by calling removeEventCallback.","Registers a callback function to be called whenever the specified event occurs.",},
			new List<string> {"func","emu.removeEventCallback","emu.removeEventCallback(reference, type)","reference - The value returned by the call to addEventCallback.\ntype - *Enum* See eventType.","","Removes a previously registered callback function.",},
			new List<string> {"func","emu.addMemoryCallback","emu.addMemoryCallback(function, type, startAddress, endAddress)","function - A Lua function.\ntype - *Enum* See memCallbackType\nstartAddress - *Integer* Start of the CPU memory address range to register the callback on.\nendAddress - (optional) *Integer* End of the CPU memory address range to register the callback on.","Returns an integer value that can be used to remove the callback by callingremoveMemoryCallback.","Registers a callback function to be called whenever the specified event occurs."},
			new List<string> {"func","emu.removeMemoryCallback","emu.removeMemoryCallback(reference, type, startAddress, endAddress)","reference - The value returned by the call to addMemoryCallback.\ntype - *Enum* See memCallbackType.\nstartAddress - *Integer* Start of the CPU memory address range to unregister the callback from.\nendAddress - (optional) *Integer* End of the CPU memory address range to unregister the callback from.","","Removes a previously registered callback function."},
			new List<string> {"func","emu.read","emu.read(address, type, signed)","address - *Integer* The address/offset to read from.\ntype - *Enum* The type of memory to read from. See memType.\nsigned - (optional) *Boolean* If true, the value returned will be interpreted as a signed value.","An 8-bit (read) or 16-bit (readWord) value.","Reads a value from the specified memory type.\n\nWhen calling read / readWord with the memType.cpu or memType.ppu memory types, emulation side-effects may occur.\nTo avoid triggering side-effects, use the memType.cpuDebug or memType.ppuDebug types, which will not cause side-effects."},
			new List<string> {"func","emu.readWord","emu.readWord(address, type, signed)","address - *Integer* The address/offset to read from.\ntype - *Enum* The type of memory to read from. See memType.\nsigned - (optional) *Boolean* If true, the value returned will be interpreted as a signed value.","An 8-bit (read) or 16-bit (readWord) value.","Reads a value from the specified memory type.\n\nWhen calling read / readWord with the memType.cpu or memType.ppu memory types, emulation side-effects may occur.\nTo avoid triggering side-effects, use the memType.cpuDebug or memType.ppuDebug types, which will not cause side-effects."},
			new List<string> {"func","emu.write","emu.write(address, value, type)","address - *Integer* The address/offset to write to.\nvalue - *Integer* The value to write.\ntype - *Enum* The type of memory to write to. See memType.","","Writes an 8-bit or 16-bit value to the specified memory type.\n\nNormally read-only types such as PRG-ROM or CHR-ROM can be written to when using memType.prgRom or memType.chrRom.\nChanges will remain in effect until a power cycle occurs.\nTo revert changes done to ROM, see revertPrgChrChanges.\n\nWhen calling write / writeWord with the memType.cpu or memType.ppu memory types, emulation side-effects may occur.\nTo avoid triggering side-effects, use the memType.cpuDebug or memType.ppuDebug types, which will not cause side-effects."},
			new List<string> {"func","emu.writeWord","emu.writeWord(address, value, type)","address - *Integer* The address/offset to write to.\nvalue - *Integer* The value to write.\ntype - *Enum* The type of memory to write to. See memType.","","Writes an 8-bit or 16-bit value to the specified memory type.\n\nNormally read-only types such as PRG-ROM or CHR-ROM can be written to when using memType.prgRom or memType.chrRom.\nChanges will remain in effect until a power cycle occurs.\nTo revert changes done to ROM, see revertPrgChrChanges.\n\nWhen calling write / writeWord with the memType.cpu or memType.ppu memory types, emulation side-effects may occur.\nTo avoid triggering side-effects, use the memType.cpuDebug or memType.ppuDebug types, which will not cause side-effects."},
			//new List<string> {"func","emu.revertPrgChrChanges","emu.revertPrgChrChanges()","","","Reverts all modifications done to PRG-ROM and CHR-ROM via write/writeWord calls."},
			new List<string> {"func","emu.drawPixel","emu.drawPixel(x, y, color, duration, delay)","x - *Integer* X position\ny - *Integer* Y position\ncolor - *Integer* Color\nduration - *Integer* Number of frames to display (Default: 1 frame)\ndelay - *Integer* Number of frames to wait before drawing the pixel (Default: 0 frames)","","Draws a pixel at the specified (x, y) coordinates using the specified color for a specific number of frames."},
			new List<string> {"func","emu.drawLine","emu.drawLine(x, y, x2, y2, color, duration, delay)","x - *Integer* X position (start of line)\ny - *Integer* Y position (start of line)\nx2 - *Integer* X position (end of line)\ny2 - *Integer* Y position (end of line)\ncolor - *Integer* Color\nduration - *Integer* Number of frames to display (Default: 1 frame)\ndelay - *Integer* Number of frames to wait before drawing the line (Default: 0 frames)","","Draws a line between (x, y) to (x2, y2) using the specified color for a specific number of frames."},
			new List<string> {"func","emu.drawRectangle","emu.drawRectangle(x, y, width, height, color, fill, duration, delay)","x - *Integer* X position\ny - *Integer* Y position\nwidth - *Integer* The rectangle's width\nheight - *Integer* The rectangle's height\ncolor - *Integer* Color\nfill - *Boolean* Whether or not to draw an outline, or a filled rectangle.\nduration - *Integer* Number of frames to display (Default: 1 frame)\ndelay - *Integer* Number of frames to wait before drawing the rectangle (Default: 0 frames)","","Draws a rectangle between (x, y) to (x+width, y+height) using the specified color for a specific number of frames.\nIf *fill* is false, only the rectangle's outline will be drawn."},
			new List<string> {"func","emu.drawString","emu.drawString(x, y, text, textColor, backgroundColor, duration, delay)","x - *Integer* X position\ny - *Integer* Y position\ntext- *String* The text to display\ntextColor - *Integer* Color to use for the text\nbackgroundColor - *Integer* Color to use for the text's background color\nduration - *Integer* Number of frames to display (Default: 1 frame)\ndelay - *Integer* Number of frames to wait before drawing the text (Default: 0 frames)","","Draws text at (x, y) using the specified text and colors for a specific number of frames."},
			new List<string> {"func","emu.clearScreen","emu.clearScreen()","","","Removes all drawings from the screen."},
			new List<string> {"func","emu.getPixel","emu.getPixel(x, y)","x - *Integer* X position\ny - *Integer* Y position","*Integer* ARGB color","Returns the color (in ARGB format) of the PPU's output for the specified location."},
			new List<string> {"func","emu.displayMessage","emu.displayMessage(category, text)","category - *String* The category is the portion shown between brackets[]\ntext - *String* Text to show on the screen","","Displays a message on the main window in the format '[category] text'"},
			new List<string> {"func","emu.log","emu.log(text)","text - *String* Text to log","","Logs the given string in the script's log window - useful for debugging scripts."},
			new List<string> {"func","emu.getState","emu.getState()","","* Table* Current emulation state","Return a table containing information about the state of the CPU, PPU, APU and cartridge."},
			//new List<string> {"func","emu.setState","emu.setState(state)","state - *Table* A table containing the state of the emulation to apply.","","Updates the CPU and PPU's state.\nThe* state* parameter must be a table in the same format as the one returned by getState()\nNote: the state of the APU or cartridge cannot be modified by using setState()." },
			new List<string> {"func","emu.breakExecution","emu.breakExecution()","","","Breaks the execution of the game and displays the debugger window."},
			new List<string> {"func","emu.execute","emu.execute(count, type)","count - *Integer* The number of cycles or instructions to run before breaking\ntype - *Enum* See executeCountType","","Runs the emulator for the specified number of cycles/instructions and then breaks the execution."},
			new List<string> {"func","emu.reset","emu.reset()","","","Resets the current game."},
			new List<string> {"func","emu.resume","emu.resume()","","","Resumes execution after breaking."},
			new List<string> {"func","emu.rewind","emu.rewind(seconds)","seconds - *Integer* The number of seconds to rewind","","Instantly rewinds the emulation by the number of seconds specified.\n Note: this can only be called from within a 'StartFrame' event callback."},
			new List<string> {"func","emu.getLabelAddress","emu.getLabelAddress(label)","label - *String* The label to look up.","","Returns the integer address of the given label."},

			new List<string> {"func","emu.getScreenBuffer","emu.getScreenBuffer()","", "*Array* 32-bit integers in ARGB format", "Returns an array of ARGB values for the entire screen (256px by 240px) - can be used with emu.setScreenBuffer() to alter the frame."},
			new List<string> {"func","emu.setScreenBuffer","emu.setScreenBuffer(screenBuffer)", "screenBuffer - *Array* An array of 32-bit integers in ARGB format", "","Replaces the current frame with the contents of the specified array."},

			new List<string> {"func","emu.getAccessCounters","emu.getAccessCounters(counterMemType, counterOpType)", "counterMemType - *Enum* A value from the emu.counterMemType enum\ncounterOpType - *Enum* A value from the emu.counterOpType enum", "*Array* 32-bit integers", "Returns an array of access counters for the specified memory and operation types."},
			new List<string> {"func","emu.resetAccessCounters","emu.resetAccessCounters()", "", "", "Resets all access counters."},

			/*new List<string> {"func", "emu.getPrgRomOffset", "emu.getPrgRomOffset(address)", "address - *Integer* A CPU address (Valid range: $0000-$FFFF)", "*Integer* The corresponding byte offset in PRG ROM", "Returns an integer representing the byte offset of the specified CPU address in PRG ROM based on the mapper's current configuration.\nReturns -1 when the specified address is not mapped to PRG ROM."},
			new List<string> {"func", "emu.getChrRomOffset", "emu.getChrRomOffset(address)", "address - *Integer* A PPU address (Valid range: $0000-$3FFF)", "*Integer* The corresponding byte offset in CHR ROM", "Returns an integer representing the byte offset of the specified PPU address in CHR ROM based on the mapper's current configuration.\nReturns -1 when the specified address is not mapped to CHR ROM."},

			new List<string> {"func","emu.saveSavestate","emu.saveSavestate()","","*String* A string containing a binary blob representing the emulation's current state.","Creates a savestate and returns it as a binary string. (The savestate is not saved on disk)\n Note: this can only be called from within a “startFrame” event callback or “cpuExec” memory callback."},
			new List<string> {"func","emu.loadSavestate","emu.loadSavestate(savestate)","savestate - *String* A binary blob representing a savestate, as returned by saveSavestate()","","Loads the specified savestate.\nNote: this can only be called from within a “startFrame” event callback or “cpuExec” memory callback."},
			new List<string> {"func","emu.saveSavestateAsync","emu.saveSavestateAsync()","slotNumber - *Integer* A slot number to which the savestate data will be stored (slotNumber must be >= 0)","","Queues a save savestate request. As soon at the emulator is able to process the request, it will be saved into the specified slot.\nThis API is asynchronous because save states can only be taken in-between 2 CPU instructions, not in the middle of an instruction.\nWhen called while the CPU is in-between 2 instructions (e.g: inside the callback of an cpuExec or startFrame event),\nthe save state will be taken immediately and its data will be available via getSavestateData right after the call to saveSavestateAsync.\nThe savestate can be loaded by calling the loadSavestateAsync function."},
			new List<string> {"func","emu.loadSavestateAsync","emu.loadSavestateAsync()","slotNumber - *Integer* The slot number to load the savestate data from (must be a slot number that was used in a preceding saveSavestateAsync call)","*Boolean* Returns true if the slot number was valid.","Queues a load savestate request. As soon at the emulator is able to process the request, the savestate will be loaded from the specified slot.\nThis API is asynchronous because save states can only be loaded in-between 2 CPU instructions, not in the middle of an instruction.\nWhen called while the CPU is in-between 2 instructions (e.g: inside the callback of an cpuExec or startFrame event), the savestate will be loaded immediately."},
			new List<string> {"func","emu.getSavestateData","emu.getSavestateData()","slotNumber - *Integer* The slot number to get the savestate data from (must be a slot number that was used in a preceding saveSavestateAsync call)","*String* A binary string containing the savestate","Returns the savestate stored in the specified savestate slot."},
			new List<string> {"func","emu.clearSavestateData","emu.clearSavestateData()","slotNumber - *Integer* The slot number to get the savestate data from (must be a slot number that was used in a preceding saveSavestateAsync call)","","Clears the specified savestate slot (any savestate data in that slot will be removed from memory)."},
			*/
			new List<string> {"func","emu.getInput","emu.getInput(port)","port - *Integer* The port number to read (0 to 3)","*Table* A table containing the status of all 8 buttons.","Returns a table containing the status of all 8 buttons: { a, b, select, start, up, down, left, right }"},
			//new List<string> {"func","emu.setInput","emu.setInput(port, input)","port - *Integer* The port number to apply the input to (0 to 3)\ninput - *Table* A table containing the state of some (or all) of the 8 buttons (same format as returned by getInput)","","Buttons enabled or disabled via setInput will keep their state until the next inputPolled event.\nIf a button’s value is not specified to either true or false in the input argument, then the player retains control of that button.\nFor example, setInput(0, { select = false, start = false}) will prevent the player 1 from using both the start and select buttons,\nbut all other buttons will still work as normal.To properly control the emulation, it is recommended to use this function\nwithin a callback for the inputPolled event.\nOtherwise, the inputs may not be applied before the ROM has the chance to read them."},
			new List<string> {"func","emu.getMouseState","emu.getMouseState()","","*Table* The mouse's state","Returns a table containing the position and the state of all 3 buttons: { x, y, left, middle, right }"},
			new List<string> {"func","emu.isKeyPressed","emu.isKeyPressed(keyName)","keyName - *String* The name of the key to check","*Boolean* The key’s state (true = pressed)","Returns whether or not a specific key is pressed. The “keyName” must be the same as the string shown in the UI when the key is bound to a button."},
			//new List<string> {"func","emu.addCheat","emu.addCheat(cheatCode)","cheatCode - *String* A game genie format cheat code.","","Activates a game genie cheat code (6 or 8 characters).\nNote: cheat codes added via this function are not permanent and not visible in the UI."},
			//new List<string> {"func","emu.clearCheats","emu.clearCheats()","","","Removes all active cheat codes (has no impact on cheat codes saved within the UI)"},
			new List<string> {"func","emu.getLogWindowLog","emu.getLogWindowLog()","","*Table* A string containing the log shown in the log window","Returns the same text as what is shown in the emulator's Log Window."},
			new List<string> {"func","emu.getRomInfo","emu.getRomInfo()","","*Table* Information about the current ROM","Returns information about the ROM file that is currently running."},
			new List<string> {"func","emu.getScriptDataFolder","emu.getScriptDataFolder()","","*String* The script’s data folder.","This function returns the path to a unique folder (based on the script’s filename) where the script should store its data (if any data needs to be saved).\nThe data will be saved in subfolders inside the LuaScriptData folder in Mesen’s home folder."},
			new List<string> {"func","emu.takeScreenshot","emu.takeScreenshot()","","*String* A binary string containing a PNG image.","Takes a screenshot and returns a PNG file as a string.\nThe screenshot is not saved to the disk."},
			new List<string> {"enum","emu.eventType","emu.eventType.[value]","","","Values:\nreset = 0,\nnmi = 1,\nirq = 2,\nstartFrame = 3,\nendFrame = 4,\ncodeBreak = 5\nstateLoaded = 6,\nstateSaved = 7,\ninputPolled = 8,\nspriteZeroHit = 9,\nscriptEnded = 10\n\nUsed by addEventCallback / removeEventCallback calls."},
			new List<string> {"enum","emu.eventType.reset","Triggered when a soft reset occurs","","",""},
			new List<string> {"enum","emu.eventType.nmi","Triggered when an nmi occurs","","",""},
			new List<string> {"enum","emu.eventType.irq","Triggered when an irq occurs","","",""},
			new List<string> {"enum","emu.eventType.startFrame","Triggered at the start of a frame (cycle 0, scanline 0)","","",""},
			new List<string> {"enum","emu.eventType.endFrame","Triggered at the end of a frame (cycle 0 on the NMI scanline)","","",""},
			//new List<string> {"enum","emu.eventType.codeBreak","Triggered when code execution breaks (e.g due to a breakpoint, etc.)","","",""},
			new List<string> {"enum","emu.eventType.stateLoaded","Triggered when a user manually loads a savestate","","",""},
			new List<string> {"enum","emu.eventType.stateSaved","Triggered when a user manually saves a savestate","","",""},
			new List<string> {"enum","emu.eventType.inputPolled","Triggered when the emulation core polls the state of the input devices for the next frame","","",""},
			new List<string> {"enum","emu.eventType.scriptEnded","Triggered when the current Lua script ends (script window closed, execution stopped, etc.)","","",""},
			new List<string> {"enum","emu.executeCountType","emu.executeCountType.[value]","","","Values:\ncpuCycles = 0,\nppuCycles = 1,\ncpuInstructions = 2\n\nUsed by execute calls." },
			new List<string> {"enum","emu.executeCountType.cpuCycles","Count the number of CPU cycles","","",""},
			new List<string> {"enum","emu.executeCountType.ppuCycles","Count the number of PPU cycles","","",""},
			new List<string> {"enum","emu.executeCountType.cpuInstructions","Count the number of CPU instructions","","",""},
			new List<string> {"enum","emu.memCallbackType","emu.memCallbackType.[value]","","","Values:\nread,\nwrite,\nexec\n\nUsed by addMemoryCallback / removeMemoryCallback calls."},
			new List<string> {"enum","emu.memCallbackType.read","Triggered when a byte is read ","","",""},
			new List<string> {"enum","emu.memCallbackType.write","Triggered when a byte is written","","",""},
			new List<string> {"enum","emu.memCallbackType.exec","Triggered when any memory read occurs due to the CPU's code execution","","",""},
			new List<string> {"enum","emu.memType","emu.memType.[value]", "","",""},
			new List<string> {"enum","emu.memType.cpu","CPU memory - Max: $FFFFFF","","","Warning: Reading or writing to this memory type may cause side-effects!"},
			new List<string> {"enum","emu.memType.spc","SPC memory - Max: $FFFF","","","Warning: Reading or writing to this memory type may cause side-effects!"},
			new List<string> {"enum","emu.memType.cgram","CG RAM - Max: $1FF","","",""},
			new List<string> {"enum","emu.memType.oam","OAM memory - Max: $21F","","",""},
			new List<string> {"enum","emu.memType.vram","Video RAM - Max: $FFFF","","",""},
			new List<string> {"enum","emu.memType.prgRom","PRG ROM - Range varies by game","","",""},
			new List<string> {"enum","emu.memType.workRam","Work RAM - Max: $1FFFF","","",""},
			new List<string> {"enum","emu.memType.saveRam","Save RAM - Range varies by game","","",""},
			new List<string> {"enum","emu.memType.cpuDebug","CPU memory - Max: $FFFFFF","","","Same as \"memType.cpu\" but does NOT cause any side-effects."},
			new List<string> {"enum","emu.memType.spcDebug", "SPC memory - Max: $FFFF", "","","Same as \"memType.spc\" but does NOT cause any side-effects."},
			new List<string> {"enum","emu.counterMemType","emu.counterMemType.[value]","","","Values:\nprgRom,\nworkRam,\nsaveRam\n\nUsed by getAccessCounters calls."},
			new List<string> {"enum","emu.counterMemType.prgRom","Returns access counter data for PRG ROM","","",""},
			new List<string> {"enum","emu.counterMemType.workRam", "Returns access counter data for Work RAM", "","",""},
			new List<string> {"enum","emu.counterMemType.saveRam", "Returns access counter data for Save RAM", "","",""},
			new List<string> {"enum","emu.counterOpType","emu.counterOpType.[value]","","","Values:\nread,\nwrite,\nexec\n\nUsed by getAccessCounters calls."},
			new List<string> {"enum","emu.counterOpType.read","Returns access counter data for reads","","",""},
			new List<string> {"enum","emu.counterOpType.write","Returns access counter data for writes","","",""},
			new List<string> {"enum","emu.counterOpType.exec", "Returns access counter data for executed bytes", "","",""},
		};
	}

	public class MethodAutocompleteItem : AutocompleteItem
	{
		string firstPart;
		string lastPart;

		public MethodAutocompleteItem(string text) : base(text)
		{
			var i = text.LastIndexOf('.');
			if(i < 0) {
				firstPart = text;
			} else {
				firstPart = text.Substring(0, i);
				lastPart = text.Substring(i + 1);
			}
		}

		public override CompareResult Compare(string fragmentText)
		{
			int i = fragmentText.LastIndexOf('.');

			if(i < 0) {
				if(firstPart.StartsWith(fragmentText) && string.IsNullOrEmpty(lastPart)) {
					return CompareResult.VisibleAndSelected;
				}
				//if (firstPart.ToLower().Contains(fragmentText.ToLower()))
				//  return CompareResult.Visible;
			} else {
				var fragmentFirstPart = fragmentText.Substring(0, i);
				var fragmentLastPart = fragmentText.Substring(i + 1);

				if(firstPart != fragmentFirstPart) {
					return CompareResult.Hidden;
				}

				if(lastPart != null && lastPart.StartsWith(fragmentLastPart)) {
					return CompareResult.VisibleAndSelected;
				}

				if(lastPart != null && lastPart.ToLower().Contains(fragmentLastPart.ToLower())) {
					return CompareResult.Visible;
				}

			}

			return CompareResult.Hidden;
		}

		public override string GetTextForReplace()
		{
			if(lastPart == null) {
				return firstPart;
			}

			return firstPart + "." + lastPart;
		}

		public override string ToString()
		{
			if(lastPart == null) {
				return firstPart;
			}

			return lastPart;
		}
	}
}
