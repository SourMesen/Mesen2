using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class SmsEventViewerConfig : ViewModelBase
	{
		[Reactive] public EventViewerCategoryCfg VdpPaletteWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x29, 0x29));
		[Reactive] public EventViewerCategoryCfg VdpVramWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		[Reactive] public EventViewerCategoryCfg VdpVCounterRead { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));
		[Reactive] public EventViewerCategoryCfg VdpHCounterRead { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0x7C, 0xD9));
		[Reactive] public EventViewerCategoryCfg VdpVramRead { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		[Reactive] public EventViewerCategoryCfg VdpControlPortRead { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));
		[Reactive] public EventViewerCategoryCfg VdpControlPortWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x75, 0x97));

		[Reactive] public EventViewerCategoryCfg PsgWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0x5E));
		[Reactive] public EventViewerCategoryCfg IoWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));
		[Reactive] public EventViewerCategoryCfg IoRead { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));
		
		[Reactive] public EventViewerCategoryCfg MemoryControlWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0xFE, 0xAC));

		[Reactive] public EventViewerCategoryCfg GameGearPortWrite { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));
		[Reactive] public EventViewerCategoryCfg GameGearPortRead { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAC, 0xFE, 0xAC));

		[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC4, 0xF4, 0x7A));
		[Reactive] public EventViewerCategoryCfg Nmi { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF4, 0xF4, 0x7A));

		[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		[Reactive] public bool ShowPreviousFrameEvents { get; set; } = true;

		public InteropSmsEventViewerConfig ToInterop()
		{
			return new InteropSmsEventViewerConfig() {
				Irq = this.Irq,
				Nmi = this.Nmi,
				MarkedBreakpoints = this.MarkedBreakpoints,
				
				VdpPaletteWrite = this.VdpPaletteWrite,
				VdpVramWrite = this.VdpVramWrite,
				
				VdpVCounterRead = this.VdpVCounterRead,
				VdpHCounterRead = this.VdpHCounterRead,
				VdpVramRead = this.VdpVramRead,
				VdpControlPortRead = this.VdpControlPortRead,
				VdpControlPortWrite = this.VdpControlPortWrite,
				
				PsgWrite = this.PsgWrite,
				IoWrite = this.IoWrite,
				IoRead = this.IoRead,

				MemoryControlWrite = this.MemoryControlWrite,
				GameGearPortWrite = this.GameGearPortWrite,
				GameGearPortRead = this.GameGearPortRead,

				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents
			};
		}
	}
}
