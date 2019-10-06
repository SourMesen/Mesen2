using Mesen.GUI.Utilities;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class EventViewerConfig
	{
		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;

		public int ImageScale = 1;
		public bool RefreshOnBreakPause = true;

		public bool AutoRefresh = true;
		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Normal;

		public bool ShowPpuRegisterWrites = true;
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
		
		public XmlColor IrqColor = ColorTranslator.FromHtml("#FFADAC");
		public XmlColor NmiColor = ColorTranslator.FromHtml("#FFADAC");
		public XmlColor BreakpointColor = ColorTranslator.FromHtml("#FFADAC");
		public XmlColor PpuRegisterReadColor = ColorTranslator.FromHtml("#007597");
		public XmlColor PpuRegisterWriteColor = ColorTranslator.FromHtml("#C92929");
		public XmlColor ApuRegisterReadColor = ColorTranslator.FromHtml("#F9FEAC");
		public XmlColor ApuRegisterWriteColor = ColorTranslator.FromHtml("#9F93C6");
		public XmlColor CpuRegisterReadColor = ColorTranslator.FromHtml("#1898E4");
		public XmlColor CpuRegisterWriteColor = ColorTranslator.FromHtml("#FF5E5E");
		public XmlColor WorkRamRegisterReadColor = ColorTranslator.FromHtml("#8E33FF");
		public XmlColor WorkRamRegisterWriteColor = ColorTranslator.FromHtml("#2EFF28");

		public EventViewerDisplayOptions GetInteropOptions()
		{
			return new EventViewerDisplayOptions() {
				ShowPpuRegisterWrites = this.ShowPpuRegisterWrites,
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
				PpuRegisterWriteColor = (uint)this.PpuRegisterWriteColor.Color.ToArgb(),
				ApuRegisterReadColor = (uint)this.ApuRegisterReadColor.Color.ToArgb(),
				ApuRegisterWriteColor = (uint)this.ApuRegisterWriteColor.Color.ToArgb(),
				CpuRegisterReadColor = (uint)this.CpuRegisterReadColor.Color.ToArgb(),
				CpuRegisterWriteColor = (uint)this.CpuRegisterWriteColor.Color.ToArgb(),
				WorkRamRegisterReadColor = (uint)this.WorkRamRegisterReadColor.Color.ToArgb(),
				WorkRamRegisterWriteColor = (uint)this.WorkRamRegisterWriteColor.Color.ToArgb(),

				ShowDmaChannels = new bool[8] {
					this.ShowDmaChannel0,
					this.ShowDmaChannel1,
					this.ShowDmaChannel2,
					this.ShowDmaChannel3,
					this.ShowDmaChannel4,
					this.ShowDmaChannel5,
					this.ShowDmaChannel6,
					this.ShowDmaChannel7
				}
			};
		}
	}
}
