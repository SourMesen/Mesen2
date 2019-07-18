using Mesen.GUI.Config;
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Debugger;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Emulation;
using Mesen.GUI.Forms.Config;
using Mesen.GUI.Updates;
using Mesen.GUI.Utilities;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms
{
	public partial class frmMain : BaseInputForm
	{
		private NotificationListener _notifListener;
		private ShortcutHandler _shortcuts;
		private DisplayManager _displayManager;
		private CommandLineHelper _commandLine;

		public frmMain(string[] args)
		{
			InitializeComponent();
			if(DesignMode) {
				return;
			}

			_commandLine = new CommandLineHelper(args);

			ResourceHelper.LoadResources(Language.English);
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			if(!ConfigManager.Config.WindowSize.IsEmpty) {
				this.StartPosition = FormStartPosition.Manual;
				this.Location = ConfigManager.Config.WindowLocation;
				this.Size = ConfigManager.Config.WindowSize;
			}
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			EmuApi.InitDll();
			ConfigManager.Config.Video.ApplyConfig();
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, Handle, ctrlRenderer.Handle, false, false, false);

			ConfigManager.Config.InitializeDefaults();
			ConfigManager.Config.ApplyConfig();

			_displayManager = new DisplayManager(this, ctrlRenderer, pnlRenderer, mnuMain, ctrlRecentGames);
			_displayManager.SetScaleBasedOnWindowSize();
			_shortcuts = new ShortcutHandler(_displayManager);

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;
			
			_commandLine.LoadGameFromCommandLine();

			Task.Run(() => {
				System.Threading.Thread.Sleep(25);
				this.BeginInvoke((Action)(() => {
					SaveStateManager.InitializeStateMenu(mnuSaveState, true, _shortcuts);
					SaveStateManager.InitializeStateMenu(mnuLoadState, false, _shortcuts);

					BindShortcuts();

					ResizeRecentGames();
					ctrlRecentGames.Initialize();

					if(!EmuRunner.IsRunning()) {
						ctrlRecentGames.Visible = true;
					}
				}));
			});

			if(ConfigManager.Config.Preferences.AutomaticallyCheckForUpdates) {
				UpdateHelper.CheckForUpdates(true);
			}

			InBackgroundHelper.StartBackgroundTimer();
			this.Resize += frmMain_Resize;
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);

			DebugApi.ResumeExecution();
			DebugWindowManager.CloseAll();

			EmuApi.Stop();

			if(_notifListener != null) {
				_notifListener.Dispose();
				_notifListener = null;
			}

			ConfigManager.Config.WindowLocation = this.WindowState == FormWindowState.Normal ? this.Location : this.RestoreBounds.Location;
			ConfigManager.Config.WindowSize = this.WindowState == FormWindowState.Normal ? this.Size : this.RestoreBounds.Size;
			ConfigManager.ApplyChanges();

			EmuApi.Release();
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					this.BeginInvoke((Action)(() => {
						UpdateDebuggerMenu();
						ctrlRecentGames.Visible = false;
						SaveStateManager.UpdateStateMenu(mnuLoadState, false);
						SaveStateManager.UpdateStateMenu(mnuSaveState, true);

						RomInfo romInfo = EmuApi.GetRomInfo();
						this.Text = "Mesen-S - " + romInfo.GetRomName();

						if(DebugWindowManager.HasOpenedWindow) {
							DebugWorkspaceManager.GetWorkspace();
						}
					}));
					break;

				case ConsoleNotificationType.BeforeEmulationStop:
					this.Invoke((Action)(() => {
						DebugWindowManager.CloseAll();
					}));
					break;

				case ConsoleNotificationType.EmulationStopped:
					this.BeginInvoke((Action)(() => {
						this.Text = "Mesen-S";
						UpdateDebuggerMenu();
						ctrlRecentGames.Initialize();
						ctrlRecentGames.Visible = true;
						ResizeRecentGames();
						if(_displayManager.ExclusiveFullscreen) {
							_displayManager.SetFullscreenState(false);
						}
					}));
					break;

				case ConsoleNotificationType.ResolutionChanged:
					this.BeginInvoke((Action)(() => {
						_displayManager.UpdateViewerSize();
					}));
					break;

				case ConsoleNotificationType.ExecuteShortcut:
					this.BeginInvoke((Action)(() => {
						_shortcuts.ExecuteShortcut((EmulatorShortcut)e.Parameter);
					}));
					break;
			}
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(_displayManager.HideMenuStrip && (keyData & Keys.Alt) == Keys.Alt) {
				if(mnuMain.Visible && !mnuMain.ContainsFocus) {
					mnuMain.Visible = false;
				} else {
					mnuMain.Visible = true;
					mnuMain.Focus();
				}
			}
			return base.ProcessCmdKey(ref msg, keyData);
		}

		private void BindShortcuts()
		{
			Func<bool> notClient = () => { return true; }; //TODO
			Func<bool> running = () => { return EmuRunner.IsRunning(); };
			Func<bool> runningNotClient = () => { return EmuRunner.IsRunning(); }; //TODO
			Func<bool> runningNotClientNotMovie = () => { return EmuRunner.IsRunning(); }; //TODO

			_shortcuts.BindShortcut(mnuOpen, EmulatorShortcut.OpenFile);
			_shortcuts.BindShortcut(mnuExit, EmulatorShortcut.Exit);
			_shortcuts.BindShortcut(mnuIncreaseSpeed, EmulatorShortcut.IncreaseSpeed, notClient);
			_shortcuts.BindShortcut(mnuDecreaseSpeed, EmulatorShortcut.DecreaseSpeed, notClient);
			_shortcuts.BindShortcut(mnuEmuSpeedMaximumSpeed, EmulatorShortcut.MaxSpeed, notClient);

			_shortcuts.BindShortcut(mnuPause, EmulatorShortcut.Pause, runningNotClient);
			_shortcuts.BindShortcut(mnuReset, EmulatorShortcut.Reset, runningNotClientNotMovie);
			_shortcuts.BindShortcut(mnuPowerCycle, EmulatorShortcut.PowerCycle, runningNotClientNotMovie);
			_shortcuts.BindShortcut(mnuPowerOff, EmulatorShortcut.PowerOff, runningNotClient);

			_shortcuts.BindShortcut(mnuShowFPS, EmulatorShortcut.ToggleFps);

			_shortcuts.BindShortcut(mnuScale1x, EmulatorShortcut.SetScale1x);
			_shortcuts.BindShortcut(mnuScale2x, EmulatorShortcut.SetScale2x);
			_shortcuts.BindShortcut(mnuScale3x, EmulatorShortcut.SetScale3x);
			_shortcuts.BindShortcut(mnuScale4x, EmulatorShortcut.SetScale4x);
			_shortcuts.BindShortcut(mnuScale5x, EmulatorShortcut.SetScale5x);
			_shortcuts.BindShortcut(mnuScale6x, EmulatorShortcut.SetScale6x);

			_shortcuts.BindShortcut(mnuFullscreen, EmulatorShortcut.ToggleFullscreen);

			_shortcuts.BindShortcut(mnuTakeScreenshot, EmulatorShortcut.TakeScreenshot, running);

			mnuDebugger.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenDebugger));
			mnuSpcDebugger.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenSpcDebugger));
			mnuMemoryTools.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenMemoryTools));
			mnuEventViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenEventViewer));
			mnuTilemapViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenTilemapViewer));
			mnuTileViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenTileViewer));
			mnuSpriteViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenSpriteViewer));
			mnuPaletteViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenPaletteViewer));
			mnuTraceLogger.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenTraceLogger));
			mnuScriptWindow.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenScriptWindow));

			mnuNoneFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.None); };
			mnuNtscFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.NTSC); };

			mnuHQ2xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.HQ2x); };
			mnuHQ3xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.HQ3x); };
			mnuHQ4xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.HQ4x); };

			mnuPrescale2xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Prescale2x); };
			mnuPrescale3xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Prescale3x); };
			mnuPrescale4xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Prescale4x); };
			mnuPrescale6xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Prescale6x); };
			mnuPrescale8xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Prescale8x); };
			mnuPrescale10xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Prescale10x); };

			mnuScale2xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Scale2x); };
			mnuScale3xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Scale3x); };
			mnuScale4xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Scale4x); };

			mnu2xSaiFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType._2xSai); };
			mnuSuper2xSaiFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.Super2xSai); };
			mnuSuperEagleFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.SuperEagle); };

			mnuXBRZ2xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.xBRZ2x); };
			mnuXBRZ3xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.xBRZ3x); };
			mnuXBRZ4xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.xBRZ4x); };
			mnuXBRZ5xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.xBRZ5x); };
			mnuXBRZ6xFilter.Click += (s, e) => { _shortcuts.SetVideoFilter(VideoFilterType.xBRZ6x); };

			mnuBilinearInterpolation.Click += (s, e) => { _shortcuts.ToggleBilinearInterpolation(); };

			mnuRegionAuto.Click += (s, e) => { _shortcuts.SetRegion(ConsoleRegion.Auto); };
			mnuRegionNtsc.Click += (s, e) => { _shortcuts.SetRegion(ConsoleRegion.Ntsc); };
			mnuRegionPal.Click += (s, e) => { _shortcuts.SetRegion(ConsoleRegion.Pal); };

			mnuDebugger.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.Debugger); };
			mnuSpcDebugger.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.SpcDebugger); };
			mnuTraceLogger.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.TraceLogger); };
			mnuMemoryTools.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.MemoryTools); };
			mnuTilemapViewer.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.TilemapViewer); };
			mnuTileViewer.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.TileViewer); };
			mnuSpriteViewer.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.SpriteViewer); };
			mnuPaletteViewer.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.PaletteViewer); };
			mnuEventViewer.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.EventViewer); };
			mnuScriptWindow.Click += (s, e) => { DebugWindowManager.OpenDebugWindow(DebugWindow.ScriptWindow); };

			UpdateDebuggerMenu();
		}

		private void UpdateDebuggerMenu()
		{
			bool running = EmuRunner.IsRunning();
			mnuDebugger.Enabled = running;
			mnuSpcDebugger.Enabled = running;
			mnuTraceLogger.Enabled = running;
			mnuScriptWindow.Enabled = running;
			mnuMemoryTools.Enabled = running;
			mnuTilemapViewer.Enabled = running;
			mnuTileViewer.Enabled = running;
			mnuSpriteViewer.Enabled = running;
			mnuPaletteViewer.Enabled = running;
			mnuEventViewer.Enabled = running;
		}
		
		private void ResizeRecentGames()
		{
			ctrlRecentGames.Height = this.ClientSize.Height - ctrlRecentGames.Top - 80;
		}

		private void frmMain_Resize(object sender, EventArgs e)
		{
			ResizeRecentGames();
		}
		
		private void mnuVideoConfig_Click(object sender, EventArgs e)
		{
			using(frmVideoConfig frm = new frmVideoConfig()) {
				frm.ShowDialog(sender, this);
			}
			ConfigManager.Config.Video.ApplyConfig();
		}

		private void mnuAudioConfig_Click(object sender, EventArgs e)
		{
			using(frmAudioConfig frm = new frmAudioConfig()) {
				frm.ShowDialog(sender, this);
			}
			ConfigManager.Config.Audio.ApplyConfig();
		}
		
		private void mnuEmulationConfig_Click(object sender, EventArgs e)
		{
			using(frmEmulationConfig frm = new frmEmulationConfig()) {
				frm.ShowDialog(sender, this);
			}
			ConfigManager.Config.Emulation.ApplyConfig();
		}
		
		private void mnuInputConfig_Click(object sender, EventArgs e)
		{
			using(frmInputConfig frm = new frmInputConfig()) {
				frm.ShowDialog(sender, this);
			}
			ConfigManager.Config.Input.ApplyConfig();
		}

		private void mnuPreferences_Click(object sender, EventArgs e)
		{
			using(frmPreferences frm = new frmPreferences()) {
				frm.ShowDialog(sender, this);
				ConfigManager.Config.Preferences.ApplyConfig();
				if(frm.NeedRestart) {
					this.Close();
				}
			}
		}

		protected override void OnDragDrop(DragEventArgs e)
		{
			base.OnDragDrop(e);

			try {
				string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
				if(File.Exists(files[0])) {
					EmuRunner.LoadFile(files[0]);
					this.Activate();
				} else {
					EmuApi.DisplayMessage("Error", "File not found: " + files[0]);
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		protected override void OnDragEnter(DragEventArgs e)
		{
			base.OnDragEnter(e);

			try {
				if(e.Data != null && e.Data.GetDataPresent(DataFormats.FileDrop)) {
					e.Effect = DragDropEffects.Copy;
				} else {
					EmuApi.DisplayMessage("Error", "Unsupported operation.");
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		private void mnuLogWindow_Click(object sender, EventArgs e)
		{
			new frmLogWindow().Show();
		}

		private void mnuCheckForUpdates_Click(object sender, EventArgs e)
		{
			UpdateHelper.CheckForUpdates(false);
		}

		private void mnuReportBug_Click(object sender, EventArgs e)
		{
			Process.Start("https://www.mesen.ca/snes/ReportBug.php");
		}

		private void mnuAbout_Click(object sender, EventArgs e)
		{
			using(frmAbout frm = new frmAbout()) {
				frm.ShowDialog(this);
			}
		}

		private void mnuFile_DropDownOpening(object sender, EventArgs e)
		{
			mnuRecentFiles.DropDownItems.Clear();
			mnuRecentFiles.DropDownItems.AddRange(ConfigManager.Config.RecentFiles.GetMenuItems().ToArray());
			mnuRecentFiles.Enabled = ConfigManager.Config.RecentFiles.Items.Count > 0;
		}
		
		private void mnuVideoFilter_DropDownOpening(object sender, EventArgs e)
		{
			VideoFilterType filterType = ConfigManager.Config.Video.VideoFilter;
			mnuNoneFilter.Checked = (filterType == VideoFilterType.None);
			mnuNtscFilter.Checked = (filterType == VideoFilterType.NTSC);
			mnuXBRZ2xFilter.Checked = (filterType == VideoFilterType.xBRZ2x);
			mnuXBRZ3xFilter.Checked = (filterType == VideoFilterType.xBRZ3x);
			mnuXBRZ4xFilter.Checked = (filterType == VideoFilterType.xBRZ4x);
			mnuXBRZ5xFilter.Checked = (filterType == VideoFilterType.xBRZ5x);
			mnuXBRZ6xFilter.Checked = (filterType == VideoFilterType.xBRZ6x);
			mnuHQ2xFilter.Checked = (filterType == VideoFilterType.HQ2x);
			mnuHQ3xFilter.Checked = (filterType == VideoFilterType.HQ3x);
			mnuHQ4xFilter.Checked = (filterType == VideoFilterType.HQ4x);
			mnuScale2xFilter.Checked = (filterType == VideoFilterType.Scale2x);
			mnuScale3xFilter.Checked = (filterType == VideoFilterType.Scale3x);
			mnuScale4xFilter.Checked = (filterType == VideoFilterType.Scale4x);
			mnu2xSaiFilter.Checked = (filterType == VideoFilterType._2xSai);
			mnuSuper2xSaiFilter.Checked = (filterType == VideoFilterType.Super2xSai);
			mnuSuperEagleFilter.Checked = (filterType == VideoFilterType.SuperEagle);
			mnuPrescale2xFilter.Checked = (filterType == VideoFilterType.Prescale2x);
			mnuPrescale3xFilter.Checked = (filterType == VideoFilterType.Prescale3x);
			mnuPrescale4xFilter.Checked = (filterType == VideoFilterType.Prescale4x);
			mnuPrescale6xFilter.Checked = (filterType == VideoFilterType.Prescale6x);
			mnuPrescale8xFilter.Checked = (filterType == VideoFilterType.Prescale8x);
			mnuPrescale10xFilter.Checked = (filterType == VideoFilterType.Prescale10x);

			mnuBilinearInterpolation.Checked = ConfigManager.Config.Video.UseBilinearInterpolation;
		}

		private void mnuVideoScale_DropDownOpening(object sender, EventArgs e)
		{
			double scale = ConfigManager.Config.Video.VideoScale;
			mnuScale1x.Checked = (scale == 1.0);
			mnuScale2x.Checked = (scale == 2.0);
			mnuScale3x.Checked = (scale == 3.0);
			mnuScale4x.Checked = (scale == 4.0);
			mnuScale5x.Checked = (scale == 5.0);
			mnuScale6x.Checked = (scale == 6.0);

			mnuFullscreen.Checked = _displayManager.Fullscreen;
		}

		private void mnuEmulationSpeed_DropDownOpening(object sender, EventArgs e)
		{
			uint emulationSpeed = ConfigManager.Config.Emulation.EmulationSpeed;
			mnuEmuSpeedNormal.Checked = emulationSpeed == 100;
			mnuEmuSpeedQuarter.Checked = emulationSpeed == 25;
			mnuEmuSpeedHalf.Checked = emulationSpeed == 50;
			mnuEmuSpeedDouble.Checked = emulationSpeed == 200;
			mnuEmuSpeedTriple.Checked = emulationSpeed == 300;
			mnuEmuSpeedMaximumSpeed.Checked = emulationSpeed == 0;

			mnuShowFPS.Checked = ConfigManager.Config.Preferences.ShowFps;
		}

		private void mnuLoadState_DropDownOpening(object sender, EventArgs e)
		{
			SaveStateManager.UpdateStateMenu(mnuLoadState, false);
		}

		private void mnuSaveState_DropDownOpening(object sender, EventArgs e)
		{
			SaveStateManager.UpdateStateMenu(mnuSaveState, true);
		}

		private void mnuRegion_DropDownOpening(object sender, EventArgs e)
		{
			mnuRegionAuto.Checked = ConfigManager.Config.Emulation.Region == ConsoleRegion.Auto;
			mnuRegionNtsc.Checked = ConfigManager.Config.Emulation.Region == ConsoleRegion.Ntsc;
			mnuRegionPal.Checked = ConfigManager.Config.Emulation.Region == ConsoleRegion.Pal;
		}

		private void mnuAviRecord_Click(object sender, EventArgs e)
		{
			using(frmRecordAvi frm = new frmRecordAvi()) {
				if(frm.ShowDialog(mnuVideoRecorder, this) == DialogResult.OK) {
					RecordApi.AviRecord(frm.Filename, ConfigManager.Config.AviRecord.Codec, ConfigManager.Config.AviRecord.CompressionLevel);
				}
			}
		}

		private void mnuAviStop_Click(object sender, EventArgs e)
		{
			RecordApi.AviStop();
		}

		private void mnuTools_DropDownOpening(object sender, EventArgs e)
		{
			mnuSoundRecorder.Enabled = EmuRunner.IsRunning();
			mnuWaveRecord.Enabled = EmuRunner.IsRunning() && !RecordApi.WaveIsRecording();
			mnuWaveStop.Enabled = EmuRunner.IsRunning() && RecordApi.WaveIsRecording();

			mnuVideoRecorder.Enabled = EmuRunner.IsRunning();
			mnuAviRecord.Enabled = EmuRunner.IsRunning() && !RecordApi.AviIsRecording();
			mnuAviStop.Enabled = EmuRunner.IsRunning() && RecordApi.AviIsRecording();
		}

		private void mnuWaveRecord_Click(object sender, EventArgs e)
		{
			using(SaveFileDialog sfd = new SaveFileDialog()) {
				sfd.SetFilter(ResourceHelper.GetMessage("FilterWave"));
				sfd.InitialDirectory = ConfigManager.WaveFolder;
				sfd.FileName = EmuApi.GetRomInfo().GetRomName() + ".wav";
				if(sfd.ShowDialog(this) == DialogResult.OK) {
					RecordApi.WaveRecord(sfd.FileName);
				}
			}
		}

		private void mnuWaveStop_Click(object sender, EventArgs e)
		{
			RecordApi.WaveStop();
		}

		private void mnu_DropDownOpened(object sender, EventArgs e)
		{
			Interlocked.Increment(ref _inMenu);
		}

		private void mnu_DropDownClosed(object sender, EventArgs e)
		{
			Task.Run(() => {
				Thread.Sleep(100);
				Interlocked.Decrement(ref _inMenu);
			});
		}
	}
}
