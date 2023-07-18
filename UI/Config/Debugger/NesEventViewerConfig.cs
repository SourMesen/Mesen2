using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class NesEventViewerConfig : ViewModelBase
	{
		[Reactive] public EventViewerCategoryCfg MapperRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x00, 0x6E, 0x8E));
		[Reactive] public EventViewerCategoryCfg MapperRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x78, 0xA5, 0xB2));
		
		[Reactive] public EventViewerCategoryCfg ApuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x97, 0x75, 0x00));
		[Reactive] public EventViewerCategoryCfg ApuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB7, 0xAA, 0x7B));
		
		[Reactive] public EventViewerCategoryCfg ControlRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x06, 0xDA, 0xDF));
		[Reactive] public EventViewerCategoryCfg ControlRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9B, 0xFF, 0xDE));

		[Reactive] public EventViewerCategoryCfg Ppu2000Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x96, 0x45, 0x45));
		[Reactive] public EventViewerCategoryCfg Ppu2001Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x8E, 0x33, 0xFF));
		[Reactive] public EventViewerCategoryCfg Ppu2003Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x84, 0xE0));
		[Reactive] public EventViewerCategoryCfg Ppu2004Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFA, 0xFF, 0x39));
		[Reactive] public EventViewerCategoryCfg Ppu2005Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x2E, 0xFF, 0x28));
		[Reactive] public EventViewerCategoryCfg Ppu2006Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x3D, 0x2D, 0xFF));
		[Reactive] public EventViewerCategoryCfg Ppu2007Write { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x06, 0x0D));

		[Reactive] public EventViewerCategoryCfg Ppu2002Read { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x74, 0x0A));
		[Reactive] public EventViewerCategoryCfg Ppu2004Read { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFD, 0xFF, 0xB3));
		[Reactive] public EventViewerCategoryCfg Ppu2007Read { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x7D, 0x7D));

		[Reactive] public EventViewerCategoryCfg DmcDmaReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0xFF, 0xFD));
		[Reactive] public EventViewerCategoryCfg OtherDmaReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0xC9, 0xFD));
		[Reactive] public EventViewerCategoryCfg SpriteZeroHit { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x84, 0x73, 0xC0));

		[Reactive] public EventViewerCategoryCfg Nmi { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xAB, 0xAD, 0xAC));
		[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC4, 0xF4, 0x7A));
	
		[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

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
				OtherDmaReads = this.OtherDmaReads,
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
