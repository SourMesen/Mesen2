using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class PceEventViewerConfig : ViewModelBase
	{
		[Reactive] public EventViewerCategoryCfg VdcStatusReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x29, 0xC9, 0xC9));
		[Reactive] public EventViewerCategoryCfg VdcVramWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));
		[Reactive] public EventViewerCategoryCfg VdcVramReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x29, 0xC9, 0x29));
		[Reactive] public EventViewerCategoryCfg VdcRegSelectWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x29, 0x29, 0xC9));
		[Reactive] public EventViewerCategoryCfg VdcControlWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x29, 0xC6, 0x93));
		[Reactive] public EventViewerCategoryCfg VdcRcrWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC6, 0x9F, 0x93));
		[Reactive] public EventViewerCategoryCfg VdcHvConfigWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x93, 0xC6, 0x9F));
		[Reactive] public EventViewerCategoryCfg VdcMemoryWidthWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC6, 0x29, 0xC6));
		[Reactive] public EventViewerCategoryCfg VdcScrollWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC6, 0xC6, 0x29));
		[Reactive] public EventViewerCategoryCfg VdcDmaWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x93, 0x29, 0xC9));
		
		[Reactive] public EventViewerCategoryCfg CdRomWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xDA, 0xB4, 0x7A));
		[Reactive] public EventViewerCategoryCfg CdRomReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x44, 0x53, 0xD7));
		[Reactive] public EventViewerCategoryCfg AdpcmWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD7, 0x7A, 0xDA));
		[Reactive] public EventViewerCategoryCfg AdpcmReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x53, 0x44));
		[Reactive] public EventViewerCategoryCfg ArcadeCardWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x7A, 0xDA, 0xD7));
		[Reactive] public EventViewerCategoryCfg ArcadeCardReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x44, 0xB4, 0x53));

		[Reactive] public EventViewerCategoryCfg VceWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		[Reactive] public EventViewerCategoryCfg VceReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));
		
		[Reactive] public EventViewerCategoryCfg PsgWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		[Reactive] public EventViewerCategoryCfg PsgReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));
		
		[Reactive] public EventViewerCategoryCfg TimerWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));
		[Reactive] public EventViewerCategoryCfg TimerReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x75, 0x97));

		[Reactive] public EventViewerCategoryCfg IoWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0x5E));
		[Reactive] public EventViewerCategoryCfg IoReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));
		[Reactive] public EventViewerCategoryCfg IrqControlWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0xDD));
		[Reactive] public EventViewerCategoryCfg IrqControlReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x98, 0xE4));

		[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC4, 0xF4, 0x7A));

		[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		[Reactive] public bool ShowPreviousFrameEvents { get; set; } = true;

		public InteropPceEventViewerConfig ToInterop()
		{
			return new InteropPceEventViewerConfig() {
				VdcStatusReads = this.VdcStatusReads,
				VdcVramWrites = this.VdcVramWrites,
				VdcVramReads = this.VdcVramReads,
				VdcRegSelectWrites = this.VdcRegSelectWrites,
				VdcControlWrites = this.VdcControlWrites,
				VdcRcrWrites = this.VdcRcrWrites,
				VdcHvConfigWrites = this.VdcHvConfigWrites,
				VdcMemoryWidthWrites = this.VdcMemoryWidthWrites,
				VdcScrollWrites = this.VdcScrollWrites,
				VdcDmaWrites = this.VdcDmaWrites,

				VceWrites = this.VceWrites,
				VceReads = this.VceReads,
				PsgWrites = this.PsgWrites,
				PsgReads = this.PsgReads,
				TimerWrites = this.TimerWrites,
				TimerReads = this.TimerReads,
				IoWrites = this.IoWrites,
				IoReads = this.IoReads,
				IrqControlWrites = this.IrqControlWrites,
				IrqControlReads = this.IrqControlReads,

				CdRomWrites = this.CdRomWrites,
				CdRomReads = this.CdRomReads,
				AdpcmWrites = this.AdpcmWrites,
				AdpcmReads = this.AdpcmReads,
				ArcadeCardReads = this.ArcadeCardReads,
				ArcadeCardWrites = this.ArcadeCardWrites,

				Irq = this.Irq,
				MarkedBreakpoints = this.MarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents
			};
		}
	}
}
