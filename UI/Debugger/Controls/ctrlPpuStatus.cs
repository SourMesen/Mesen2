using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlPpuStatus : BaseControl
	{
		private EntityBinder _binder = new EntityBinder();
		private DebugState _lastState;

		public ctrlPpuStatus()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}

			_binder.Entity = new PpuState();
			_binder.AddBinding(nameof(PpuState.Cycle), txtCycle, eNumberFormat.Decimal);
			_binder.AddBinding(nameof(PpuState.Scanline), txtScanline, eNumberFormat.Decimal);
		}

		public void UpdateStatus(DebugState state)
		{
			_lastState = state;

			_binder.Entity = state.Ppu;
			_binder.UpdateUI();

			txtHClocks.Text = (state.Ppu.Cycle * 4).ToString();
		}
	}
}
