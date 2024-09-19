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

		private CpuType _cpuType;

		[Obsolete("For designer only")]
		public RefreshTimingViewModel() : this(new RefreshTimingConfig(), CpuType.Snes) { }

		public RefreshTimingViewModel(RefreshTimingConfig config, CpuType cpuType)
		{
			Config = config;
			_cpuType = cpuType;

			UpdateMinMaxValues(_cpuType);
			ResetCommand = ReactiveCommand.Create(Reset);
		}

		public void Reset()
		{
			Config.RefreshScanline = _cpuType.GetConsoleType() switch {
				ConsoleType.Snes => 240,
				ConsoleType.Nes => 241,
				ConsoleType.Gameboy => 144,
				ConsoleType.PcEngine => 240, //TODOv2
				ConsoleType.Sms => 192,
				ConsoleType.Gba => 160,
				ConsoleType.Ws => 144,
				_ => throw new Exception("Invalid console type")
			};

			Config.RefreshCycle = 0;
		}

		public void UpdateMinMaxValues(CpuType cpuType)
		{
			_cpuType = cpuType;
			TimingInfo timing = EmuApi.GetTimingInfo(_cpuType);
			MinScanline = timing.FirstScanline;
			MaxScanline = (int)timing.ScanlineCount + timing.FirstScanline - 1;
			MaxCycle = (int)timing.CycleCount - 1;

			if(Config.RefreshScanline < MinScanline || Config.RefreshScanline > MaxScanline || Config.RefreshCycle > MaxCycle) {
				Reset();
			}
		}
	}
}
