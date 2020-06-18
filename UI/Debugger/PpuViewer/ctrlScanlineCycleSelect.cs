using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;

namespace Mesen.GUI.Debugger.Controls
{
	public partial class ctrlScanlineCycleSelect : UserControl
	{
		private static int _nextViewerId = 0;

		private int _viewerId = 0;
		private int _scanline = 241;
		private int _cycle = 0;
		private CpuType _cpuType = CpuType.Cpu;

		public int Scanline { get { return _scanline; } }
		public int Cycle { get { return _cycle; } }
		public int ViewerId { get { return _viewerId; } }

		public ctrlScanlineCycleSelect()
		{
			InitializeComponent();
			_viewerId = GetNextViewerId();
		}

		private int GetNextViewerId()
		{
			return _nextViewerId++;
		}

		public void Initialize(int scanline, int cycle, CpuType cpuType)
		{
			_scanline = scanline;
			_cycle = cycle;
			_cpuType = cpuType;

			this.nudScanline.Value = _scanline;
			this.nudCycle.Value = _cycle;

			RefreshSettings();
		}

		public void RefreshSettings()
		{
			bool isGameboy = _cpuType == CpuType.Gameboy;
			DebugApi.SetViewerUpdateTiming(_viewerId, Math.Min(_scanline, isGameboy ? 153 : 312), _cycle, _cpuType);
		}

		private void SetUpdateScanlineCycle(int scanline, int cycle)
		{
			_scanline = scanline;
			_cycle = cycle;
			RefreshSettings();
		}

		private void nudScanlineCycle_ValueChanged(object sender, EventArgs e)
		{
			SetUpdateScanlineCycle((int)this.nudScanline.Value, (int)this.nudCycle.Value);
		}

		private void btnReset_Click(object sender, EventArgs e)
		{
			bool isGameboy = _cpuType == CpuType.Gameboy;
			this.nudScanline.Value = isGameboy ? 144 : 241;
			this.nudCycle.Value = 0;
		}
	}
}
