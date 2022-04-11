using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class PceEventViewerConfig : ViewModelBase
	{
		[Reactive] public EventViewerCategoryCfg VdcWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x29, 0x29));
		[Reactive] public EventViewerCategoryCfg VdcReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));

		[Reactive] public EventViewerCategoryCfg VceWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		[Reactive] public EventViewerCategoryCfg VceReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));
		
		//[Reactive] public EventViewerCategoryCfg BgScrollWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0x7C, 0xD9));
		
		[Reactive] public EventViewerCategoryCfg PsgWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		[Reactive] public EventViewerCategoryCfg PsgReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));
		
		[Reactive] public EventViewerCategoryCfg TimerWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));
		[Reactive] public EventViewerCategoryCfg TimerReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x75, 0x97));

		[Reactive] public EventViewerCategoryCfg IoWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0x5E));
		[Reactive] public EventViewerCategoryCfg IoReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xAD, 0xAC));

		[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAF, 0xFF, 0xAF));

		[Reactive] public bool ShowPreviousFrameEvents { get; set; } = true;

		public InteropPceEventViewerConfig ToInterop()
		{
			return new InteropPceEventViewerConfig() {
				VdcWrites = this.VdcWrites,
				VdcReads = this.VdcReads,
				VceWrites = this.VceWrites,
				VceReads = this.VceReads,
				PsgWrites = this.PsgWrites,
				PsgReads = this.PsgReads,
				TimerWrites = this.TimerWrites,
				TimerReads = this.TimerReads,
				IoWrites = this.IoWrites,
				IoReads = this.IoReads,
				Irq = this.Irq,
				MarkedBreakpoints = this.MarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents
			};
		}
	}
}
