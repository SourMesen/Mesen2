using Avalonia.Media;
using Mesen.Interop;

namespace Mesen.Config
{
	public class SnesEventViewerConfig
	{
		public EventViewerCategoryCfg PpuRegisterCgramWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x29, 0x29));
		public EventViewerCategoryCfg PpuRegisterVramWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		public EventViewerCategoryCfg PpuRegisterOamWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));
		public EventViewerCategoryCfg PpuRegisterMode7Writes { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFE, 0x78, 0x7B));
		public EventViewerCategoryCfg PpuRegisterBgOptionWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xBF, 0x80, 0x20));
		public EventViewerCategoryCfg PpuRegisterBgScrollWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0x7C, 0xD9));
		public EventViewerCategoryCfg PpuRegisterWindowWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		public EventViewerCategoryCfg PpuRegisterOtherWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));
		public EventViewerCategoryCfg PpuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x75, 0x97));

		public EventViewerCategoryCfg CpuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0x5E));
		public EventViewerCategoryCfg CpuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		public EventViewerCategoryCfg ApuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));
		public EventViewerCategoryCfg ApuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));

		public EventViewerCategoryCfg WorkRamRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x2E, 0xFF, 0x28));
		public EventViewerCategoryCfg WorkRamRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x8E, 0x33, 0xFF));

		public EventViewerCategoryCfg Nmi { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xAD, 0xAC));
		public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xAD, 0xAC));

		public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAF, 0xFF, 0xAF));

		public bool ShowPreviousFrameEvents { get; set; } = true;

		public bool ShowDmaChannel0 { get; set; } = true;
		public bool ShowDmaChannel1 { get; set; } = true;
		public bool ShowDmaChannel2 { get; set; } = true;
		public bool ShowDmaChannel3 { get; set; } = true;
		public bool ShowDmaChannel4 { get; set; } = true;
		public bool ShowDmaChannel5 { get; set; } = true;
		public bool ShowDmaChannel6 { get; set; } = true;
		public bool ShowDmaChannel7 { get; set; } = true;

		public InteropSnesEventViewerConfig ToInterop()
		{
			return new InteropSnesEventViewerConfig() {
				PpuRegisterCgramWrites = this.PpuRegisterCgramWrites,
				PpuRegisterVramWrites = this.PpuRegisterVramWrites,
				PpuRegisterOamWrites = this.PpuRegisterOamWrites,
				PpuRegisterMode7Writes = this.PpuRegisterMode7Writes,
				PpuRegisterBgOptionWrites = this.PpuRegisterBgOptionWrites,
				PpuRegisterBgScrollWrites = this.PpuRegisterBgScrollWrites,
				PpuRegisterWindowWrites = this.PpuRegisterWindowWrites,
				PpuRegisterOtherWrites = this.PpuRegisterOtherWrites,
				PpuRegisterReads = this.PpuRegisterReads,

				CpuRegisterWrites = this.CpuRegisterWrites,
				CpuRegisterReads = this.CpuRegisterReads,
				ApuRegisterWrites = this.ApuRegisterWrites,
				ApuRegisterReads = this.ApuRegisterReads,
				WorkRamRegisterWrites = this.WorkRamRegisterWrites,
				WorkRamRegisterReads = this.WorkRamRegisterReads,
				Nmi = this.Nmi,
				Irq = this.Irq,
				MarkedBreakpoints = this.MarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents,

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
