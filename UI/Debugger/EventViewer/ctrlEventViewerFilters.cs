using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlEventViewerFilters : BaseControl
	{
		public event EventHandler OptionsChanged;

		private EntityBinder _entityBinder = new EntityBinder();

		public ctrlEventViewerFilters()
		{
			InitializeComponent();
		}

		public void Init()
		{
			_entityBinder.Entity = ConfigManager.Config.Debug.EventViewer;
			_entityBinder.AddBinding(nameof(EventViewerConfig.ApuRegisterReadColor), picApuReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ApuRegisterWriteColor), picApuWrites);
			_entityBinder.AddBinding(nameof(EventViewerConfig.CpuRegisterReadColor), picCpuReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.CpuRegisterWriteColor), picCpuWrites);
			_entityBinder.AddBinding(nameof(EventViewerConfig.IrqColor), picIrq);
			_entityBinder.AddBinding(nameof(EventViewerConfig.BreakpointColor), picMarkedBreakpoints);
			_entityBinder.AddBinding(nameof(EventViewerConfig.NmiColor), picNmi);
			_entityBinder.AddBinding(nameof(EventViewerConfig.PpuRegisterReadColor), picPpuReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.PpuRegisterWriteColor), picPpuWrites);
			_entityBinder.AddBinding(nameof(EventViewerConfig.WorkRamRegisterReadColor), picWramReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.WorkRamRegisterWriteColor), picWramWrites);

			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowApuRegisterReads), chkShowApuRegisterReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowApuRegisterWrites), chkShowApuRegisterWrites);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowCpuRegisterReads), chkShowCpuRegisterReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowCpuRegisterWrites), chkShowCpuRegisterWrites);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowIrq), chkShowIrq);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowMarkedBreakpoints), chkShowMarkedBreakpoints);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowNmi), chkShowNmi);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowPpuRegisterReads), chkShowPpuRegisterReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowPpuRegisterWrites), chkShowPpuRegisterWrites);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowWorkRamRegisterReads), chkShowWorkRamRegisterReads);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowWorkRamRegisterWrites), chkShowWorkRamRegisterWrites);

			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel0), chkDmaChannel0);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel1), chkDmaChannel1);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel2), chkDmaChannel2);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel3), chkDmaChannel3);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel4), chkDmaChannel4);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel5), chkDmaChannel5);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel6), chkDmaChannel6);
			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowDmaChannel7), chkDmaChannel7);

			_entityBinder.AddBinding(nameof(EventViewerConfig.ShowPreviousFrameEvents), chkShowPreviousFrameEvents);

			_entityBinder.UpdateUI();
		}

		protected override void OnHandleDestroyed(EventArgs e)
		{
			if(!IsDesignMode) {
				_entityBinder.UpdateObject();
				ConfigManager.ApplyChanges();
			}

			base.OnHandleDestroyed(e);
		}

		private void chkOption_Click(object sender, EventArgs e)
		{
			_entityBinder.UpdateObject();
			this.OptionsChanged?.Invoke(this, e);
		}
	}
}
