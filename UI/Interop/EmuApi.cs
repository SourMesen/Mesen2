using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using Mesen.Localization;
using Mesen.Config.Shortcuts;
using Mesen.Utilities;
using Avalonia.Media.Imaging;

namespace Mesen.Interop
{
	public class EmuApi
	{
		public const string DllName = "MesenCore.dll";
		private const string DllPath = EmuApi.DllName;

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool TestDll();
		[DllImport(DllPath)] public static extern void InitDll();

		[DllImport(DllPath, EntryPoint = "GetMesenVersion")] private static extern UInt32 GetMesenVersionWrapper();
		public static Version GetMesenVersion()
		{
			UInt32 version = GetMesenVersionWrapper();
			UInt32 revision = version & 0xFF;
			UInt32 minor = (version >> 8) & 0xFF;
			UInt32 major = (version >> 16) & 0xFFFF;
			return new Version((int)major, (int)minor, (int)revision);
		}

		[DllImport(DllPath, EntryPoint = "GetMesenBuildDate")] private static extern IntPtr GetMesenBuildDateWrapper();
		public static string GetMesenBuildDate()
		{
			return Utf8Utilities.PtrToStringUtf8(GetMesenBuildDateWrapper());
		}

		[DllImport(DllPath)] public static extern IntPtr RegisterNotificationCallback(NotificationListener.NotificationCallback callback);
		[DllImport(DllPath)] public static extern void UnregisterNotificationCallback(IntPtr notificationListener);

		[DllImport(DllPath)] public static extern void InitializeEmu([MarshalAs(UnmanagedType.LPUTF8Str)]string homeFolder, IntPtr windowHandle, IntPtr dxViewerHandle, [MarshalAs(UnmanagedType.I1)] bool useSoftwareRenderer, [MarshalAs(UnmanagedType.I1)]bool noAudio, [MarshalAs(UnmanagedType.I1)]bool noVideo, [MarshalAs(UnmanagedType.I1)]bool noInput);

		[DllImport(DllPath)] public static extern void Release();

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsRunning();
		[DllImport(DllPath)] public static extern void Stop();
		[DllImport(DllPath)] public static extern Int32 GetStopCode();

		[DllImport(DllPath)] public static extern void Pause();
		[DllImport(DllPath)] public static extern void Resume();
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsPaused();

		[DllImport(DllPath)] public static extern void TakeScreenshot();

		[DllImport(DllPath)] public static extern void ProcessAudioPlayerAction(AudioPlayerActionParams p);

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool LoadRom(
			[MarshalAs(UnmanagedType.LPUTF8Str)]string filepath,
			[MarshalAs(UnmanagedType.LPUTF8Str)]string? patchFile = null
		);

		[DllImport(DllPath, EntryPoint = "GetRomInfo")] private static extern void GetRomInfoWrapper(out InteropRomInfo romInfo);
		public static RomInfo GetRomInfo()
		{
			InteropRomInfo info;
			EmuApi.GetRomInfoWrapper(out info);
			return new RomInfo(info);
		}

		[DllImport(DllPath)] public static extern void LoadRecentGame([MarshalAs(UnmanagedType.LPUTF8Str)]string filepath, [MarshalAs(UnmanagedType.I1)]bool resetGame);

		[DllImport(DllPath)] public static extern void AddKnownGameFolder([MarshalAs(UnmanagedType.LPUTF8Str)]string folder);

		[DllImport(DllPath)] public static extern void SetExclusiveFullscreenMode([MarshalAs(UnmanagedType.I1)]bool fullscreen, IntPtr windowHandle);

		[DllImport(DllPath)] public static extern TimingInfo GetTimingInfo(CpuType cpuType);

		[DllImport(DllPath)] public static extern double GetAspectRatio();
		[DllImport(DllPath)] public static extern FrameInfo GetBaseScreenSize();
		[DllImport(DllPath)] public static extern Int32 GetGameMemorySize(MemoryType type);

		[DllImport(DllPath)] public static extern void SetRendererSize(UInt32 width, UInt32 height);

		[DllImport(DllPath)] public static extern void ExecuteShortcut(ExecuteShortcutParams p);
		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool IsShortcutAllowed(EmulatorShortcut shortcut, UInt32 shortcutParam = 0);

		[DllImport(DllPath, EntryPoint = "GetLog")] private static extern void GetLogWrapper(IntPtr outLog, Int32 maxLength);
		public static string GetLog() { return Utf8Utilities.CallStringApi(GetLogWrapper, 100000); }

		[DllImport(DllPath)] public static extern void WriteLogEntry([MarshalAs(UnmanagedType.LPUTF8Str)]string message);
		[DllImport(DllPath)] public static extern void DisplayMessage([MarshalAs(UnmanagedType.LPUTF8Str)]string title, [MarshalAs(UnmanagedType.LPUTF8Str)]string message, [MarshalAs(UnmanagedType.LPUTF8Str)]string? param1 = null);

		[DllImport(DllPath, EntryPoint = "GetRomHash")] private static extern void GetRomHashWrapper(HashType hashType, IntPtr outLog, Int32 maxLength);
		public static string GetRomHash(HashType hashType) { return Utf8Utilities.CallStringApi((IntPtr outLog, Int32 maxLength) => {
			GetRomHashWrapper(hashType, outLog, maxLength);
		}, 1000000); }

		[DllImport(DllPath)] public static extern IntPtr GetArchiveRomList([MarshalAs(UnmanagedType.LPUTF8Str)]string filename, IntPtr outFileList, Int32 maxLength);

		[DllImport(DllPath)] public static extern void SaveState(UInt32 stateIndex);
		[DllImport(DllPath)] public static extern void LoadState(UInt32 stateIndex);
		[DllImport(DllPath)] public static extern void SaveStateFile([MarshalAs(UnmanagedType.LPUTF8Str)]string filepath);
		[DllImport(DllPath)] public static extern void LoadStateFile([MarshalAs(UnmanagedType.LPUTF8Str)]string filepath);

		[DllImport(DllPath, EntryPoint = "GetSaveStatePreview")] private static extern Int32 GetSaveStatePreviewWrapper([MarshalAs(UnmanagedType.LPUTF8Str)]string saveStatePath, [Out]byte[] imgData);
		public static Bitmap? GetSaveStatePreview(string saveStatePath)
		{
			if(File.Exists(saveStatePath)) {
				byte[] buffer = new byte[512*478*4];
				Int32 size = EmuApi.GetSaveStatePreviewWrapper(saveStatePath, buffer);
				if(size > 0) {
					Array.Resize(ref buffer, size);
					using(MemoryStream stream = new MemoryStream(buffer)) {
						return new Bitmap(stream);
					}
				}
			}
			return null;
		}

		[DllImport(DllPath)] [return: MarshalAs(UnmanagedType.I1)] public static extern bool GetConvertedCheat([In]InteropCheatCode input, ref InteropInternalCheatCode output);
		[DllImport(DllPath)] public static extern void SetCheats([In]InteropCheatCode[] cheats, UInt32 cheatCount);
		[DllImport(DllPath)] public static extern void ClearCheats();

		[DllImport(DllPath)] public static extern void InputBarcode(UInt64 barcode, UInt32 digitCount);
		[DllImport(DllPath)] public static extern void ProcessTapeRecorderAction(TapeRecorderAction action, [MarshalAs(UnmanagedType.LPUTF8Str)] string filename = "");
	}

	public struct TimingInfo
	{
		public double Fps;
		public UInt64 MasterClock;
		public UInt32 MasterClockRate;
		public UInt32 FrameCount;

		public UInt32 ScanlineCount;
		public Int32 FirstScanline;
		public UInt32 CycleCount;
	}

	public struct FrameInfo
	{
		public UInt32 Width;
		public UInt32 Height;
	}

	public enum RomFormat
	{
		Unknown,

		Sfc,
		Spc,

		Gb,
		Gbs,

		iNes,
		Unif,
		Fds,
		VsSystem,
		VsDualSystem,
		Nsf,
		StudyBox,

		Pce,
		PceCdRom,
		PceHes,

		Sms,
		GameGear,
		Sg,
		ColecoVision,

		Gba,

		Ws
	}

	public enum ConsoleType
	{
		Snes = 0,
		Gameboy = 1,
		Nes = 2,
		PcEngine = 3,
		Sms = 4,
		Gba = 5,
		Ws = 6,
	}

	public struct InteropDipSwitchInfo
	{
		public UInt32 DatabaseId;
		public UInt32 DipSwitchCount;
	}

	public struct InteropRomInfo
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2000)]
		public byte[] RomPath;
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 2000)]
		public byte[] PatchPath;

		public RomFormat Format;
		public ConsoleType ConsoleType;
		public InteropDipSwitchInfo DipSwitches;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5)]
		public CpuType[] CpuTypes;
		public UInt32 CpuTypeCount;
	}

	public class RomInfo
	{
		public string RomPath = "";
		public string PatchPath = "";
		public RomFormat Format = RomFormat.Unknown;
		public ConsoleType ConsoleType = ConsoleType.Snes;
		public InteropDipSwitchInfo DipSwitches;
		public HashSet<CpuType> CpuTypes = new HashSet<CpuType>();

		public RomInfo() { }

		public RomInfo(InteropRomInfo romInfo)
		{
			RomPath = (ResourcePath)Utf8Utilities.GetStringFromArray(romInfo.RomPath);
			PatchPath = (ResourcePath)Utf8Utilities.GetStringFromArray(romInfo.PatchPath);
			Format = romInfo.Format;
			ConsoleType = romInfo.ConsoleType;
			DipSwitches = romInfo.DipSwitches;

			for(int i = 0; i < romInfo.CpuTypeCount; i++) {
				CpuTypes.Add(romInfo.CpuTypes[i]);
			}
		}

		public string GetRomName()
		{
			return Path.GetFileNameWithoutExtension(((ResourcePath)RomPath).FileName);
		}
	}

	public enum FirmwareType
	{
		DSP1,
		DSP1B,
		DSP2,
		DSP3,
		DSP4,
		ST010,
		ST011,
		ST018,
		Satellaview,
		SufamiTurbo,
		Gameboy,
		GameboyColor,
		GameboyAdvance,
		Sgb1GameboyCpu,
		Sgb2GameboyCpu,
		SGB1,
		SGB2,
		FDS,
		StudyBox,
		PceSuperCd,
		PceGamesExpress,
		ColecoVision,
		WonderSwan,
		WonderSwanColor,
		SwanCrystal,
		Ymf288AdpcmRom,
		SmsBootRom,
		GgBootRom
	}

	public struct MissingFirmwareMessage
	{
		public IntPtr Filename;
		public FirmwareType Firmware;
		public UInt32 Size;
		public UInt32 AltSize;
	}

	public struct SufamiTurboFilePromptMessage
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 5000)]
		public byte[] Filename;
	}

	public struct ExecuteShortcutParams
	{
		public EmulatorShortcut Shortcut;
		public UInt32 Param;
		public IntPtr ParamPtr;
	};

	public enum AudioPlayerAction
	{
		NextTrack,
		PrevTrack,
		SelectTrack,
	}

	public struct AudioPlayerActionParams
	{
		public AudioPlayerAction Action;
		public UInt32 TrackNumber;
	}

	public enum TapeRecorderAction
	{
		Play,
		StartRecord,
		StopRecord
	}

	public enum CheatType : byte
	{
		NesGameGenie = 0,
		NesProActionRocky,
		NesCustom,
		GbGameGenie,
		GbGameShark,
		SnesGameGenie,
		SnesProActionReplay,
		PceRaw,
		PceAddress,
		SmsProActionReplay,
		SmsGameGenie
	}

	public static class CheatTypeExtensions
	{
		public static CpuType ToCpuType(this CheatType cheatType)
		{
			return cheatType switch {
				CheatType.NesGameGenie or CheatType.NesProActionRocky or CheatType.NesCustom => CpuType.Nes,
				CheatType.SnesGameGenie or CheatType.SnesProActionReplay => CpuType.Snes,
				CheatType.GbGameGenie or CheatType.GbGameShark => CpuType.Gameboy,
				CheatType.PceRaw or CheatType.PceAddress => CpuType.Pce,
				CheatType.SmsGameGenie or CheatType.SmsProActionReplay => CpuType.Sms,
				_ => throw new NotImplementedException("unsupported cheat type")
			};
		}
	}

	public struct InteropCheatCode
	{
		public CheatType Type;

		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 16)]
		public byte[] Code;

		public InteropCheatCode(CheatType type, string code)
		{
			Type = type;

			Code = new byte[16];
			byte[] codeBytes = Encoding.UTF8.GetBytes(code);
			Array.Copy(codeBytes, Code, Math.Min(codeBytes.Length, 15));
		}
	}

	public struct InteropInternalCheatCode
	{
		public MemoryType MemType;
		public UInt32 Address;
		public Int16 Compare;
		public byte Value;
		public CheatType Type;
		public CpuType Cpu;
		[MarshalAs(UnmanagedType.I1)] public bool IsRamCode;
		[MarshalAs(UnmanagedType.I1)] public bool IsAbsoluteAddress;
	}

	public enum HashType
	{
		Sha1,
		Sha1Cheat
	}

	public struct SoftwareRendererSurface
	{
		public IntPtr FrameBuffer;
		public UInt32 Width;
		public UInt32 Height;
		[MarshalAs(UnmanagedType.I1)] public bool IsDirty;
	}

	public struct SoftwareRendererFrame
	{
		public SoftwareRendererSurface Frame;
		public SoftwareRendererSurface EmuHud;
		public SoftwareRendererSurface ScriptHud;
	}
}
