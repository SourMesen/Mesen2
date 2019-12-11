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
		
		public XmlColor IrqColor = ColorTranslator.FromHtml("#FFADAC");
		public XmlColor NmiColor = ColorTranslator.FromHtml("#FFADAC");
		public XmlColor BreakpointColor = ColorTranslator.FromHtml("#AFFFAF");
		public XmlColor ApuRegisterReadColor = ColorTranslator.FromHtml("#F9FEAC");
		public XmlColor ApuRegisterWriteColor = ColorTranslator.FromHtml("#9F93C6");
		public XmlColor CpuRegisterReadColor = ColorTranslator.FromHtml("#1898E4");
		public XmlColor CpuRegisterWriteColor = ColorTranslator.FromHtml("#FF5E5E");
		public XmlColor WorkRamRegisterReadColor = ColorTranslator.FromHtml("#8E33FF");
		public XmlColor WorkRamRegisterWriteColor = ColorTranslator.FromHtml("#2EFF28");

		public XmlColor PpuRegisterReadColor = ColorTranslator.FromHtml("#007597");
		public XmlColor PpuRegisterWriteCgramColor = ColorTranslator.FromHtml("#C92929");
		public XmlColor PpuRegisterWriteVramColor = ColorTranslator.FromHtml("#B47ADA");
		public XmlColor PpuRegisterWriteOamColor = ColorTranslator.FromHtml("#53D744");
		public XmlColor PpuRegisterWriteBgOptionColor = ColorTranslator.FromHtml("#BF8020");
		public XmlColor PpuRegisterWriteBgScrollColor = ColorTranslator.FromHtml("#4A7CD9");
		public XmlColor PpuRegisterWriteMode7Color = ColorTranslator.FromHtml("#FE787B");
		public XmlColor PpuRegisterWriteWindowColor = ColorTranslator.FromHtml("#E251F7");
		public XmlColor PpuRegisterWriteOtherColor = ColorTranslator.FromHtml("#D1DD42");

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
