using Mesen.GUI.Config;
using Mesen.GUI.Config.Shortcuts;
using Mesen.GUI.Debugger;
using Mesen.GUI.Emulation;
using Mesen.GUI.Forms.Config;
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

namespace Mesen.GUI.Forms
{
	public partial class frmMain : BaseInputForm
	{
		private NotificationListener _notifListener;
		private ShortcutHandler _shortcuts;

		public frmMain(string[] args)
		{
			InitializeComponent();
			ResourceHelper.LoadResources(Language.English);
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			_shortcuts = new ShortcutHandler();

			EmuApi.InitDll();
			ConfigManager.Config.Video.ApplyConfig();
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, Handle, ctrlRenderer.Handle, false, false, false);

			ConfigManager.Config.InitializeDefaults();
			ConfigManager.Config.ApplyConfig();

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			SaveStateManager.InitializeStateMenu(mnuSaveState, true, _shortcuts);
			SaveStateManager.InitializeStateMenu(mnuLoadState, false, _shortcuts);

			BindShortcuts();
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);

			if(_notifListener != null) {
				_notifListener.Dispose();
				_notifListener = null;
			}

			ConfigManager.ApplyChanges();

			DebugApi.ResumeExecution();
			EmuApi.Stop();
			EmuApi.Release();
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					this.BeginInvoke((Action)(() => {
						SaveStateManager.UpdateStateMenu(mnuLoadState, false);
						SaveStateManager.UpdateStateMenu(mnuSaveState, true);
					}));
					break;

				case ConsoleNotificationType.ResolutionChanged:
					ScreenSize size = EmuApi.GetScreenSize(false);
					this.BeginInvoke((Action)(() => {
						UpdateViewerSize(size);
					}));
					break;

				case ConsoleNotificationType.ExecuteShortcut:
					this.BeginInvoke((Action)(() => {
						_shortcuts.ExecuteShortcut((EmulatorShortcut)e.Parameter);
					}));
					break;
			}
		}

		private void BindShortcuts()
		{
			Func<bool> notClient = () => { return true; }; //TODO
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

			_shortcuts.BindShortcut(mnuTakeScreenshot, EmulatorShortcut.TakeScreenshot);

			mnuDebugger.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenDebugger));
			mnuMemoryTools.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenMemoryTools));
			mnuEventViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenEventViewer));
			mnuTilemapViewer.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenPpuViewer));
			mnuTraceLogger.InitShortcut(this, nameof(DebuggerShortcutsConfig.OpenTraceLogger));

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
		}
		
		private void UpdateViewerSize(ScreenSize screenSize)
		{
			//this.Resize -= frmMain_Resize;

			//if(forceUpdate || (!_customSize && this.WindowState != FormWindowState.Maximized)) {
				Size newSize = new Size(screenSize.Width, screenSize.Height);

				//UpdateScaleMenu(size.Scale);
				this.ClientSize = new Size(newSize.Width, newSize.Height + pnlRenderer.Top);
			//}

			ctrlRenderer.Size = new Size(screenSize.Width, screenSize.Height);
			ctrlRenderer.Top = (pnlRenderer.Height - ctrlRenderer.Height) / 2;
			ctrlRenderer.Left = (pnlRenderer.Width - ctrlRenderer.Width) / 2;

			//this.Resize += frmMain_Resize;
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

		private void mnuPreferences_Click(object sender, EventArgs e)
		{
			using(frmPreferences frm = new frmPreferences()) {
				frm.ShowDialog(sender, this);
			}
			ConfigManager.Config.Preferences.ApplyConfig();
		}

		private void mnuDebugger_Click(object sender, EventArgs e)
		{
			DebugWindowManager.OpenDebugWindow(DebugWindow.Debugger);
		}

		private void mnuTraceLogger_Click(object sender, EventArgs e)
		{
			DebugWindowManager.OpenDebugWindow(DebugWindow.TraceLogger);
		}

		private void mnuMemoryTools_Click(object sender, EventArgs e)
		{
			DebugWindowManager.OpenDebugWindow(DebugWindow.MemoryTools);
		}

		private void mnuTilemapViewer_Click(object sender, EventArgs e)
		{
			DebugWindowManager.OpenDebugWindow(DebugWindow.TilemapViewer);
		}

		private void mnuEventViewer_Click(object sender, EventArgs e)
		{
			DebugWindowManager.OpenDebugWindow(DebugWindow.EventViewer);
		}

		protected override void OnDragDrop(DragEventArgs e)
		{
			base.OnDragDrop(e);

			try {
				string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
				if(File.Exists(files[0])) {
					EmuRunner.LoadRom(files[0]);
					this.Activate();
				} else {
					//InteropEmu.DisplayMessage("Error", "File not found: " + files[0]);
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
					//InteropEmu.DisplayMessage("Error", "Unsupported operation.");
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		private void mnuExit_Click(object sender, EventArgs e)
		{
			this.Close();
		}
		
		private void mnuLogWindow_Click(object sender, EventArgs e)
		{
			new frmLogWindow().Show();
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
		}

		private void mnuLoadState_DropDownOpening(object sender, EventArgs e)
		{
			SaveStateManager.UpdateStateMenu(mnuLoadState, false);
		}

		private void mnuSaveState_DropDownOpening(object sender, EventArgs e)
		{
			SaveStateManager.UpdateStateMenu(mnuSaveState, true);
		}
	}
}
