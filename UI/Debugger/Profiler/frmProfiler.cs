using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmProfiler : BaseForm, IRefresh
	{
		private ctrlProfiler ctrlProfilerCoprocessor = new ctrlProfiler();
		private WindowRefreshManager _refreshManager;
		private NotificationListener _notifListener;
		private TabPage _selectedTab;

		public ctrlScanlineCycleSelect ScanlineCycleSelect => null;

		public frmProfiler()
		{
			InitializeComponent();

			if(!DesignMode) {
				ProfilerConfig cfg = ConfigManager.Config.Debug.Profiler;
				RestoreLocation(cfg.WindowLocation, cfg.WindowSize);
			}
		}

		protected override void OnLoad(EventArgs e)
		{
			base.OnLoad(e);

			ctrlProfiler.CpuType = CpuType.Cpu;
			ctrlProfilerSpc.CpuType = CpuType.Spc;

			_selectedTab = tpgCpu;
			UpdateAvailableTabs();
			RefreshData();
			RefreshViewer();

			_refreshManager = new WindowRefreshManager(this);
			_refreshManager.AutoRefresh = true;
			_refreshManager.AutoRefreshSpeed = RefreshSpeed.Low;

			_notifListener = new NotificationListener();
			_notifListener.OnNotification += OnNotificationReceived;
		}

		private void OnNotificationReceived(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					_selectedTab = tpgCpu;
					this.BeginInvoke((Action)(() => {
						UpdateAvailableTabs();
					}));
					break;
			}
		}

		public void RefreshData()
		{
			if(_selectedTab == tpgCpu) {
				ctrlProfiler.RefreshData();
			} else if(_selectedTab == tpgSpc) {
				ctrlProfilerSpc.RefreshData();
			} else if(_selectedTab != null) {
				ctrlProfilerCoprocessor.RefreshData();
			}
		}

		public void RefreshViewer()
		{
			if(_selectedTab == tpgCpu) {
				ctrlProfiler.RefreshList();
			} else if(_selectedTab == tpgSpc) {
				ctrlProfilerSpc.RefreshList();
			} else if(_selectedTab != null) {
				ctrlProfilerCoprocessor.RefreshList();
			}
		}

		private void UpdateAvailableTabs()
		{
			tabMain.SelectedIndexChanged -= tabMain_SelectedIndexChanged;
			tabMain.TabPages.Clear();

			RomInfo romInfo = EmuApi.GetRomInfo();
			if(romInfo.CoprocessorType != CoprocessorType.Gameboy) {
				tabMain.TabPages.Add(tpgCpu);
				tabMain.TabPages.Add(tpgSpc);
				tabMain.SelectedTab = tpgCpu;
			}

			if(romInfo.CoprocessorType == CoprocessorType.SA1 || romInfo.CoprocessorType == CoprocessorType.Gameboy || romInfo.CoprocessorType == CoprocessorType.SGB) {
				TabPage coprocessorTab = new TabPage(ResourceHelper.GetEnumText(romInfo.CoprocessorType));
				tabMain.TabPages.Add(coprocessorTab);

				coprocessorTab.Controls.Add(ctrlProfilerCoprocessor);
				ctrlProfilerCoprocessor.Dock = DockStyle.Fill;
				ctrlProfilerCoprocessor.CpuType = romInfo.CoprocessorType == CoprocessorType.SA1 ? CpuType.Sa1 : CpuType.Gameboy;
			}

			_selectedTab = tabMain.TabPages[0];
			tabMain.SelectedIndexChanged += tabMain_SelectedIndexChanged;
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			base.OnFormClosing(e);
			
			ProfilerConfig cfg = ConfigManager.Config.Debug.Profiler;

			cfg.WindowSize = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Size : this.Size;
			cfg.WindowLocation = this.WindowState != FormWindowState.Normal ? this.RestoreBounds.Location : this.Location;
			ConfigManager.ApplyChanges();
		}

		private void tabMain_SelectedIndexChanged(object sender, EventArgs e)
		{
			_selectedTab = tabMain.SelectedTab;
			RefreshData();
			RefreshViewer();
		}
	}
}
