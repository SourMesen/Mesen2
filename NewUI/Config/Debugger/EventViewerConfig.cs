using Avalonia;
using Avalonia.Media;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class EventViewerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public int ImageScale = 1;
		public bool RefreshOnBreakPause = true;

		public bool AutoRefresh = true;
		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Normal;

		public bool ShowPpuRegisterCgramWrites = true;
		public bool ShowPpuRegisterVramWrites = true;
		public bool ShowPpuRegisterOamWrites = true;
		public bool ShowPpuRegisterMode7Writes = true;
		public bool ShowPpuRegisterBgOptionWrites = true;
		public bool ShowPpuRegisterBgScrollWrites = true;
		public bool ShowPpuRegisterWindowWrites = true;
		public bool ShowPpuRegisterOtherWrites = true;
		public bool ShowPpuRegisterReads = true;

		public bool ShowCpuRegisterWrites = true;
		public bool ShowCpuRegisterReads = true;

		public bool ShowApuRegisterWrites = false;
		public bool ShowApuRegisterReads = false;
		public bool ShowWorkRamRegisterWrites = false;
		public bool ShowWorkRamRegisterReads = false;

		public bool ShowNmi = true;
		public bool ShowIrq = true;

		public bool ShowMarkedBreakpoints = true;
		public bool ShowPreviousFrameEvents = true;

		public bool ShowDmaChannel0 = true;
		public bool ShowDmaChannel1 = true;
		public bool ShowDmaChannel2 = true;
		public bool ShowDmaChannel3 = true;
		public bool ShowDmaChannel4 = true;
		public bool ShowDmaChannel5 = true;
		public bool ShowDmaChannel6 = true;
		public bool ShowDmaChannel7 = true;
		
		public Color IrqColor = Color.FromRgb(0xFF, 0xAD, 0xAC);
		public Color NmiColor = Color.FromRgb(0xFF, 0xAD, 0xAC);
		public Color BreakpointColor = Color.FromRgb(0xAF, 0xFF, 0xAF);
		public Color ApuRegisterReadColor = Color.FromRgb(0xF9, 0xFE, 0xAC);
		public Color ApuRegisterWriteColor = Color.FromRgb(0x9F, 0x93, 0xC6);
		public Color CpuRegisterReadColor = Color.FromRgb(0x18, 0x98, 0xE4);
		public Color CpuRegisterWriteColor = Color.FromRgb(0xFF, 0x5E, 0x5E);
		public Color WorkRamRegisterReadColor = Color.FromRgb(0x8E, 0x33, 0xFF);
		public Color WorkRamRegisterWriteColor = Color.FromRgb(0x2E, 0xFF, 0x28);

		public Color PpuRegisterReadColor = Color.FromRgb(0x00, 0x75, 0x97);
		public Color PpuRegisterWriteCgramColor = Color.FromRgb(0xC9, 0x29, 0x29);
		public Color PpuRegisterWriteVramColor = Color.FromRgb(0xB4, 0x7A, 0xDA);
		public Color PpuRegisterWriteOamColor = Color.FromRgb(0x53, 0xD7, 0x44);
		public Color PpuRegisterWriteBgOptionColor = Color.FromRgb(0xBF, 0x80, 0x20);
		public Color PpuRegisterWriteBgScrollColor = Color.FromRgb(0x4A, 0x7C, 0xD9);
		public Color PpuRegisterWriteMode7Color = Color.FromRgb(0xFE, 0x78, 0x7B);
		public Color PpuRegisterWriteWindowColor = Color.FromRgb(0xE2, 0x51, 0xF7);
		public Color PpuRegisterWriteOtherColor = Color.FromRgb(0xD1, 0xDD, 0x42);

		public EventViewerDisplayOptions GetInteropOptions()
		{
			return new EventViewerDisplayOptions() {
				ShowPpuRegisterCgramWrites = this.ShowPpuRegisterCgramWrites,
				ShowPpuRegisterVramWrites = this.ShowPpuRegisterVramWrites,
				ShowPpuRegisterOamWrites = this.ShowPpuRegisterOamWrites,
				ShowPpuRegisterMode7Writes = this.ShowPpuRegisterMode7Writes,
				ShowPpuRegisterBgOptionWrites = this.ShowPpuRegisterBgOptionWrites,
				ShowPpuRegisterBgScrollWrites = this.ShowPpuRegisterBgScrollWrites,
				ShowPpuRegisterWindowWrites = this.ShowPpuRegisterWindowWrites,
				ShowPpuRegisterOtherWrites = this.ShowPpuRegisterOtherWrites,
				ShowPpuRegisterReads = this.ShowPpuRegisterReads,

				ShowCpuRegisterWrites = this.ShowCpuRegisterWrites,
				ShowCpuRegisterReads = this.ShowCpuRegisterReads,
				ShowApuRegisterWrites = this.ShowApuRegisterWrites,
				ShowApuRegisterReads = this.ShowApuRegisterReads,
				ShowWorkRamRegisterWrites = this.ShowWorkRamRegisterWrites,
				ShowWorkRamRegisterReads = this.ShowWorkRamRegisterReads,
				ShowNmi = this.ShowNmi,
				ShowIrq = this.ShowIrq,
				ShowMarkedBreakpoints = this.ShowMarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents,
				IrqColor = (uint)this.IrqColor.ToUint32(),
				NmiColor = (uint)this.NmiColor.ToUint32(),
				BreakpointColor = (uint)this.BreakpointColor.ToUint32(),

				PpuRegisterReadColor = (uint)this.PpuRegisterReadColor.ToUint32(),
				PpuRegisterWriteCgramColor = (uint)this.PpuRegisterWriteCgramColor.ToUint32(),
				PpuRegisterWriteVramColor = (uint)this.PpuRegisterWriteVramColor.ToUint32(),
				PpuRegisterWriteOamColor = (uint)this.PpuRegisterWriteOamColor.ToUint32(),
				PpuRegisterWriteMode7Color = (uint)this.PpuRegisterWriteMode7Color.ToUint32(),
				PpuRegisterWriteBgOptionColor = (uint)this.PpuRegisterWriteBgOptionColor.ToUint32(),
				PpuRegisterWriteBgScrollColor = (uint)this.PpuRegisterWriteBgScrollColor.ToUint32(),
				PpuRegisterWriteWindowColor = (uint)this.PpuRegisterWriteWindowColor.ToUint32(),
				PpuRegisterWriteOtherColor = (uint)this.PpuRegisterWriteOtherColor.ToUint32(),

				ApuRegisterReadColor = (uint)this.ApuRegisterReadColor.ToUint32(),
				ApuRegisterWriteColor = (uint)this.ApuRegisterWriteColor.ToUint32(),
				CpuRegisterReadColor = (uint)this.CpuRegisterReadColor.ToUint32(),
				CpuRegisterWriteColor = (uint)this.CpuRegisterWriteColor.ToUint32(),
				WorkRamRegisterReadColor = (uint)this.WorkRamRegisterReadColor.ToUint32(),
				WorkRamRegisterWriteColor = (uint)this.WorkRamRegisterWriteColor.ToUint32(),

				ShowDmaChannels = new byte[8] {
					(byte)(this.ShowDmaChannel0 ? 1 : 0),
					(byte)(this.ShowDmaChannel1 ? 1 : 0),
					(byte)(this.ShowDmaChannel2 ? 1 : 0),
					(byte)(this.ShowDmaChannel3 ? 1 : 0),
					(byte)(this.ShowDmaChannel4 ? 1 : 0),
					(byte)(this.ShowDmaChannel5 ? 1 : 0),
					(byte)(this.ShowDmaChannel6 ? 1 : 0),
					(byte)(this.ShowDmaChannel7 ? 1 : 0)
				}
			};
		}
	}
}
