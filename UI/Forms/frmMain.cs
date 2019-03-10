using Mesen.GUI.Config;
using Mesen.GUI.Debugger;
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

		public frmMain(string[] args)
		{
			InitializeComponent();
			ResourceHelper.LoadResources(Language.English);
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			EmuApi.InitDll();
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, Handle, ctrlRenderer.Handle, false, false, false);

			ConfigManager.Config.ApplyConfig();

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;

			new frmLogWindow().Show();
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
				case ConsoleNotificationType.ResolutionChanged:
					ScreenSize size = EmuApi.GetScreenSize(false);
					this.BeginInvoke((Action)(() => {
						UpdateViewerSize(size);
					}));
					break;
			}
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

		private void mnuStep_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1);
		}

		private void mnuOpen_Click(object sender, EventArgs e)
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.Filter = ResourceHelper.GetMessage("FilterRom");
				if(ofd.ShowDialog() == DialogResult.OK) {
					EmuRunner.LoadRom(ofd.FileName);
				}
			}
		}

		private void mnuRun_Click(object sender, EventArgs e)
		{
			DebugApi.ResumeExecution();
		}

		private void mnuRun100Instructions_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1000);
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

		private void mnuFile_DropDownOpening(object sender, EventArgs e)
		{
			mnuRecentFiles.DropDownItems.Clear();
			mnuRecentFiles.DropDownItems.AddRange(ConfigManager.Config.RecentFiles.GetMenuItems().ToArray());
			mnuRecentFiles.Enabled = ConfigManager.Config.RecentFiles.Items.Count > 0;
		}
	}
}
