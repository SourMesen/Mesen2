using Avalonia.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.ViewModels;

public class NesHeaderEditViewModel : DisposableViewModel
{
	public NesHeader Header { get; }

	[Reactive] public bool IsBatteryCheckboxEnabled { get; private set; }
	[Reactive] public bool IsVsSystemVisible { get; private set; }
	[Reactive] public bool IsNes20 { get; private set; }
	
	[Reactive] public Enum[]? AvailableSystemTypes { get; private set; } = null;
	[Reactive] public Enum[]? AvailableTimings { get; private set; } = null;

	[Reactive] public string HeaderBytes { get; private set; } = "";
	[Reactive] public string ErrorMessage { get; private set; } = "";

	private RomInfo _romInfo;

	public NesHeaderEditViewModel()
	{
		bool releaseDebugger = !DebugWindowManager.HasOpenedDebugWindows();
		byte[] headerBytes = DebugApi.GetRomHeader();
		if(releaseDebugger) {
			//GetRomHeader will initialize the debugger - stop the debugger if no other debug window is opened
			DebugApi.ReleaseDebugger();
		}

		if(headerBytes.Length < 16) {
			Array.Resize(ref headerBytes, 16);
		}

		_romInfo = EmuApi.GetRomInfo();
		Header = NesHeader.FromBytes(headerBytes);

		AddDisposable(this.WhenAnyValue(x => x.Header.SaveRam, x => x.Header.ChrRamBattery).Subscribe(x => {
			IsBatteryCheckboxEnabled = Header.SaveRam == MemorySizes.None  && Header.ChrRamBattery == MemorySizes.None;
			if(!IsBatteryCheckboxEnabled) {
				Header.HasBattery = true;
			}
		}));

		AddDisposable(this.WhenAnyValue(x => x.Header.System, x => x.Header.FileType).Subscribe(x => {
			bool isVsSystem = Header.System == TvSystem.VsSystem;
			IsVsSystemVisible = isVsSystem && Header.FileType == NesFileType.Nes2_0;
			if(!IsVsSystemVisible) {
				Header.VsPpu = VsPpuType.RP2C03B;
				Header.VsSystem = VsSystemType.Default;
			}
		}));

		AddDisposable(this.WhenAnyValue(x => x.Header.FileType).Subscribe(x => {
			IsNes20 = Header.FileType == NesFileType.Nes2_0;

			if(IsNes20) {
				AvailableSystemTypes = null;
				AvailableTimings = null;
			} else {
				AvailableSystemTypes = new Enum[] { TvSystem.NesFamicomDendy, TvSystem.VsSystem, TvSystem.Playchoice };
				AvailableTimings = new Enum[] { FrameTiming.Ntsc, FrameTiming.Pal, FrameTiming.NtscAndPal };

				Header.SubmapperId = 0;
				Header.ChrRam = MemorySizes.None;
				Header.ChrRamBattery = MemorySizes.None;
				Header.SaveRam = MemorySizes.None;
				Header.WorkRam = MemorySizes.None;
			}
		}));

		Header.PropertyChanged += Header_PropertyChanged;
		UpdateHeaderPreview();
	}

	protected override void DisposeView()
	{
		base.DisposeView();
		Header.PropertyChanged -= Header_PropertyChanged;
	}

	private void Header_PropertyChanged(object? sender, System.ComponentModel.PropertyChangedEventArgs e)
	{
		UpdateHeaderPreview();
		ErrorMessage = GetErrorMessage();
	}

	private void UpdateHeaderPreview()
	{
		byte[] headerBytes = Header.ToBytes();
		StringBuilder sb = new();
		for(int i = 0; i < 16; i++) {
			sb.Append(headerBytes[i].ToString("X2") + " ");
		}
		HeaderBytes = sb.ToString().Trim();
	}

	private string GetErrorMessage()
	{
		bool isValidPrgSize = Header.FileType == NesFileType.Nes2_0 && NesHeader.IsValidSize(Header.PrgRom);
		bool isValidChrSize = Header.FileType == NesFileType.Nes2_0 && NesHeader.IsValidSize(Header.ChrRom);
		if(!isValidPrgSize && (Header.PrgRom % 16) != 0) {
			return "Error: PRG ROM size must be a multiple of 16 KB";
		} if(!isValidChrSize && (Header.ChrRom % 8) != 0) {
			return "Error: CHR ROM size must be a multiple of 8 KB";
		}

		if(Header.FileType == NesFileType.Nes2_0) {
			if(Header.MapperId >= 4096) {
				return "Error: Mapper ID must be lower than 4096";
			}
			if(Header.SubmapperId >= 16) {
				return "Error: Submapper ID must be lower than 16";
			}
			if(!isValidChrSize && Header.ChrRom >= 16384) {
				return "Error: CHR ROM size must be lower than 16384 KB";
			}
			if(!isValidPrgSize && Header.PrgRom >= 32768) {
				return "Error: PRG ROM size must be lower than 32768 KB";
			}
		} else {
			if(Header.MapperId >= 256) {
				return "Error: Mapper ID must be lower than 256 ";
			}
			if(Header.ChrRom >= 2048) {
				return "Error: CHR ROM size must be lower than 2048 KB";
			}
			if(Header.PrgRom >= 4096) {
				return "Error: PRG ROM size must be lower than 4096 KB";
			}
		}

		return "";
	}

	public async Task<bool> Save(Window wnd)
	{
		string? filepath = await FileDialogHelper.SaveFile(Path.GetDirectoryName(_romInfo.RomPath), Path.GetFileName(_romInfo.RomPath), wnd, FileDialogHelper.NesExt);
		if(filepath != null) {
			byte[]? data = FileHelper.ReadAllBytes(_romInfo.RomPath);
			if(data != null) {
				byte[] header = Header.ToBytes();
				for(int i = 0; i < 16; i++) {
					data[i] = header[i];
				}
				return FileHelper.WriteAllBytes(filepath, data);
			}
		}
		return false;
	}

	public class NesHeader : ViewModelBase
	{
		private static Dictionary<UInt64, int> _validSizeValues = new Dictionary<UInt64, int>();

		[Reactive] public NesFileType FileType { get; set; }

		[Reactive] public uint MapperId { get; set; }
		[Reactive] public uint SubmapperId { get; set; }

		[Reactive] public UInt64 PrgRom { get; set; }
		[Reactive] public UInt64 ChrRom { get; set; }

		[Reactive] public iNesMirroringType Mirroring { get; set; }

		[Reactive] public FrameTiming Timing { get; set; }
		[Reactive] public TvSystem System { get; set; }
		[Reactive] public bool HasTrainer { get; set; }
		[Reactive] public bool HasBattery { get; set; }
		[Reactive] public VsPpuType VsPpu { get; set; }
		[Reactive] public VsSystemType VsSystem { get; set; }
		[Reactive] public GameInputType InputType { get; set; }

		[Reactive] public MemorySizes WorkRam { get; set; } = MemorySizes.None;
		[Reactive] public MemorySizes SaveRam { get; set; } = MemorySizes.None;
		[Reactive] public MemorySizes ChrRam { get; set; } = MemorySizes.None;
		[Reactive] public MemorySizes ChrRamBattery { get; set; } = MemorySizes.None;

		static NesHeader()
		{
			_validSizeValues = new Dictionary<UInt64, int>();
			for(int i = 0; i < 256; i++) {
				int multiplier = (i & 0x03) * 2 + 1;
				UInt64 value = ((UInt64)1 << (i >> 2)) / 1024;
				_validSizeValues[(UInt64)multiplier * value] = i;
			}
		}

		public byte[] ToBytes()
		{
			byte[] header = new byte[16];
			header[0] = 0x4E;
			header[1] = 0x45;
			header[2] = 0x53;
			header[3] = 0x1A;

			UInt64 prgRomValue = PrgRom / 16;
			UInt64 chrRomValue = ChrRom / 8;

			if(FileType == NesFileType.Nes2_0) {
				if((PrgRom % 16) != 0 || PrgRom >= 32768) {
					if(_validSizeValues.ContainsKey(PrgRom)) {
						//This value is a valid exponent+multiplier combo (NES 2.0 only)
						prgRomValue = ((uint)_validSizeValues[PrgRom] & 0xFF) | 0xF00;
					}
				}

				if((ChrRom % 8) != 0 || ChrRom >= 16384) {
					if(_validSizeValues.ContainsKey(ChrRom)) {
						//This value is a valid exponent+multiplier combo (NES 2.0 only)
						chrRomValue = ((uint)_validSizeValues[ChrRom] & 0xFF) | 0xF00;
					}
				}

				//NES 2.0
				header[4] = (byte)(prgRomValue);
				header[5] = (byte)(chrRomValue);

				header[6] = (byte)(
					((byte)(MapperId & 0x0F) << 4) |
					(byte)Mirroring | (HasTrainer ? 0x04 : 0x00) | (HasBattery ? 0x02 : 0x00)
				);

				header[7] = (byte)(MapperId & 0xF0);

				switch(System) {
					case TvSystem.NesFamicomDendy: header[7] |= 0x00; break;
					case TvSystem.VsSystem: header[7] |= 0x01; break;
					case TvSystem.Playchoice: header[7] |= 0x02; break;
					default: header[7] |= 0x03; break;
				}

				//Enable NES 2.0 header
				header[7] |= 0x08;

				header[8] = (byte)(((SubmapperId & 0x0F) << 4) | ((MapperId & 0xF00) >> 8));
				header[9] = (byte)(((prgRomValue & 0xF00) >> 8) | ((chrRomValue & 0xF00) >> 4));

				header[10] = (byte)((byte)WorkRam | ((byte)SaveRam) << 4);
				header[11] = (byte)((byte)ChrRam | ((byte)ChrRamBattery) << 4);

				header[12] = (byte)Timing;

				if(System == TvSystem.VsSystem) {
					header[13] = (byte)(((byte)VsPpu & 0x0F) | ((((byte)VsSystem) & 0x0F) << 4));
				} else {
					header[13] = (byte)System;
				}
				header[14] = 0;
				header[15] = (byte)InputType;
			} else {
				//iNES
				if(prgRomValue == 0x100) {
					header[4] = 0;
				} else {
					header[4] = (byte)(prgRomValue);
				}
				header[5] = (byte)(chrRomValue);

				header[6] = (byte)(
					((byte)(MapperId & 0x0F) << 4) |
					(byte)Mirroring | (HasTrainer ? 0x04 : 0x00) | (HasBattery ? 0x02 : 0x00)
				);
				header[7] = (byte)(
					((byte)MapperId & 0xF0) |
					(byte)(System == TvSystem.VsSystem ? 0x01 : 0x00) | (System == TvSystem.Playchoice ? 0x02 : 0x00)
				);

				header[8] = 0;
				header[9] = (byte)(Timing == FrameTiming.Pal ? 0x01 : 0x00);
				header[10] = 0;
				header[11] = 0;
				header[12] = 0;
				header[13] = 0;
				header[14] = 0;
				header[15] = 0;
			}

			return header;
		}

		public static NesHeader FromBytes(byte[] bytes)
		{
			BinaryHeader binHeader = new BinaryHeader(bytes);

			NesHeader header = new NesHeader();
			header.FileType = binHeader.GetRomHeaderVersion() == RomHeaderVersion.Nes2_0 ? NesFileType.Nes2_0 : NesFileType.iNes;
			header.PrgRom = (uint)(binHeader.GetPrgSize());
			header.ChrRom = (uint)(binHeader.GetChrSize());
			header.HasTrainer = binHeader.HasTrainer();
			header.HasBattery = binHeader.HasBattery();

			header.System = binHeader.GetTvSystem();
			header.Timing = binHeader.GetFrameTiming();

			header.Mirroring = binHeader.GetMirroringType();
			header.MapperId = (uint)binHeader.GetMapperID();
			header.SubmapperId = (uint)binHeader.GetSubMapper();
			header.WorkRam = (MemorySizes)binHeader.GetWorkRamSize();
			header.SaveRam = (MemorySizes)binHeader.GetSaveRamSize();
			header.ChrRam = (MemorySizes)binHeader.GetChrRamSize();
			header.ChrRamBattery = (MemorySizes)binHeader.GetSaveChrRamSize();
			header.InputType = binHeader.GetInputType();
			header.VsPpu = (VsPpuType)(bytes[13] & 0x0F);
			header.VsSystem = binHeader.GetVsSystemType();

			return header;
		}

		public static bool IsValidSize(ulong size)
		{
			return _validSizeValues.ContainsKey(size);
		}
	}

	private class BinaryHeader
	{
		private byte[] _bytes;
		private byte PrgCount;
		private byte ChrCount;

		public BinaryHeader(byte[] bytes)
		{
			_bytes = bytes;
			PrgCount = bytes[4];
			ChrCount = bytes[5];
		}

		public RomHeaderVersion GetRomHeaderVersion()
		{
			if((_bytes[7] & 0x0C) == 0x08) {
				return RomHeaderVersion.Nes2_0;
			} else if((_bytes[7] & 0x0C) == 0x00) {
				return RomHeaderVersion.iNes;
			} else {
				return RomHeaderVersion.OldiNes;
			}
		}

		public int GetMapperID()
		{
			switch(GetRomHeaderVersion()) {
				case RomHeaderVersion.Nes2_0:
					return ((_bytes[8] & 0x0F) << 8) | (_bytes[7] & 0xF0) | (_bytes[6] >> 4);
				default:
				case RomHeaderVersion.iNes:
					return (_bytes[7] & 0xF0) | (_bytes[6] >> 4);
				case RomHeaderVersion.OldiNes:
					return (_bytes[6] >> 4);
			}
		}

		public bool HasBattery()
		{
			return (_bytes[6] & 0x02) == 0x02;
		}

		public bool HasTrainer()
		{
			return (_bytes[6] & 0x04) == 0x04;
		}

		public FrameTiming GetFrameTiming()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				return (FrameTiming)(_bytes[12] & 0x03);
			} else if(GetRomHeaderVersion() == RomHeaderVersion.iNes) {
				return (_bytes[9] & 0x01) == 0x01 ? FrameTiming.Pal : FrameTiming.Ntsc;
			}
			return FrameTiming.Ntsc;
		}

		public TvSystem GetTvSystem()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				switch(_bytes[7] & 0x03) {
					case 0: return TvSystem.NesFamicomDendy;
					case 1: return TvSystem.VsSystem;
					case 2: return TvSystem.Playchoice;
					case 3: return (TvSystem)_bytes[13];
				}
			} else if(GetRomHeaderVersion() == RomHeaderVersion.iNes) {
				if((_bytes[7] & 0x01) == 0x01) {
					return TvSystem.VsSystem;
				} else if((_bytes[7] & 0x02) == 0x02) {
					return TvSystem.Playchoice;
				}
			}
			return TvSystem.NesFamicomDendy;
		}

		private UInt64 GetSizeValue(int exponent, int multiplier)
		{
			multiplier = multiplier * 2 + 1;
			return (UInt64)multiplier * (((UInt64)1 << exponent) / 1024);
		}

		public UInt64 GetPrgSize()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				if((_bytes[9] & 0x0F) == 0x0F) {
					return GetSizeValue(PrgCount >> 2, PrgCount & 0x03);
				} else {
					return (UInt64)(((_bytes[9] & 0x0F) << 8) | PrgCount) * 16;
				}
			} else {
				if(PrgCount == 0) {
					return 256 * 16; //0 is a special value and means 256
				} else {
					return (UInt64)PrgCount * 16;
				}
			}
		}

		public UInt64 GetChrSize()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				if((_bytes[9] & 0xF0) == 0xF0) {
					return GetSizeValue(ChrCount >> 2, ChrCount & 0x03);
				} else {
					return (UInt64)(((_bytes[9] & 0xF0) << 4) | ChrCount) * 8;
				}
			} else {
				return (UInt64)ChrCount * 8;
			}
		}

		public int GetWorkRamSize()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				return _bytes[10] & 0x0F;
			} else {
				return 0;
			}
		}

		public int GetSaveRamSize()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				return (_bytes[10] & 0xF0) >> 4;
			} else {
				return 0;
			}
		}

		public int GetChrRamSize()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				return _bytes[11] & 0x0F;
			} else {
				return 0;
			}
		}

		public int GetSaveChrRamSize()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				return (_bytes[11] & 0xF0) >> 4;
			} else {
				return 0;
			}
		}

		public int GetSubMapper()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				return (_bytes[8] & 0xF0) >> 4;
			} else {
				return 0;
			}
		}

		public iNesMirroringType GetMirroringType()
		{
			if((_bytes[6] & 0x08) != 0) {
				return iNesMirroringType.FourScreens;
			} else {
				return (_bytes[6] & 0x01) != 0 ? iNesMirroringType.Vertical : iNesMirroringType.Horizontal;
			}
		}

		public GameInputType GetInputType()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				if(_bytes[15] < Enum.GetValues<GameInputType>().Length) {
					return (GameInputType)_bytes[15];
				}
				return GameInputType.Unspecified;
			} else {
				return GameInputType.Unspecified;
			}
		}

		public VsSystemType GetVsSystemType()
		{
			if(GetRomHeaderVersion() == RomHeaderVersion.Nes2_0) {
				if((_bytes[13] >> 4) <= 0x06) {
					return (VsSystemType)(_bytes[13] >> 4);
				}
			}
			return VsSystemType.Default;
		}
	}

	public enum RomHeaderVersion
	{
		iNes = 0,
		Nes2_0 = 1,
		OldiNes = 2
	}

	public enum NesFileType
	{
		iNes = 0,
		Nes2_0 = 1
	}

	public enum iNesMirroringType
	{
		Horizontal = 0,
		Vertical = 1,
		FourScreens = 8
	}

	public enum FrameTiming
	{
		Ntsc = 0,
		Pal = 1,
		NtscAndPal = 2,
		Dendy = 3
	}

	public enum TvSystem
	{
		NesFamicomDendy,
		VsSystem,
		Playchoice,
		CloneWithDecimal,
		EpsmModule,
		Vt01RedCyan,
		Vt02,
		Vt03,
		Vt09,
		Vt32,
		Vt369,
		UmcUm6578,
		FamicomNetworkSystem,
		ReservedD,
		ReservedE,
		ReservedF
	}

	public enum MemorySizes
	{
		None = 0,
		_128Bytes = 1,
		_256Bytes = 2,
		_512Bytes = 3,
		_1KB = 4,
		_2KB = 5,
		_4KB = 6,
		_8KB = 7,
		_16KB = 8,
		_32KB = 9,
		_64KB = 10,
		_128KB = 11,
		_256KB = 12,
		_512KB = 13,
		_1024KB = 14,
		Reserved = 15
	}

	public enum VsPpuType
	{
		RP2C03B = 0,
		RP2C03G = 1,
		RP2C040001 = 2,
		RP2C040002 = 3,
		RP2C040003 = 4,
		RP2C040004 = 5,
		RC2C03B = 6,
		RC2C03C = 7,
		RC2C0501 = 8,
		RC2C0502 = 9,
		RC2C0503 = 10,
		RC2C0504 = 11,
		RC2C0505 = 12,
		ReservedD = 13,
		ReservedE = 14,
		ReservedF = 15
	}

	public enum VsSystemType
	{
		Default = 0,
		RbiBaseballProtection = 1,
		TkoBoxingProtection = 2,
		SuperXeviousProtection = 3,
		IceClimberProtection = 4,
		VsDualSystem = 5,
		RaidOnBungelingBayProtection = 6,
	}

	public enum GameInputType
	{
		Unspecified = 0,
		StandardControllers = 1,
		FourScore = 2,
		FourPlayerAdapter = 3,
		VsSystem = 4,
		VsSystemSwapped = 5,
		VsSystemSwapAB = 6,
		VsZapper = 7,
		Zapper = 8,
		TwoZappers = 9,
		BandaiHypershot = 0x0A,
		PowerPadSideA = 0x0B,
		PowerPadSideB = 0x0C,
		FamilyTrainerSideA = 0x0D,
		FamilyTrainerSideB = 0x0E,
		ArkanoidControllerNes = 0x0F,
		ArkanoidControllerFamicom = 0x10,
		DoubleArkanoidController = 0x11,
		KonamiHyperShot = 0x12,
		PachinkoController = 0x13,
		ExcitingBoxing = 0x14,
		JissenMahjong = 0x15,
		PartyTap = 0x16,
		OekaKidsTablet = 0x17,
		BarcodeBattler = 0x18,
		MiraclePiano = 0x19, //not supported yet
		PokkunMoguraa = 0x1A, //not supported yet
		TopRider = 0x1B, //not supported yet
		DoubleFisted = 0x1C, //not supported yet
		Famicom3dSystem = 0x1D, //not supported yet
		DoremikkoKeyboard = 0x1E, //not supported yet
		ROB = 0x1F, //not supported yet
		FamicomDataRecorder = 0x20,
		TurboFile = 0x21,
		BattleBox = 0x22,
		FamilyBasicKeyboard = 0x23,
		Pec586Keyboard = 0x24, //not supported yet
		Bit79Keyboard = 0x25, //not supported yet
		SuborKeyboard = 0x26,
		SuborKeyboardMouse1 = 0x27,
		SuborKeyboardMouse2 = 0x28,
		SnesMouse = 0x29,
		GenericMulticart = 0x2A, //not supported yet
		SnesControllers = 0x2B,
		RacerMateBicycle = 0x2C, //not supported yet
		UForce = 0x2D, //not supported yet
		RobStackUp = 0x2E, //not supported yet
		CityPatrolmanLightgun = 0x2F, //not supported yet
		SharpC1CassetteInterface = 0x30, //not supported yet
		StandardControllerWithSwappedButtons = 0x31, //not supported yet
		ExcaliborSudokuPad = 0x32, //not supported yet
		AblPinball = 0x33, //not supported yet
		GoldenNuggetCasino = 0x34 //not supported yet
	}
}
