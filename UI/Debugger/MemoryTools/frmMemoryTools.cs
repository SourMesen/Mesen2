using System;
using System.Drawing;
using System.IO;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Forms;
using Mesen.GUI.Debugger.Controls;
using System.Collections.Generic;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Debugger.Labels;

namespace Mesen.GUI.Debugger
{
	public partial class frmMemoryTools : BaseForm
	{
		private EntityBinder _entityBinder = new EntityBinder();
		private NotificationListener _notifListener;
		private SnesMemoryType _memoryType = SnesMemoryType.CpuMemory;
		private bool _updating = false;
		private DateTime _lastUpdate = DateTime.MinValue;
		private bool _formClosed;

		public frmMemoryTools()
		{
			InitializeComponent();
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			HexEditorInfo config = ConfigManager.Config.Debug.HexEditor;
			_entityBinder.Entity = config;
			_entityBinder.AddBinding(nameof(config.AutoRefresh), mnuAutoRefresh);
			_entityBinder.AddBinding(nameof(config.HighDensityTextMode), mnuHighDensityMode);
			_entityBinder.AddBinding(nameof(config.ByteEditingMode), mnuByteEditingMode);
			_entityBinder.AddBinding(nameof(config.EnablePerByteNavigation), mnuEnablePerByteNavigation);
			_entityBinder.AddBinding(nameof(config.IgnoreRedundantWrites), mnuIgnoreRedundantWrites);
			_entityBinder.AddBinding(nameof(config.HighlightCurrentRowColumn), mnuHighlightCurrentRowColumn);
			_entityBinder.AddBinding(nameof(config.ShowCharacters), mnuShowCharacters);
			_entityBinder.AddBinding(nameof(config.ShowLabelInfo), mnuShowLabelInfoOnMouseOver);

			_entityBinder.AddBinding(nameof(config.HighlightExecution), mnuHighlightExecution);
			_entityBinder.AddBinding(nameof(config.HighlightReads), mnuHightlightReads);
			_entityBinder.AddBinding(nameof(config.HighlightWrites), mnuHighlightWrites);
			_entityBinder.AddBinding(nameof(config.HideUnusedBytes), mnuHideUnusedBytes);
			_entityBinder.AddBinding(nameof(config.HideReadBytes), mnuHideReadBytes);
			_entityBinder.AddBinding(nameof(config.HideWrittenBytes), mnuHideWrittenBytes);
			_entityBinder.AddBinding(nameof(config.HideExecutedBytes), mnuHideExecutedBytes);

			_entityBinder.AddBinding(nameof(config.HighlightLabelledBytes), mnuHighlightLabelledBytes);
			_entityBinder.AddBinding(nameof(config.HighlightBreakpoints), mnuHighlightBreakpoints);
			_entityBinder.AddBinding(nameof(config.HighlightCodeBytes), mnuHighlightCodeBytes);
			_entityBinder.AddBinding(nameof(config.HighlightDataBytes), mnuHighlightDataBytes);

			_entityBinder.UpdateUI();

			UpdateRefreshSpeedMenu();
			UpdateFlags();

			this.ctrlHexViewer.HighlightCurrentRowColumn = config.HighlightCurrentRowColumn;
			this.ctrlHexViewer.TextZoom = config.TextZoom;
			this.ctrlHexViewer.BaseFont = new Font(config.FontFamily, config.FontSize, config.FontStyle);

			//TODO this.ctrlMemoryAccessCounters.BaseFont = new Font(config.FontFamily, config.FontSize, config.FontStyle);
			//TODO this.ctrlMemoryAccessCounters.TextZoom = config.TextZoom;

			this.UpdateFadeOptions();

			this.InitTblMappings();

			this.ctrlHexViewer.StringViewVisible = mnuShowCharacters.Checked;
			//TODO
			//this.ctrlHexViewer.MemoryViewer = this;

			UpdateImportButton();
			InitMemoryTypeDropdown(true);

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			this.mnuShowCharacters.CheckedChanged += this.mnuShowCharacters_CheckedChanged;
			this.mnuIgnoreRedundantWrites.CheckedChanged += mnuIgnoreRedundantWrites_CheckedChanged;

			if(!config.WindowSize.IsEmpty) {
				this.StartPosition = FormStartPosition.Manual;
				this.Size = config.WindowSize;
				this.Location = config.WindowLocation;
			}

			this.InitShortcuts();
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);
			ctrlHexViewer.Focus();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);

			HexEditorInfo config = ConfigManager.Config.Debug.HexEditor;
			config.TextZoom = this.ctrlHexViewer.TextZoom;
			config.FontFamily = ctrlHexViewer.BaseFont.FontFamily.Name;
			config.FontStyle = ctrlHexViewer.BaseFont.Style;
			config.FontSize = ctrlHexViewer.BaseFont.Size;
			config.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			config.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			config.MemoryType = cboMemoryType.GetEnumValue<SnesMemoryType>();
			_entityBinder.UpdateObject();
			ConfigManager.ApplyChanges();
			
			if(this._notifListener != null) {
				this._notifListener.Dispose();
				this._notifListener = null;
			}

			_formClosed = true;
		}

		private void InitShortcuts()
		{
			mnuIncreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.IncreaseFontSize));
			mnuDecreaseFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.DecreaseFontSize));
			mnuResetFontSize.InitShortcut(this, nameof(DebuggerShortcutsConfig.ResetFontSize));

			mnuImport.InitShortcut(this, nameof(DebuggerShortcutsConfig.MemoryViewer_Import));
			mnuExport.InitShortcut(this, nameof(DebuggerShortcutsConfig.MemoryViewer_Export));

			mnuRefresh.InitShortcut(this, nameof(DebuggerShortcutsConfig.Refresh));

			mnuGoToAll.InitShortcut(this, nameof(DebuggerShortcutsConfig.GoToAll));
			mnuGoTo.InitShortcut(this, nameof(DebuggerShortcutsConfig.GoTo));
			mnuFind.InitShortcut(this, nameof(DebuggerShortcutsConfig.Find));
			mnuFindNext.InitShortcut(this, nameof(DebuggerShortcutsConfig.FindNext));
			mnuFindPrev.InitShortcut(this, nameof(DebuggerShortcutsConfig.FindPrev));
		}

		private void InitMemoryTypeDropdown(bool forStartup)
		{
			cboMemoryType.SelectedIndexChanged -= this.cboMemoryType_SelectedIndexChanged;

			SnesMemoryType originalValue = forStartup ? ConfigManager.Config.Debug.HexEditor.MemoryType : cboMemoryType.GetEnumValue<SnesMemoryType>();

			cboMemoryType.BeginUpdate();
			cboMemoryType.Items.Clear();

			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.CpuMemory));
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SpcMemory));
			cboMemoryType.Items.Add("-");
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.PrgRom));
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.WorkRam));
			if(DebugApi.GetMemorySize(SnesMemoryType.SaveRam) > 0) {
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SaveRam));
			}
			cboMemoryType.Items.Add("-");
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.VideoRam));
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.CGRam));
			cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.SpriteRam));

			if(DebugApi.GetMemorySize(SnesMemoryType.DspProgramRom) > 0) {
				cboMemoryType.Items.Add("-");
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.DspProgramRom));
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.DspDataRom));
				cboMemoryType.Items.Add(ResourceHelper.GetEnumText(SnesMemoryType.DspDataRam));
			}

			cboMemoryType.SelectedIndex = 0;
			cboMemoryType.SetEnumValue(originalValue);
			cboMemoryType.SelectedIndexChanged += this.cboMemoryType_SelectedIndexChanged;

			cboMemoryType.EndUpdate();
			UpdateMemoryType();
		}
		
		private void UpdateFlags()
		{
			//TODO
			/*
			if(mnuIgnoreRedundantWrites.Checked) {
				DebugWorkspaceManager.SetFlags(DebuggerFlags.IgnoreRedundantWrites);
			} else {
				DebugWorkspaceManager.ClearFlags(DebuggerFlags.IgnoreRedundantWrites);
			}
			*/
		}

		public void ShowAddress(int address, SnesMemoryType memoryType)
		{
			cboMemoryType.SetEnumValue(memoryType);
			ctrlHexViewer.GoToAddress(address);
		}

		//TODO
		/*
		public void GoToDestination(GoToDestination dest)
		{
			if(_memoryType == DebugMemoryType.CpuMemory && dest.CpuAddress >= 0) {
				this.ShowAddress(dest.CpuAddress, DebugMemoryType.CpuMemory);
			} else if(dest.AddressInfo != null) {
				this.ShowAddress(dest.AddressInfo.Address, dest.AddressInfo.Type.ToMemoryType());
			} else if(dest.Label != null) {
				int relAddress = dest.Label.GetRelativeAddress();
				if(_memoryType == DebugMemoryType.CpuMemory && relAddress >= 0) {
					this.ShowAddress(relAddress, DebugMemoryType.CpuMemory);
				} else {
					this.ShowAddress((int)dest.Label.Address, dest.Label.AddressType.ToMemoryType());
				}
			} else if(dest.CpuAddress >= 0) {
				this.ShowAddress(dest.CpuAddress, DebugMemoryType.CpuMemory);
			}

			this.BringToFront();
		}

		public void GoToAll()
		{
			using(frmGoToAll frm = new frmGoToAll(true, false)) {
				if(frm.ShowDialog() == DialogResult.OK) {
					GoToDestination(frm.Destination);
				}
			}
		}*/

		private void InitTblMappings()
		{
			DebugWorkspace workspace = DebugWorkspaceManager.GetWorkspace();
			if(workspace.TblMappings != null && workspace.TblMappings.Count > 0) {
				var tblDict = TblLoader.ToDictionary(workspace.TblMappings.ToArray());
				if(tblDict != null) {
					this.ctrlHexViewer.ByteCharConverter = new TblByteCharConverter(tblDict);
				}
			} else {
				this.ctrlHexViewer.ByteCharConverter = null;
			}
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.CodeBreak:
					this.BeginInvoke((MethodInvoker)(() => this.RefreshData()));
					break;
				
				case ConsoleNotificationType.GameReset:
				case ConsoleNotificationType.GameLoaded:
					this.BeginInvoke((Action)(() => {
						if(_formClosed) {
							return;
						}
						this.InitTblMappings();
						this.InitMemoryTypeDropdown(false);
						//TODO ctrlMemoryAccessCounters.InitMemoryTypeDropdown();
					}));
					this.UpdateFlags();
					break;

				case ConsoleNotificationType.PpuFrameDone:
					int refreshDelay = 90;
					switch(ConfigManager.Config.Debug.HexEditor.AutoRefreshSpeed) {
						case RefreshSpeed.Low: refreshDelay = 90; break;
						case RefreshSpeed.Normal: refreshDelay = 32; break;
						case RefreshSpeed.High: refreshDelay = 16; break;
					}

					DateTime now = DateTime.Now;
					if(!_updating && ConfigManager.Config.Debug.HexEditor.AutoRefresh && (now - _lastUpdate).Milliseconds >= refreshDelay) {
						_lastUpdate = now;
						_updating = true;
						this.BeginInvoke((Action)(() => {
							this.RefreshData();
							_updating = false;
						}));
					}
					break;
			}
		}

		private void cboMemoryType_SelectedIndexChanged(object sender, EventArgs e)
		{
			UpdateMemoryType();
		}

		private void UpdateMemoryType()
		{
			this._memoryType = this.cboMemoryType.GetEnumValue<SnesMemoryType>();
			this.UpdateImportButton();
			this.RefreshData();
		}

		private void UpdateByteColorProvider()
		{
			this.ctrlHexViewer.ByteColorProvider = new ByteColorProvider(
				this._memoryType,
				mnuHighlightExecution.Checked,
				mnuHighlightWrites.Checked,
				mnuHightlightReads.Checked,
				ConfigManager.Config.Debug.HexEditor.FadeSpeed,
				mnuHideUnusedBytes.Checked,
				mnuHideReadBytes.Checked,
				mnuHideWrittenBytes.Checked,
				mnuHideExecutedBytes.Checked,
				mnuHighlightDataBytes.Checked,
				mnuHighlightCodeBytes.Checked,
				mnuHighlightLabelledBytes.Checked,
				mnuHighlightBreakpoints.Checked
			);
		}

		private void mnuRefresh_Click(object sender, EventArgs e)
		{
			this.RefreshData();
		}

		private void RefreshData()
		{
			if(_formClosed) {
				return;
			}

			this.UpdateByteColorProvider();
			this.ctrlHexViewer.RefreshData(_memoryType);
		}

		private void mnuFind_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.OpenSearchBox();
		}

		private void mnuFindNext_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.FindNext();
		}

		private void mnuFindPrev_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.FindPrevious();
		}

		private void mnuGoTo_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.GoToAddress();
		}

		private void mnuGoToAll_Click(object sender, EventArgs e)
		{
			//TODO
			//this.GoToAll();
		}

		private void mnuIncreaseFontSize_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.TextZoom += 10;
			//TODO this.ctrlMemoryAccessCounters.TextZoom += 10;
		}

		private void mnuDecreaseFontSize_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.TextZoom -= 10;
			//TODO this.ctrlMemoryAccessCounters.TextZoom -= 10;
		}

		private void mnuResetFontSize_Click(object sender, EventArgs e)
		{
			this.ctrlHexViewer.TextZoom = 100;
			//TODO this.ctrlMemoryAccessCounters.TextZoom = 100;
		}

		private void mnuClose_Click(object sender, EventArgs e)
		{
			this.Close();
		}
		
		private void UpdateImportButton()
		{
			switch(_memoryType) {
				case SnesMemoryType.VideoRam:
				case SnesMemoryType.CGRam:
				case SnesMemoryType.SpriteRam:
				case SnesMemoryType.WorkRam:
				case SnesMemoryType.SaveRam:
					mnuImport.Enabled = true;
					break;

				default:
					mnuImport.Enabled = false;
					break;
			}
		}

		private void mnuImport_Click(object sender, EventArgs e)
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.SetFilter("Memory dump files (*.dmp)|*.dmp|All files (*.*)|*.*");
				ofd.InitialDirectory = ConfigManager.DebuggerFolder;
				if(ofd.ShowDialog() == DialogResult.OK) {
					byte[] fileContent = File.ReadAllBytes(ofd.FileName);
					DebugApi.SetMemoryState(_memoryType, fileContent, fileContent.Length);
					RefreshData();
				}
			}
		}

		private void mnuExport_Click(object sender, EventArgs e)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter("Memory dump files (*.dmp)|*.dmp|All files (*.*)|*.*");
				sfd.InitialDirectory = ConfigManager.DebuggerFolder;
				sfd.FileName = EmuApi.GetRomInfo().GetRomName() + " - " + cboMemoryType.SelectedItem.ToString() + ".dmp";
				sfd.FileName = cboMemoryType.SelectedItem.ToString() + ".dmp";
				if(sfd.ShowDialog() == DialogResult.OK) {
					File.WriteAllBytes(sfd.FileName, this.ctrlHexViewer.GetData());
				}
			}
		}

		private void tabMain_SelectedIndexChanged(object sender, EventArgs e)
		{
			this.RefreshData();
		}

		private void ctrlHexViewer_RequiredWidthChanged(object sender, EventArgs e)
		{
			this.Size = new Size(this.ctrlHexViewer.RequiredWidth + 20, this.Height);
		}

		private void mnuLoadTblFile_Click(object sender, EventArgs e)
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.SetFilter("TBL files (*.tbl)|*.tbl");
				if(ofd.ShowDialog() == DialogResult.OK) {
					string[] fileContents = File.ReadAllLines(ofd.FileName);
					var tblDict = TblLoader.ToDictionary(fileContents);
					if(tblDict == null) {
						MessageBox.Show("Could not load TBL file.  The file selected file appears to be invalid.", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
					} else {
						DebugWorkspaceManager.GetWorkspace().TblMappings = new List<string>(fileContents);
						this.ctrlHexViewer.ByteCharConverter = new TblByteCharConverter(tblDict);
						this.mnuShowCharacters.Checked = true;
						InitTblMappings();
					}
				}
			}
		}

		private void mnuResetTblMappings_Click(object sender, EventArgs e)
		{
			DebugWorkspaceManager.GetWorkspace().TblMappings = null;
			this.ctrlHexViewer.ByteCharConverter = null;
		}

		private void mnuShowCharacters_CheckedChanged(object sender, EventArgs e)
		{
			this.ctrlHexViewer.StringViewVisible = mnuShowCharacters.Checked;
		}

		private void mnuIgnoreRedundantWrites_CheckedChanged(object sender, EventArgs e)
		{
			this.UpdateFlags();
		}

		private void UpdateFadeOptions()
		{
			int fadeSpeed = ConfigManager.Config.Debug.HexEditor.FadeSpeed;
			mnuFadeSlow.Checked = fadeSpeed == 600;
			mnuFadeNormal.Checked = fadeSpeed == 300;
			mnuFadeFast.Checked = fadeSpeed == 120;
			mnuFadeNever.Checked = fadeSpeed == 0;
			mnuCustomFadeSpeed.Checked = !mnuFadeSlow.Checked && !mnuFadeNormal.Checked && !mnuFadeFast.Checked && !mnuFadeSlow.Checked;
		}

		private void mnuFadeSpeed_Click(object sender, EventArgs e)
		{
			HexEditorInfo config = ConfigManager.Config.Debug.HexEditor;
			if(sender == mnuFadeSlow) {
				config.FadeSpeed = 600;
			} else if(sender == mnuFadeNormal) {
				config.FadeSpeed = 300;
			} else if(sender == mnuFadeFast) {
				config.FadeSpeed = 120;
			} else if(sender == mnuFadeNever) {
				config.FadeSpeed = 0;
			}
			ConfigManager.ApplyChanges();
			UpdateFadeOptions();
		}

		private void mnuCustomFadeSpeed_Click(object sender, EventArgs e)
		{
			using(frmFadeSpeed frm = new frmFadeSpeed(ConfigManager.Config.Debug.HexEditor.FadeSpeed)) {
				if(frm.ShowDialog() == DialogResult.OK) {
					ConfigManager.Config.Debug.HexEditor.FadeSpeed = frm.FadeSpeed;
					ConfigManager.ApplyChanges();
					UpdateFadeOptions();
				}
			}
		}
		
		private void mnuColorProviderOptions_Click(object sender, EventArgs e)
		{
			this.UpdateByteColorProvider();
		}

		
		private void mnuConfigureColors_Click(object sender, EventArgs e)
		{
			using(frmMemoryToolsColors frm = new frmMemoryToolsColors()) {
				if(frm.ShowDialog(this, this) == DialogResult.OK) {
					this.RefreshData();
				}
			}
		}

		private int _lastTooltipAddress = -1;
		private void ctrlHexViewer_ByteMouseHover(int address, Point position)
		{
			ctrlTooltip tooltip = BaseForm.GetPopupTooltip(this);
			if(address < 0 || !mnuShowLabelInfoOnMouseOver.Checked) {
				_lastTooltipAddress = -1;
				tooltip.Hide();
				return;
			} else if(_lastTooltipAddress == address) {
				return;
			}

			_lastTooltipAddress = address;

			CodeLabel label = null;
			int arrayOffset = 0;
			switch(_memoryType) {
				case SnesMemoryType.CpuMemory:
				case SnesMemoryType.SpcMemory:
					AddressInfo relAddress = new AddressInfo() {
						Address = address,
						Type = _memoryType
					};
					AddressInfo absAddress = DebugApi.GetAbsoluteAddress(relAddress);
					if(absAddress.Address >= 0) {
						label = LabelManager.GetLabel((uint)absAddress.Address, absAddress.Type);
						if(label != null) {
							arrayOffset = relAddress.Address - (int)label.Address;
						}
					}
					break;

				case SnesMemoryType.WorkRam:
				case SnesMemoryType.SaveRam:
				case SnesMemoryType.PrgRom:
				case SnesMemoryType.SpcRam:
				case SnesMemoryType.SpcRom:
					label = LabelManager.GetLabel((uint)address, _memoryType);
					if(label != null) {
						arrayOffset = address - (int)label.Address;
					}
					break;
			}

			if(label != null && !string.IsNullOrWhiteSpace(label.Label)) {
				Dictionary<string, string> values = new Dictionary<string, string>();
				if(!string.IsNullOrWhiteSpace(label.Label)) {
					values["Label"] = label.Label;
					if(label.Length > 1) {
						values["Label"] += "+" + arrayOffset.ToString();
					}
				}
				values["Address"] = "$" + (label.Address + arrayOffset).ToString("X4");
				values["Type"] = ResourceHelper.GetEnumText(label.MemoryType);
				if(!string.IsNullOrWhiteSpace(label.Comment)) {
					values["Comment"] = label.Comment;
				}

				tooltip.SetTooltip(this.PointToClient(position), values);
			} else {
				tooltip.Hide();
			}
		}
		
		private void mnuAutoRefreshSpeed_Click(object sender, EventArgs e)
		{
			HexEditorInfo config = ConfigManager.Config.Debug.HexEditor;

			if(sender == mnuAutoRefreshLow) {
				config.AutoRefreshSpeed = RefreshSpeed.Low;
			} else if(sender == mnuAutoRefreshNormal) {
				config.AutoRefreshSpeed = RefreshSpeed.Normal;
			} else if(sender == mnuAutoRefreshHigh) {
				config.AutoRefreshSpeed = RefreshSpeed.High;
			}
			ConfigManager.ApplyChanges();

			UpdateRefreshSpeedMenu();
		}

		private void UpdateRefreshSpeedMenu()
		{
			HexEditorInfo config = ConfigManager.Config.Debug.HexEditor;
			mnuAutoRefreshLow.Checked = config.AutoRefreshSpeed == RefreshSpeed.Low;
			mnuAutoRefreshNormal.Checked = config.AutoRefreshSpeed == RefreshSpeed.Normal;
			mnuAutoRefreshHigh.Checked = config.AutoRefreshSpeed == RefreshSpeed.High;
		}

		private void mnuHighDensityMode_CheckedChanged(object sender, EventArgs e)
		{
			ctrlHexViewer.HighDensityMode = mnuHighDensityMode.Checked;
		}

		private void mnuEnablePerByteNavigation_CheckedChanged(object sender, EventArgs e)
		{
			ctrlHexViewer.EnablePerByteNavigation = mnuEnablePerByteNavigation.Checked;
		}

		private void mnuSelectFont_Click(object sender, EventArgs e)
		{
			ctrlHexViewer.BaseFont = FontDialogHelper.SelectFont(ctrlHexViewer.BaseFont);
			//TODO ctrlMemoryAccessCounters.BaseFont = ctrlHexViewer.BaseFont;
		}

		private void mnuByteEditingMode_CheckedChanged(object sender, EventArgs e)
		{
			ctrlHexViewer.ByteEditingMode = mnuByteEditingMode.Checked;
		}

		private void mnuHighlightCurrentRowColumn_CheckedChanged(object sender, EventArgs e)
		{
			ctrlHexViewer.HighlightCurrentRowColumn = mnuHighlightCurrentRowColumn.Checked;
		}
	}
}
