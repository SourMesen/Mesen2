using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class NesEventViewerConfig : ViewModelBase
	{
		[Reactive] public EventViewerCategoryCfg MapperRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x29, 0x29));
		[Reactive] public EventViewerCategoryCfg MapperRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		
		[Reactive] public EventViewerCategoryCfg ApuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));
		[Reactive] public EventViewerCategoryCfg ApuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFE, 0x78, 0x7B));
		
		[Reactive] public EventViewerCategoryCfg ControlRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xBF, 0x80, 0x20));
		[Reactive] public EventViewerCategoryCfg ControlRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0x7C, 0xD9));

		[Reactive] public EventViewerCategoryCfg Ppu2000Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		[Reactive] public EventViewerCategoryCfg Ppu2001Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));
		[Reactive] public EventViewerCategoryCfg Ppu2003Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x75, 0x97));
		[Reactive] public EventViewerCategoryCfg Ppu2004Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x5E, 0x5E));
		[Reactive] public EventViewerCategoryCfg Ppu2005Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));
		[Reactive] public EventViewerCategoryCfg Ppu2006Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));
		[Reactive] public EventViewerCategoryCfg Ppu2007Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));

		[Reactive] public EventViewerCategoryCfg Ppu2002Read { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE4, 0x98, 0x18));
		[Reactive] public EventViewerCategoryCfg Ppu2004Read { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x93, 0xC6, 0x9F));
		[Reactive] public EventViewerCategoryCfg Ppu2007Read { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAC, 0xF9, 0xFE));

		[Reactive] public EventViewerCategoryCfg DmcDmaReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x2E, 0xFF, 0x28));
		[Reactive] public EventViewerCategoryCfg SpriteZeroHit { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x8E, 0x33, 0xFF));

		[Reactive] public EventViewerCategoryCfg Nmi { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xAD, 0xAC));
		[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xAD, 0xAC));
	
		[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAF, 0xFF, 0xAF));

		[Reactive] public bool ShowPreviousFrameEvents { get; set; } = true;
		[Reactive] public bool ShowNtscBorders { get; set; } = false;

		public InteropNesEventViewerConfig ToInterop()
		{
			return new InteropNesEventViewerConfig() {
				MapperRegisterWrites = this.MapperRegisterWrites,
				MapperRegisterReads = this.MapperRegisterReads,
				ApuRegisterWrites = this.ApuRegisterWrites,
				ApuRegisterReads = this.ApuRegisterReads,
				ControlRegisterWrites = this.ControlRegisterWrites,
				ControlRegisterReads = this.ControlRegisterReads,
				Ppu2000Write = this.Ppu2000Write,
				Ppu2001Write = this.Ppu2001Write,
				Ppu2003Write = this.Ppu2003Write,
				Ppu2004Write = this.Ppu2004Write,
				Ppu2005Write = this.Ppu2005Write,
				Ppu2006Write = this.Ppu2006Write,
				Ppu2007Write = this.Ppu2007Write,

				Ppu2002Read = this.Ppu2002Read,
				Ppu2004Read = this.Ppu2004Read,
				Ppu2007Read = this.Ppu2007Read,
				DmcDmaReads = this.DmcDmaReads,
				SpriteZeroHit = this.SpriteZeroHit,
				
				Nmi = this.Nmi,
				Irq = this.Irq,
				MarkedBreakpoints = this.MarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents,
				ShowNtscBorders = this.ShowNtscBorders
			};
		}
	}
}
