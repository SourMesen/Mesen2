using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config;

public class WsEventViewerConfig : ViewModelBase
{
	[Reactive] public EventViewerCategoryCfg PpuPaletteRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[0]);
	[Reactive] public EventViewerCategoryCfg PpuPaletteWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[1]);
	
	[Reactive] public EventViewerCategoryCfg PpuVramRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[2]);
	[Reactive] public EventViewerCategoryCfg PpuVramWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[3]);
	
	[Reactive] public EventViewerCategoryCfg PpuScrollRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[4]);
	[Reactive] public EventViewerCategoryCfg PpuScrollWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[5]);
	[Reactive] public EventViewerCategoryCfg PpuWindowRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[6]);
	[Reactive] public EventViewerCategoryCfg PpuWindowWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[7]);
	[Reactive] public EventViewerCategoryCfg PpuOtherRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[8]);
	[Reactive] public EventViewerCategoryCfg PpuOtherWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[9]);

	[Reactive] public EventViewerCategoryCfg AudioRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[10]);
	[Reactive] public EventViewerCategoryCfg AudioWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[11]);
	
	[Reactive] public EventViewerCategoryCfg SerialRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[12]);
	[Reactive] public EventViewerCategoryCfg SerialWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[13]);

	[Reactive] public EventViewerCategoryCfg DmaRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[14]);
	[Reactive] public EventViewerCategoryCfg DmaWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[15]);

	[Reactive] public EventViewerCategoryCfg InputRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[16]);
	[Reactive] public EventViewerCategoryCfg InputWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[17]);

	[Reactive] public EventViewerCategoryCfg IrqRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[18]);
	[Reactive] public EventViewerCategoryCfg IrqWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[19]);

	[Reactive] public EventViewerCategoryCfg TimerRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[20]);
	[Reactive] public EventViewerCategoryCfg TimerWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[21]);

	[Reactive] public EventViewerCategoryCfg EepromRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[22]);
	[Reactive] public EventViewerCategoryCfg EepromWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[23]);

	[Reactive] public EventViewerCategoryCfg CartRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[24]);
	[Reactive] public EventViewerCategoryCfg CartWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[25]);

	[Reactive] public EventViewerCategoryCfg OtherRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[26]);
	[Reactive] public EventViewerCategoryCfg OtherWrite { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[27]);

	[Reactive] public EventViewerCategoryCfg PpuVCounterRead { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[28]);

	[Reactive] public EventViewerCategoryCfg Irq { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[29]);

	[Reactive] public EventViewerCategoryCfg MarkedBreakpoints { get; set; } = new EventViewerCategoryCfg(EventViewerColors.Colors[30]);

	[Reactive] public bool ShowPreviousFrameEvents { get; set; } = true;

	public InteropWsEventViewerConfig ToInterop()
	{
		return new InteropWsEventViewerConfig() {
			PpuPaletteRead = this.PpuPaletteRead,
			PpuPaletteWrite = this.PpuPaletteWrite,
			PpuVramRead = this.PpuVramRead,
			PpuVramWrite = this.PpuVramWrite,
			PpuVCounterRead = this.PpuVCounterRead,
			PpuScrollRead = this.PpuScrollRead,
			PpuScrollWrite = this.PpuScrollWrite,
			PpuWindowRead = this.PpuWindowRead,
			PpuWindowWrite = this.PpuWindowWrite,
			PpuOtherRead = this.PpuOtherRead,
			PpuOtherWrite = this.PpuOtherWrite,
			AudioRead = this.AudioRead,
			AudioWrite = this.AudioWrite,
			SerialRead = this.SerialRead,
			SerialWrite = this.SerialWrite,
			DmaRead = this.DmaRead,
			DmaWrite = this.DmaWrite,
			InputRead = this.InputRead,
			InputWrite = this.InputWrite,
			IrqRead = this.IrqRead,
			IrqWrite = this.IrqWrite,
			TimerRead = this.TimerRead,
			TimerWrite = this.TimerWrite,
			EepromRead = this.EepromRead,
			EepromWrite = this.EepromWrite,
			CartRead = this.CartRead,
			CartWrite = this.CartWrite,
			OtherRead = this.OtherRead,
			OtherWrite = this.OtherWrite,
			Irq = this.Irq,
			MarkedBreakpoints = this.MarkedBreakpoints,

			ShowPreviousFrameEvents = this.ShowPreviousFrameEvents
		};
	}
}
