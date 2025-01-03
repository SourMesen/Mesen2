using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class IntegrationConfig : BaseConfig<IntegrationConfig>
	{
		[Reactive] public bool AutoLoadDbgFiles { get; set; } = true;
		[Reactive] public bool AutoLoadMlbFiles { get; set; } = true;
		[Reactive] public bool AutoLoadCdlFiles { get; set; } = false;
		[Reactive] public bool AutoLoadSymFiles { get; set; } = true;
		[Reactive] public bool AutoLoadCdbFiles { get; set; } = true;
		[Reactive] public bool AutoLoadElfFiles { get; set; } = true;
		[Reactive] public bool AutoLoadFnsFiles { get; set; } = true;

		[Reactive] public bool ResetLabelsOnImport { get; set; } = true;

		[Reactive] public bool ImportPrgRomLabels { get; set; } = true;
		[Reactive] public bool ImportWorkRamLabels { get; set; } = true;
		[Reactive] public bool ImportSaveRamLabels { get; set; } = true;
		[Reactive] public bool ImportOtherLabels { get; set; } = true;
		[Reactive] public bool ImportComments { get; set; } = true;
		
		[Reactive] public int TabSize { get; set; } = 4;

		public bool IsMemoryTypeImportEnabled(MemoryType memType)
		{
			switch(memType) {
				case MemoryType.SnesMemory:
				case MemoryType.SpcMemory:
				case MemoryType.Sa1Memory:
				case MemoryType.NecDspMemory:
				case MemoryType.GsuMemory:
				case MemoryType.Cx4Memory:
				case MemoryType.GameboyMemory:
				case MemoryType.NesMemory:
				case MemoryType.NesPpuMemory:
				case MemoryType.PceMemory:
				case MemoryType.SmsMemory:
				case MemoryType.GbaMemory:
				case MemoryType.WsMemory:
				case MemoryType.SnesVideoRam:
				case MemoryType.SnesSpriteRam:
				case MemoryType.SnesCgRam:
				case MemoryType.SnesRegister:
				case MemoryType.PceVideoRam:
				case MemoryType.PceVideoRamVdc2:
				case MemoryType.PceSpriteRam:
				case MemoryType.PceSpriteRamVdc2:
				case MemoryType.PcePaletteRam:
				case MemoryType.GbVideoRam:
				case MemoryType.GbSpriteRam:
				case MemoryType.NesNametableRam:
				case MemoryType.NesMapperRam:
				case MemoryType.NesSpriteRam:
				case MemoryType.NesSecondarySpriteRam:
				case MemoryType.NesPaletteRam:
				case MemoryType.NesChrRam:
				case MemoryType.NesChrRom:
				case MemoryType.GbaVideoRam:
				case MemoryType.GbaSpriteRam:
				case MemoryType.GbaPaletteRam:
				case MemoryType.WsBootRom:
				case MemoryType.WsPort:
					return ImportOtherLabels;

				case MemoryType.SnesPrgRom:
				case MemoryType.GbPrgRom:
				case MemoryType.NesPrgRom:
				case MemoryType.PcePrgRom:
				case MemoryType.SmsPrgRom:
				case MemoryType.SpcRom:
				case MemoryType.SufamiTurboFirmware:
				case MemoryType.SufamiTurboSecondCart:
				case MemoryType.DspProgramRom:
				case MemoryType.DspDataRom:
				case MemoryType.GbBootRom:
				case MemoryType.GbaPrgRom:
				case MemoryType.GbaBootRom:
				case MemoryType.WsPrgRom:
					return ImportPrgRomLabels;
				
				case MemoryType.SnesWorkRam:
				case MemoryType.GbWorkRam:
				case MemoryType.PceWorkRam:
				case MemoryType.PceCdromRam:
				case MemoryType.PceCardRam:
				case MemoryType.PceAdpcmRam:
				case MemoryType.PceArcadeCardRam: 
				case MemoryType.SpcRam:
				case MemoryType.DspDataRam:
				case MemoryType.Sa1InternalRam:
				case MemoryType.GsuWorkRam:
				case MemoryType.Cx4DataRam:
				case MemoryType.GbHighRam:
				case MemoryType.NesInternalRam:
				case MemoryType.NesWorkRam:
				case MemoryType.SmsWorkRam:
				case MemoryType.BsxPsRam:
				case MemoryType.GbaIntWorkRam:
				case MemoryType.GbaExtWorkRam:
				case MemoryType.WsWorkRam:
					return ImportWorkRamLabels;

				case MemoryType.SnesSaveRam:
				case MemoryType.BsxMemoryPack:
				case MemoryType.SufamiTurboSecondCartRam:
				case MemoryType.NesSaveRam:
				case MemoryType.PceSaveRam:
				case MemoryType.GbCartRam:
				case MemoryType.SmsCartRam:
				case MemoryType.GbaSaveRam:
				case MemoryType.WsCartRam:
					return ImportSaveRamLabels;

				default:
				case MemoryType.None:
					System.Diagnostics.Debug.Fail("Missing memory type case");
					return true;
			}
		}
	}
}
