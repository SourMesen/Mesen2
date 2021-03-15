using Mesen.GUI;
using Mesen.GUI.Utilities;
using System.Drawing;

namespace Mesen
{
	public class EventViewerConfig
	{
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

		public XmlColor IrqColor = Color.Yellow;
		public XmlColor NmiColor = Color.Yellow;
		public XmlColor BreakpointColor = Color.Yellow;
		public XmlColor ApuRegisterReadColor = Color.Yellow;
		public XmlColor ApuRegisterWriteColor = Color.Yellow;
		public XmlColor CpuRegisterReadColor = Color.Yellow;
		public XmlColor CpuRegisterWriteColor = Color.Yellow;
		public XmlColor WorkRamRegisterReadColor = Color.Yellow;
		public XmlColor WorkRamRegisterWriteColor = Color.Yellow;

		public XmlColor PpuRegisterReadColor = Color.Yellow;
		public XmlColor PpuRegisterWriteCgramColor = Color.Yellow;
		public XmlColor PpuRegisterWriteVramColor = Color.Yellow;
		public XmlColor PpuRegisterWriteOamColor = Color.Yellow;
		public XmlColor PpuRegisterWriteBgOptionColor = Color.Yellow;
		public XmlColor PpuRegisterWriteBgScrollColor = Color.Yellow;
		public XmlColor PpuRegisterWriteMode7Color = Color.Yellow;
		public XmlColor PpuRegisterWriteWindowColor = Color.Yellow;
		public XmlColor PpuRegisterWriteOtherColor = Color.Yellow;

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
				IrqColor = (uint)this.IrqColor.Color.ToArgb(),
				NmiColor = (uint)this.NmiColor.Color.ToArgb(),
				BreakpointColor = (uint)this.BreakpointColor.Color.ToArgb(),

				PpuRegisterReadColor = (uint)this.PpuRegisterReadColor.Color.ToArgb(),
				PpuRegisterWriteCgramColor = (uint)this.PpuRegisterWriteCgramColor.Color.ToArgb(),
				PpuRegisterWriteVramColor = (uint)this.PpuRegisterWriteVramColor.Color.ToArgb(),
				PpuRegisterWriteOamColor = (uint)this.PpuRegisterWriteOamColor.Color.ToArgb(),
				PpuRegisterWriteMode7Color = (uint)this.PpuRegisterWriteMode7Color.Color.ToArgb(),
				PpuRegisterWriteBgOptionColor = (uint)this.PpuRegisterWriteBgOptionColor.Color.ToArgb(),
				PpuRegisterWriteBgScrollColor = (uint)this.PpuRegisterWriteBgScrollColor.Color.ToArgb(),
				PpuRegisterWriteWindowColor = (uint)this.PpuRegisterWriteWindowColor.Color.ToArgb(),
				PpuRegisterWriteOtherColor = (uint)this.PpuRegisterWriteOtherColor.Color.ToArgb(),

				ApuRegisterReadColor = (uint)this.ApuRegisterReadColor.Color.ToArgb(),
				ApuRegisterWriteColor = (uint)this.ApuRegisterWriteColor.Color.ToArgb(),
				CpuRegisterReadColor = (uint)this.CpuRegisterReadColor.Color.ToArgb(),
				CpuRegisterWriteColor = (uint)this.CpuRegisterWriteColor.Color.ToArgb(),
				WorkRamRegisterReadColor = (uint)this.WorkRamRegisterReadColor.Color.ToArgb(),
				WorkRamRegisterWriteColor = (uint)this.WorkRamRegisterWriteColor.Color.ToArgb(),

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
