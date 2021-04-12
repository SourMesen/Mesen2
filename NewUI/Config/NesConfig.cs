using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class NesConfig : BaseConfig<NesConfig>
	{
		[Reactive] public bool EnableHdPacks { get; set; } = true;
		[Reactive] public bool DisableGameDatabase { get; set; } = false;
		[Reactive] public bool FdsAutoLoadDisk { get; set; } = true;
		[Reactive] public bool FdsFastForwardOnLoad { get; set; } = false;
		[Reactive] public bool FdsAutoInsertDisk { get; set; } = false;
		[Reactive] public VsDualOutputOption VsDualVideoOutput { get; set; } = VsDualOutputOption.Both;
		[Reactive] public VsDualOutputOption VsDualAudioOutput { get; set; } = VsDualOutputOption.Both;
		[Reactive] public bool NsfMoveToNextTrackAfterTime { get; set; } = true;
		[Reactive] public UInt32 NsfMoveToNextTrackTime { get; set; } = 120;
		[Reactive] public bool NsfAutoDetectSilence { get; set; } = true;
		[Reactive] public UInt32 NsfAutoDetectSilenceDelay { get; set; } = 3000;

		[Reactive] public bool DisableSprites { get; set; } = false;
		[Reactive] public bool DisableBackground { get; set; } = false;
		[Reactive] public bool ForceBackgroundFirstColumn { get; set; } = false;
		[Reactive] public bool ForceSpritesFirstColumn { get; set; } = false;
		[Reactive] public bool RemoveSpriteLimit { get; set; } = false;
		[Reactive] public bool AdaptiveSpriteLimit { get; set; } = false;

		[Reactive] public bool EnableOamDecay { get; set; } = false;
		[Reactive] public bool EnablePpuOamRowCorruption { get; set; } = false;
		[Reactive] public bool DisableOamAddrBug { get; set; } = false;
		[Reactive] public bool DisablePaletteRead { get; set; } = false;
		[Reactive] public bool DisablePpu2004Reads { get; set; } = false;
		[Reactive] public bool EnablePpu2000ScrollGlitch { get; set; } = false;
		[Reactive] public bool EnablePpu2006ScrollGlitch { get; set; } = false;
		
		[Reactive] public bool UseNes101Hvc101Behavior { get; set; } = false;
		[Reactive] public bool DisablePpuReset { get; set; } = false;
		[Reactive] public bool AllowInvalidInput { get; set; } = false;

		[Reactive] public bool RandomizeMapperPowerOnState { get; set; } = false;
		[Reactive] public bool RandomizeCpuPpuAlignment { get; set; } = false;
		[Reactive] public RamState RamPowerOnState { get; set; } = RamState.AllZeros;

		[Reactive] public UInt32 PpuExtraScanlinesBeforeNmi { get; set; } = 0;
		[Reactive] public UInt32 PpuExtraScanlinesAfterNmi { get; set; } = 0;

		[Reactive] public bool DisableNoiseModeFlag { get; set; } = false;
		[Reactive] public bool ReduceDmcPopping { get; set; } = false;
		[Reactive] public bool SilenceTriangleHighFreq { get; set; } = false;
		[Reactive] public bool SwapDutyCycles { get; set; } = false;

		[Reactive] public bool BreakOnCrash { get; set; } = false;
		[Reactive] public UInt32 DipSwitches { get; set; } = 0;

		[Reactive] public Int32 InputScanline { get; set; } = 241;

		[Reactive] public bool IsFullColorPalette { get; set; } = false;
		[Reactive] public UInt32[] UserPalette { get; set; } = new UInt32[512];

		[Reactive] [MinMax(0, 100)] public UInt32 Square1Volume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 Square2Volume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 TriangleVolume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 NoiseVolume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 DmcVolume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 FdsVolume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 Mmc5Volume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 Vrc6Volume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 Vrc7Volume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 Namco163Volume { get; set; } = 100;
		[Reactive] [MinMax(0, 100)] public UInt32 Sunsoft5bVolume { get; set; } = 100;

		[Reactive] [MinMax(-100, 100)] public Int32 Square1Panning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 Square2Panning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 TrianglePanning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 NoisePanning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 DmcPanning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 FdsPanning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 Mmc5Panning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 Vrc6Panning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 Vrc7Panning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 Namco163Panning { get; set; } = 0;
		[Reactive] [MinMax(-100, 100)] public Int32 Sunsoft5bPanning { get; set; } = 0;

		[Reactive] public StereoFilter StereoFilter { get; set; } = StereoFilter.None;
		[Reactive] [MinMax(0, 100)] public Int32 StereoDelay { get; set; } = 15;
		[Reactive] [MinMax(-180, 180)] public Int32 StereoPanningAngle { get; set; } = 15;
		[Reactive] [MinMax(1, 100)] public Int32 StereoCombFilterDelay { get; set; } = 5;
		[Reactive] [MinMax(1, 200)] public Int32 StereoCombFilterStrength { get; set; } = 100;

		public void ApplyConfig()
		{
			ConfigApi.SetNesConfig(new InteropNesConfig() {
				EnableHdPacks = EnableHdPacks,
				DisableGameDatabase = DisableGameDatabase,
				FdsAutoLoadDisk = FdsAutoLoadDisk,
				FdsFastForwardOnLoad = FdsFastForwardOnLoad,
				FdsAutoInsertDisk = FdsAutoInsertDisk,
				VsDualVideoOutput = VsDualVideoOutput,
				VsDualAudioOutput = VsDualAudioOutput,
				NsfMoveToNextTrackAfterTime = NsfMoveToNextTrackAfterTime,
				NsfMoveToNextTrackTime = NsfMoveToNextTrackTime,
				NsfAutoDetectSilence = NsfAutoDetectSilence,
				NsfAutoDetectSilenceDelay = NsfAutoDetectSilenceDelay,

				SpritesEnabled = !DisableSprites,
				BackgroundEnabled = !DisableBackground,
				ForceBackgroundFirstColumn = ForceBackgroundFirstColumn,
				ForceSpritesFirstColumn = ForceSpritesFirstColumn,
				RemoveSpriteLimit = RemoveSpriteLimit,
				AdaptiveSpriteLimit = AdaptiveSpriteLimit,
				
				UseNes101Hvc101Behavior = UseNes101Hvc101Behavior,
				DisablePpuReset = DisablePpuReset,
				AllowInvalidInput = AllowInvalidInput,

				EnableOamDecay = EnableOamDecay,
				EnablePpuOamRowCorruption = EnablePpuOamRowCorruption,
				DisableOamAddrBug = DisableOamAddrBug,
				DisablePaletteRead = DisablePaletteRead,
				DisablePpu2004Reads = DisablePpu2004Reads,
				EnablePpu2000ScrollGlitch = EnablePpu2000ScrollGlitch,
				EnablePpu2006ScrollGlitch = EnablePpu2006ScrollGlitch,

				RandomizeMapperPowerOnState = RandomizeMapperPowerOnState,
				RandomizeCpuPpuAlignment = RandomizeCpuPpuAlignment,
				RamPowerOnState = RamPowerOnState,

				PpuExtraScanlinesAfterNmi = PpuExtraScanlinesAfterNmi,
				PpuExtraScanlinesBeforeNmi = PpuExtraScanlinesBeforeNmi,

				DisableNoiseModeFlag = DisableNoiseModeFlag,
				ReduceDmcPopping = ReduceDmcPopping,
				SilenceTriangleHighFreq = SilenceTriangleHighFreq,
				SwapDutyCycles = SwapDutyCycles,

				BreakOnCrash = BreakOnCrash,
				DipSwitches = DipSwitches,
				
				InputScanline = InputScanline,

				IsFullColorPalette = IsFullColorPalette,
				UserPalette = UserPalette,

				Square1Volume = Square1Volume,
				Square2Volume = Square2Volume,
				TriangleVolume = TriangleVolume,
				NoiseVolume = NoiseVolume,
				DmcVolume = DmcVolume,
				FdsVolume = FdsVolume,
				Mmc5Volume = Mmc5Volume,
				Vrc6Volume = Vrc6Volume,
				Vrc7Volume = Vrc7Volume,
				Namco163Volume = Namco163Volume,
				Sunsoft5bVolume = Sunsoft5bVolume,
				Square1Panning = Square1Panning,
				Square2Panning = Square2Panning,
				TrianglePanning = TrianglePanning,
				NoisePanning = NoisePanning,
				DmcPanning = DmcPanning,
				FdsPanning = FdsPanning,
				Mmc5Panning = Mmc5Panning,
				Vrc6Panning = Vrc6Panning,
				Vrc7Panning = Vrc7Panning,
				Namco163Panning = Namco163Panning,
				Sunsoft5bPanning = Sunsoft5bPanning,

				StereoFilter = StereoFilter,
				StereoDelay = StereoDelay,
				StereoPanningAngle = StereoPanningAngle,
				StereoCombFilterDelay = StereoCombFilterDelay,
				StereoCombFilterStrength = StereoCombFilterStrength,
			});
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropNesConfig
	{
		[MarshalAs(UnmanagedType.I1)] public bool EnableHdPacks;
		[MarshalAs(UnmanagedType.I1)] public bool DisableGameDatabase;
		[MarshalAs(UnmanagedType.I1)] public bool FdsAutoLoadDisk;
		[MarshalAs(UnmanagedType.I1)] public bool FdsFastForwardOnLoad;
		[MarshalAs(UnmanagedType.I1)] public bool FdsAutoInsertDisk;
		public VsDualOutputOption VsDualVideoOutput;
		public VsDualOutputOption VsDualAudioOutput;
		[MarshalAs(UnmanagedType.I1)] public bool NsfMoveToNextTrackAfterTime;
		public UInt32 NsfMoveToNextTrackTime;
		[MarshalAs(UnmanagedType.I1)] public bool NsfAutoDetectSilence;
		public UInt32 NsfAutoDetectSilenceDelay;

		[MarshalAs(UnmanagedType.I1)] public bool SpritesEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool BackgroundEnabled;
		[MarshalAs(UnmanagedType.I1)] public bool ForceBackgroundFirstColumn;
		[MarshalAs(UnmanagedType.I1)] public bool ForceSpritesFirstColumn;
		[MarshalAs(UnmanagedType.I1)] public bool RemoveSpriteLimit;
		[MarshalAs(UnmanagedType.I1)] public bool AdaptiveSpriteLimit;

		[MarshalAs(UnmanagedType.I1)] public bool UseNes101Hvc101Behavior;
		[MarshalAs(UnmanagedType.I1)] public bool DisablePpuReset;
		[MarshalAs(UnmanagedType.I1)] public bool AllowInvalidInput;

		[MarshalAs(UnmanagedType.I1)] public bool EnableOamDecay;
		[MarshalAs(UnmanagedType.I1)] public bool EnablePpuOamRowCorruption;
		[MarshalAs(UnmanagedType.I1)] public bool DisableOamAddrBug;
		[MarshalAs(UnmanagedType.I1)] public bool DisablePaletteRead;
		[MarshalAs(UnmanagedType.I1)] public bool DisablePpu2004Reads;
		[MarshalAs(UnmanagedType.I1)] public bool EnablePpu2000ScrollGlitch;
		[MarshalAs(UnmanagedType.I1)] public bool EnablePpu2006ScrollGlitch;

		[MarshalAs(UnmanagedType.I1)] public bool RandomizeMapperPowerOnState;
		[MarshalAs(UnmanagedType.I1)] public bool RandomizeCpuPpuAlignment;
		public RamState RamPowerOnState;

		public UInt32 PpuExtraScanlinesBeforeNmi;
		public UInt32 PpuExtraScanlinesAfterNmi;

		[MarshalAs(UnmanagedType.I1)] public bool DisableNoiseModeFlag;
		[MarshalAs(UnmanagedType.I1)] public bool ReduceDmcPopping;
		[MarshalAs(UnmanagedType.I1)] public bool SilenceTriangleHighFreq;
		[MarshalAs(UnmanagedType.I1)] public bool SwapDutyCycles;

		[MarshalAs(UnmanagedType.I1)] public bool BreakOnCrash;
		public UInt32 DipSwitches;

		public Int32 InputScanline;

		[MarshalAs(UnmanagedType.I1)] public bool IsFullColorPalette;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 512)]
		public UInt32[] UserPalette;

		public UInt32 Square1Volume;
		public UInt32 Square2Volume;
		public UInt32 TriangleVolume;
		public UInt32 NoiseVolume;
		public UInt32 DmcVolume;
		public UInt32 FdsVolume;
		public UInt32 Mmc5Volume;
		public UInt32 Vrc6Volume;
		public UInt32 Vrc7Volume;
		public UInt32 Namco163Volume;
		public UInt32 Sunsoft5bVolume;

		public Int32 Square1Panning;
		public Int32 Square2Panning;
		public Int32 TrianglePanning;
		public Int32 NoisePanning;
		public Int32 DmcPanning;
		public Int32 FdsPanning;
		public Int32 Mmc5Panning;
		public Int32 Vrc6Panning;
		public Int32 Vrc7Panning;
		public Int32 Namco163Panning;
		public Int32 Sunsoft5bPanning;

		public StereoFilter StereoFilter;
		public Int32 StereoDelay;
		public Int32 StereoPanningAngle;
		public Int32 StereoCombFilterDelay;
		public Int32 StereoCombFilterStrength;
	}

	public enum StereoFilter
	{
		None = 0,
		Delay = 1,
		Panning = 2,
		CombFilter = 3,
	}

	public enum VsDualOutputOption
	{
		Both = 0,
		MasterOnly = 1,
		SlaveOnly = 2
	}
}
