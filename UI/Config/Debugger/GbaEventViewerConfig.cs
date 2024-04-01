using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class GbaEventViewerConfig : ViewModelBase
	{
		[Reactive] public EventViewerCategoryCfg PaletteReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x29, 0xC9, 0x29));
		[Reactive] public EventViewerCategoryCfg PaletteWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x29, 0x29));
		[Reactive] public EventViewerCategoryCfg VramReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xDA, 0xB4, 0x7A));
		[Reactive] public EventViewerCategoryCfg VramWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xB4, 0x7A, 0xDA));
		[Reactive] public EventViewerCategoryCfg OamReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x44, 0x53, 0xD7));
		[Reactive] public EventViewerCategoryCfg OamWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x53, 0xD7, 0x44));

		[Reactive] public EventViewerCategoryCfg PpuRegisterBgScrollReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD9, 0x4A, 0x7C));
		[Reactive] public EventViewerCategoryCfg PpuRegisterBgScrollWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x4A, 0x7C, 0xD9));
		[Reactive] public EventViewerCategoryCfg PpuRegisterWindowReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF7, 0xE2, 0x51));
		[Reactive] public EventViewerCategoryCfg PpuRegisterWindowWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xE2, 0x51, 0xF7));
		[Reactive] public EventViewerCategoryCfg PpuRegisterOtherReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x42, 0xD1, 0xDD));
		[Reactive] public EventViewerCategoryCfg PpuRegisterOtherWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xD1, 0xDD, 0x42));

		[Reactive] public EventViewerCategoryCfg SerialWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x7A, 0x7C, 0x29));
		[Reactive] public EventViewerCategoryCfg SerialReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0x29));

		[Reactive] public EventViewerCategoryCfg InputWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xFF, 0x42, 0x44));
		[Reactive] public EventViewerCategoryCfg InputReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x51, 0xE4));

		[Reactive] public EventViewerCategoryCfg TimerWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x51, 0xFF, 0x44));
		[Reactive] public EventViewerCategoryCfg TimerReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		[Reactive] public EventViewerCategoryCfg OtherRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x98, 0x5E, 0xFF));
		[Reactive] public EventViewerCategoryCfg OtherRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0x98, 0xE4));

		[Reactive] public EventViewerCategoryCfg ApuRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x9F, 0x93, 0xC6));
		[Reactive] public EventViewerCategoryCfg ApuRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xF9, 0xFE, 0xAC));

		[Reactive] public EventViewerCategoryCfg DmaRegisterWrites { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x51, 0xDA, 0x4A));
		[Reactive] public EventViewerCategoryCfg DmaRegisterReads { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC9, 0xB4, 0x7A));

		[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0xC4, 0xF4, 0x7A));

		[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(Color.FromRgb(0x18, 0x98, 0xE4));

		[Reactive] public bool ShowPreviousFrameEvents { get; set; } = true;

		public InteropGbaEventViewerConfig ToInterop()
		{
			return new InteropGbaEventViewerConfig() {
				PaletteWrites = this.PaletteWrites,
				VramWrites = this.VramWrites,
				OamWrites = this.OamWrites,
				PpuRegisterBgScrollWrites = this.PpuRegisterBgScrollWrites,
				PpuRegisterWindowWrites = this.PpuRegisterWindowWrites,
				PpuRegisterOtherWrites = this.PpuRegisterOtherWrites,

				PaletteReads = this.PaletteReads,
				VramReads = this.VramReads,
				OamReads = this.OamReads,
				PpuRegisterBgScrollReads = this.PpuRegisterBgScrollReads,
				PpuRegisterWindowReads = this.PpuRegisterWindowReads,
				PpuRegisterOtherReads = this.PpuRegisterOtherReads,

				SerialWrites = this.SerialWrites,
				SerialReads = this.SerialReads,

				InputWrites = this.InputWrites,
				InputReads = this.InputReads,

				TimerWrites = this.TimerWrites,
				TimerReads = this.TimerReads,

				OtherRegisterWrites = this.OtherRegisterWrites,
				OtherRegisterReads = this.OtherRegisterReads,

				ApuRegisterWrites = this.ApuRegisterWrites,
				ApuRegisterReads = this.ApuRegisterReads,

				DmaRegisterWrites = this.DmaRegisterWrites,
				DmaRegisterReads = this.DmaRegisterReads,

				Irq = this.Irq,
				MarkedBreakpoints = this.MarkedBreakpoints,
				ShowPreviousFrameEvents = this.ShowPreviousFrameEvents
			};
		}
	}
}
