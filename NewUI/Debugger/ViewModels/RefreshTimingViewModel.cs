using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class RefreshTimingViewModel : ViewModelBase
	{
		public RefreshTimingConfig Config { get; }

		[Reactive] public int MinScanline { get; private set; }
		[Reactive] public int MaxScanline { get; private set; }
		[Reactive] public int MaxCycle { get; private set; }

		public ReactiveCommand<Unit, Unit> ResetCommand { get; }

		[Obsolete("For designer only")]
		public RefreshTimingViewModel() : this(new RefreshTimingConfig()) { }

		public RefreshTimingViewModel(RefreshTimingConfig config)
		{
			Config = config;

			UpdateMinMaxValues();
			ResetCommand = ReactiveCommand.Create(Reset);
		}

		public void Reset()
		{
			Config.RefreshScanline = EmuApi.GetRomInfo().ConsoleType switch {
				ConsoleType.Snes => 240,
				ConsoleType.Nes => 241,
				ConsoleType.Gameboy => 144,
				ConsoleType.GameboyColor => 144,
				ConsoleType.PcEngine => 240, //TODO
				_ => throw new Exception("Invalid console type")
			};

			Config.RefreshCycle = 0;
		}

		public void UpdateMinMaxValues()
		{
			TimingInfo timing = EmuApi.GetTimingInfo();
			MinScanline = timing.FirstScanline;
			MaxScanline = (int)timing.ScanlineCount + timing.FirstScanline - 1;
			MaxCycle = (int)timing.CycleCount - 1;

			if(Config.RefreshScanline < MinScanline || Config.RefreshScanline > MaxScanline || Config.RefreshCycle > MaxCycle) {
				Reset();
			}
		}
	}
}
