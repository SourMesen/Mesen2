using Avalonia.Media;
using Mesen.Interop;

namespace Mesen.Config
{
	public class GbEventViewerConfig
	{
		public EventViewerCategoryCfg PpuRegisterCgramWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x29, 0x29));
		public EventViewerCategoryCfg PpuRegisterVramWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		public EventViewerCategoryCfg PpuRegisterOamWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));
		public EventViewerCategoryCfg PpuRegisterBgScrollWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0x7C, 0xD9));
		public EventViewerCategoryCfg PpuRegisterWindowWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		public EventViewerCategoryCfg PpuRegisterOtherWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));
		public EventViewerCategoryCfg PpuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x75, 0x97));

		public EventViewerCategoryCfg CpuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0x5E));
		public EventViewerCategoryCfg CpuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		public EventViewerCategoryCfg ApuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));
		public EventViewerCategoryCfg ApuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));

		public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xAD, 0xAC));

		public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAF, 0xFF, 0xAF));

		public bool ShowPreviousFrameEvents = true;

		public InteropGbEventViewerConfig ToInterop()
		{
			return new InteropGbEventViewerConfig() {
				PpuRegisterCgramWrites = this.PpuRegisterCgramWrites,
				PpuRegisterVramWrites = this.PpuRegisterVramWrites,
				PpuRegisterOamWrites = this.PpuRegisterOamWrites,
				PpuRegisterBgScrollWrites = this.PpuRegisterBgScrollWrites,
				PpuRegisterWindowWrites = this.PpuRegisterWindowWrites,
				PpuRegisterOtherWrites = this.PpuRegisterOtherWrites,
				PpuRegisterReads = this.PpuRegisterReads,

				CpuRegisterWrites = this.CpuRegisterWrites,
				CpuRegisterReads = this.CpuRegisterReads,
				ApuRegisterWrites = this.ApuRegisterWrites,
				ApuRegisterReads = this.ApuRegisterReads,
				Irq = this.Irq,
				MarkedBreakpoints = this.MarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents
			};
		}
	}
}
