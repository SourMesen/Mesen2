using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI;
using System;
using System.Reactive;

namespace Mesen.Debugger.ViewModels
{
	public class RefreshTimingViewModel : ViewModelBase
	{
		public RefreshTimingConfig Config { get; }
		
		public int MinScanline { get; }
		public int MaxScanline { get; }
		public int MaxCycle { get; }
		
		public ReactiveCommand<Unit, Unit> ResetCommand { get; }

		[Obsolete("For designer only")]
		public RefreshTimingViewModel() : this(new RefreshTimingConfig()) { }

		public RefreshTimingViewModel(RefreshTimingConfig config)
		{
			Config = config;

			TimingInfo timing = EmuApi.GetTimingInfo();
			MinScanline = timing.FirstScanline;
			MaxScanline = (int)timing.ScanlineCount + timing.FirstScanline - 1;
			MaxCycle = (int)timing.CycleCount - 1;

			config.RefreshScanline = Math.Max(MinScanline, Math.Min(MaxScanline, config.RefreshScanline));
			config.RefreshCycle = Math.Min(MaxCycle, config.RefreshCycle);

			ResetCommand = ReactiveCommand.Create(Reset);
		}

		public void Reset()
		{
			Config.RefreshScanline = EmuApi.GetRomInfo().ConsoleType switch {
				ConsoleType.Snes => 240,
				ConsoleType.Nes => 241,
				ConsoleType.Gameboy => 144,
				ConsoleType.GameboyColor => 144,
				_ => throw new Exception("Invalid console type")
			};

			Config.RefreshCycle = 0;
		}
	}
}
