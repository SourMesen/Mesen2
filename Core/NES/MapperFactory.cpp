#include "pch.h"
#include "NES/MapperFactory.h"
#include "NES/NesConsole.h"
#include "NES/Loaders/RomLoader.h"
#include "NES/Loaders/UnifBoards.h"
#include "NES/BaseMapper.h"
#include "NES/RomData.h"
#include "Utilities/VirtualFile.h"

#include "NES/Mappers/Bandai/Bandai74161_7432.h"
#include "NES/Mappers/Bandai/BandaiFcg.h"
#include "NES/Mappers/Bandai/BandaiKaraoke.h"
#include "NES/Mappers/Bandai/OekaKids.h"
#include "NES/Mappers/Codemasters/BF9096.h"
#include "NES/Mappers/Codemasters/BF909x.h"
#include "NES/Mappers/Codemasters/GoldenFive.h"
#include "NES/Mappers/FDS/Fds.h"
#include "NES/Mappers/FDS/FdsAudio.h"
#include "NES/Mappers/Homebrew/Action53.h"
#include "NES/Mappers/Homebrew/MagicFloor218.h"
#include "NES/Mappers/Homebrew/NsfCart31.h"
#include "NES/Mappers/Homebrew/Cheapocabra.h"
#include "NES/Mappers/Homebrew/FaridSlrom.h"
#include "NES/Mappers/Homebrew/FaridUnrom.h"
#include "NES/Mappers/Homebrew/Rainbow.h"
#include "NES/Mappers/Homebrew/SealieComputing.h"
#include "NES/Mappers/Homebrew/UnRom512.h"
#include "NES/Mappers/Homebrew/UnlDripGame.h"
#include "NES/Mappers/Irem/BnRom.h"
#include "NES/Mappers/Irem/IremG101.h"
#include "NES/Mappers/Irem/IremH3001.h"
#include "NES/Mappers/Irem/IremLrog017.h"
#include "NES/Mappers/Irem/IremTamS1.h"
#include "NES/Mappers/Jaleco/JalecoJf11_14.h"
#include "NES/Mappers/Jaleco/JalecoJf13.h"
#include "NES/Mappers/Jaleco/JalecoJf16.h"
#include "NES/Mappers/Jaleco/JalecoJf17_19.h"
#include "NES/Mappers/Jaleco/JalecoJfxx.h"
#include "NES/Mappers/Jaleco/JalecoSs88006.h"
#include "NES/Mappers/JyCompany/JyCompany.h"
#include "NES/Mappers/JyCompany/Mapper35.h"
#include "NES/Mappers/JyCompany/Mapper91.h"
#include "NES/Mappers/Kaiser/Kaiser202.h"
#include "NES/Mappers/Kaiser/Kaiser7012.h"
#include "NES/Mappers/Kaiser/Kaiser7013B.h"
#include "NES/Mappers/Kaiser/Kaiser7016.h"
#include "NES/Mappers/Kaiser/Kaiser7017.h"
#include "NES/Mappers/Kaiser/Kaiser7022.h"
#include "NES/Mappers/Kaiser/Kaiser7031.h"
#include "NES/Mappers/Kaiser/Kaiser7037.h"
#include "NES/Mappers/Kaiser/Kaiser7057.h"
#include "NES/Mappers/Kaiser/Kaiser7058.h"
#include "NES/Mappers/Konami/VRC1.h"
#include "NES/Mappers/Konami/VRC2_4.h"
#include "NES/Mappers/Konami/VRC3.h"
#include "NES/Mappers/Konami/VRC6.h"
#include "NES/Mappers/Konami/VRC7.h"
#include "NES/Mappers/Mmc3Variants/Bmc830118C.h"
#include "NES/Mappers/Mmc3Variants/Bmc8in1.h"
#include "NES/Mappers/Mmc3Variants/BmcGn45.h"
#include "NES/Mappers/Mmc3Variants/BmcHpxx.h"
#include "NES/Mappers/Mmc3Variants/DragonFighter.h"
#include "NES/Mappers/Mmc3Variants/MMC3_114.h"
#include "NES/Mappers/Mmc3Variants/MMC3_115.h"
#include "NES/Mappers/Mmc3Variants/MMC3_12.h"
#include "NES/Mappers/Mmc3Variants/MMC3_121.h"
#include "NES/Mappers/Mmc3Variants/MMC3_123.h"
#include "NES/Mappers/Mmc3Variants/MMC3_126.h"
#include "NES/Mappers/Mmc3Variants/MMC3_134.h"
#include "NES/Mappers/Mmc3Variants/MMC3_14.h"
#include "NES/Mappers/Mmc3Variants/MMC3_165.h"
#include "NES/Mappers/Mmc3Variants/MMC3_182.h"
#include "NES/Mappers/Mmc3Variants/MMC3_187.h"
#include "NES/Mappers/Mmc3Variants/MMC3_196.h"
#include "NES/Mappers/Mmc3Variants/MMC3_197.h"
#include "NES/Mappers/Mmc3Variants/MMC3_198.h"
#include "NES/Mappers/Mmc3Variants/MMC3_199.h"
#include "NES/Mappers/Mmc3Variants/MMC3_205.h"
#include "NES/Mappers/Mmc3Variants/MMC3_208.h"
#include "NES/Mappers/Mmc3Variants/MMC3_215.h"
#include "NES/Mappers/Mmc3Variants/MMC3_217.h"
#include "NES/Mappers/Mmc3Variants/MMC3_219.h"
#include "NES/Mappers/Mmc3Variants/MMC3_224.h"
#include "NES/Mappers/Mmc3Variants/MMC3_238.h"
#include "NES/Mappers/Mmc3Variants/MMC3_245.h"
#include "NES/Mappers/Mmc3Variants/MMC3_249.h"
#include "NES/Mappers/Mmc3Variants/MMC3_250.h"
#include "NES/Mappers/Mmc3Variants/MMC3_254.h"
#include "NES/Mappers/Mmc3Variants/MMC3_44.h"
#include "NES/Mappers/Mmc3Variants/MMC3_45.h"
#include "NES/Mappers/Mmc3Variants/MMC3_49.h"
#include "NES/Mappers/Mmc3Variants/MMC3_52.h"
#include "NES/Mappers/Mmc3Variants/MMC3_Bmc411120C.h"
#include "NES/Mappers/Mmc3Variants/MMC3_BmcF15.h"
#include "NES/Mappers/Mmc3Variants/MMC3_ChrRam.h"
#include "NES/Mappers/Mmc3Variants/MMC3_Coolboy.h"
#include "NES/Mappers/Mmc3Variants/MMC3_Kof97.h"
#include "NES/Mappers/Mmc3Variants/MMC3_MaliSB.h"
#include "NES/Mappers/Mmc3Variants/MMC3_StreetHeroes.h"
#include "NES/Mappers/Mmc3Variants/McAcc.h"
#include "NES/Mappers/Mmc3Variants/ResetTxrom.h"
#include "NES/Mappers/Mmc3Variants/Unl158B.h"
#include "NES/Mappers/Mmc3Variants/Unl8237A.h"
#include "NES/Mappers/Namco/Namco108.h"
#include "NES/Mappers/Namco/Namco108_154.h"
#include "NES/Mappers/Namco/Namco108_76.h"
#include "NES/Mappers/Namco/Namco108_88.h"
#include "NES/Mappers/Namco/Namco108_95.h"
#include "NES/Mappers/Namco/Namco163.h"
#include "NES/Mappers/Nintendo/AXROM.h"
#include "NES/Mappers/Nintendo/CNROM.h"
#include "NES/Mappers/Nintendo/CpRom.h"
#include "NES/Mappers/Nintendo/FamicomBox.h"
#include "NES/Mappers/Nintendo/FnsMmc1.h"
#include "NES/Mappers/Nintendo/GxRom.h"
#include "NES/Mappers/Nintendo/MMC1.h"
#include "NES/Mappers/Nintendo/MMC1_105.h"
#include "NES/Mappers/Nintendo/MMC1_155.h"
#include "NES/Mappers/Nintendo/MMC2.h"
#include "NES/Mappers/Nintendo/MMC3.h"
#include "NES/Mappers/Nintendo/MMC3_37.h"
#include "NES/Mappers/Nintendo/MMC3_47.h"
#include "NES/Mappers/Nintendo/MMC4.h"
#include "NES/Mappers/Nintendo/MMC5.h"
#include "NES/Mappers/Nintendo/NROM.h"
#include "NES/Mappers/Nintendo/TxSRom.h"
#include "NES/Mappers/Nintendo/UNROM.h"
#include "NES/Mappers/Nintendo/UnRom_180.h"
#include "NES/Mappers/Nintendo/UnRom_94.h"
#include "NES/Mappers/NSF/NsfMapper.h"
#include "NES/Mappers/Ntdec/Caltron41.h"
#include "NES/Mappers/Ntdec/Bmc63.h"
#include "NES/Mappers/Ntdec/BmcNtd03.h"
#include "NES/Mappers/Ntdec/Mapper112.h"
#include "NES/Mappers/Ntdec/Mapper174.h"
#include "NES/Mappers/Ntdec/Mapper221.h"
#include "NES/Mappers/Ntdec/NtdecTc112.h"
#include "NES/Mappers/Ntdec/Tf1201.h"
#include "NES/Mappers/Sachen/Sachen74LS374N.h"
#include "NES/Mappers/Sachen/Sachen8259.h"
#include "NES/Mappers/Sachen/Sachen9602.h"
#include "NES/Mappers/Sachen/Sachen_133.h"
#include "NES/Mappers/Sachen/Sachen_136.h"
#include "NES/Mappers/Sachen/Sachen_143.h"
#include "NES/Mappers/Sachen/Sachen_145.h"
#include "NES/Mappers/Sachen/Sachen_147.h"
#include "NES/Mappers/Sachen/Sachen_148.h"
#include "NES/Mappers/Sachen/Sachen_149.h"
#include "NES/Mappers/StudyBox.h"
#include "NES/Mappers/Sunsoft/Sunsoft184.h"
#include "NES/Mappers/Sunsoft/Sunsoft3.h"
#include "NES/Mappers/Sunsoft/Sunsoft4.h"
#include "NES/Mappers/Sunsoft/Sunsoft89.h"
#include "NES/Mappers/Sunsoft/Sunsoft93.h"
#include "NES/Mappers/Sunsoft/SunsoftFme7.h"
#include "NES/Mappers/Taito/TaitoTc0190.h"
#include "NES/Mappers/Taito/TaitoTc0690.h"
#include "NES/Mappers/Taito/TaitoX1005.h"
#include "NES/Mappers/Taito/TaitoX1017.h"
#include "NES/Mappers/Tengen/Rambo1.h"
#include "NES/Mappers/Tengen/Rambo1_158.h"
#include "NES/Mappers/Txc/Bmc11160.h"
#include "NES/Mappers/Txc/MMC3_189.h"
#include "NES/Mappers/Txc/Mapper61.h"
#include "NES/Mappers/Txc/Txc22000.h"
#include "NES/Mappers/Txc/Txc22211A.h"
#include "NES/Mappers/Txc/Txc22211B.h"
#include "NES/Mappers/Txc/Txc22211C.h"
#include "NES/Mappers/Unlicensed/A65AS.h"
#include "NES/Mappers/Unlicensed/Ac08.h"
#include "NES/Mappers/Unlicensed/ActionEnterprises.h"
#include "NES/Mappers/Unlicensed/Ax5705.h"
#include "NES/Mappers/Unlicensed/Bb.h"
#include "NES/Mappers/Unlicensed/Bmc12in1.h"
#include "NES/Mappers/Unlicensed/Bmc190in1.h"
#include "NES/Mappers/Unlicensed/Bmc235.h"
#include "NES/Mappers/Unlicensed/Bmc255.h"
#include "NES/Mappers/Unlicensed/Bmc51.h"
#include "NES/Mappers/Unlicensed/Bmc60311C.h"
#include "NES/Mappers/Unlicensed/Bmc64in1NoRepeat.h"
#include "NES/Mappers/Unlicensed/Bmc70in1.h"
#include "NES/Mappers/Unlicensed/Bmc80013B.h"
#include "NES/Mappers/Unlicensed/Bmc810544CA1.h"
#include "NES/Mappers/Unlicensed/Bmc8157.h"
#include "NES/Mappers/Unlicensed/Bmc830425C4391T.h"
#include "NES/Mappers/Unlicensed/BmcG146.h"
#include "NES/Mappers/Unlicensed/BmcK3046.h"
#include "NES/Mappers/Unlicensed/Cc21.h"
#include "NES/Mappers/Unlicensed/CityFighter.h"
#include "NES/Mappers/Unlicensed/ColorDreams.h"
#include "NES/Mappers/Unlicensed/ColorDreams46.h"
#include "NES/Mappers/Unlicensed/Dance2000.h"
#include "NES/Mappers/Unlicensed/DaouInfosys.h"
#include "NES/Mappers/Unlicensed/DreamTech01.h"
#include "NES/Mappers/Unlicensed/Edu2000.h"
#include "NES/Mappers/Unlicensed/Eh8813A.h"
#include "NES/Mappers/Unlicensed/FrontFareast.h"
#include "NES/Mappers/Unlicensed/Ghostbusters63in1.h"
#include "NES/Mappers/Unlicensed/Gkcx1.h"
#include "NES/Mappers/Unlicensed/Gs2004.h"
#include "NES/Mappers/Unlicensed/Gs2013.h"
#include "NES/Mappers/Unlicensed/Henggedianzi177.h"
#include "NES/Mappers/Unlicensed/Henggedianzi179.h"
#include "NES/Mappers/Unlicensed/Hp898f.h"
#include "NES/Mappers/Unlicensed/MagicKidGooGoo.h"
#include "NES/Mappers/Unlicensed/Malee.h"
#include "NES/Mappers/Unlicensed/Mapper103.h"
#include "NES/Mappers/Unlicensed/Mapper106.h"
#include "NES/Mappers/Unlicensed/Mapper107.h"
#include "NES/Mappers/Unlicensed/Mapper116.h"
#include "NES/Mappers/Unlicensed/Mapper117.h"
#include "NES/Mappers/Unlicensed/Mapper120.h"
#include "NES/Mappers/Unlicensed/Mapper15.h"
#include "NES/Mappers/Unlicensed/Mapper170.h"
#include "NES/Mappers/Unlicensed/Mapper183.h"
#include "NES/Mappers/Unlicensed/Mapper200.h"
#include "NES/Mappers/Unlicensed/Mapper202.h"
#include "NES/Mappers/Unlicensed/Mapper203.h"
#include "NES/Mappers/Unlicensed/Mapper204.h"
#include "NES/Mappers/Unlicensed/Mapper212.h"
#include "NES/Mappers/Unlicensed/Mapper213.h"
#include "NES/Mappers/Unlicensed/Mapper214.h"
#include "NES/Mappers/Unlicensed/Mapper216.h"
#include "NES/Mappers/Unlicensed/Mapper222.h"
#include "NES/Mappers/Unlicensed/Mapper225.h"
#include "NES/Mappers/Unlicensed/Mapper226.h"
#include "NES/Mappers/Unlicensed/Mapper227.h"
#include "NES/Mappers/Unlicensed/Mapper229.h"
#include "NES/Mappers/Unlicensed/Mapper230.h"
#include "NES/Mappers/Unlicensed/Mapper231.h"
#include "NES/Mappers/Unlicensed/Mapper233.h"
#include "NES/Mappers/Unlicensed/Mapper234.h"
#include "NES/Mappers/Unlicensed/Mapper240.h"
#include "NES/Mappers/Unlicensed/Mapper241.h"
#include "NES/Mappers/Unlicensed/Mapper244.h"
#include "NES/Mappers/Unlicensed/Mapper246.h"
#include "NES/Mappers/Unlicensed/Mapper39.h"
#include "NES/Mappers/Unlicensed/Mapper42.h"
#include "NES/Mappers/Unlicensed/Mapper43.h"
#include "NES/Mappers/Unlicensed/Mapper50.h"
#include "NES/Mappers/Unlicensed/Mapper57.h"
#include "NES/Mappers/Unlicensed/Mapper58.h"
#include "NES/Mappers/Unlicensed/Mapper60.h"
#include "NES/Mappers/Unlicensed/Mapper62.h"
#include "NES/Mappers/Unlicensed/Mapper83.h"
#include "NES/Mappers/Unlicensed/Nanjing.h"
#include "NES/Mappers/Unlicensed/Nina01.h"
#include "NES/Mappers/Unlicensed/Nina03_06.h"
#include "NES/Mappers/Unlicensed/NovelDiamond.h"
#include "NES/Mappers/Unlicensed/Racermate.h"
#include "NES/Mappers/Unlicensed/Rt01.h"
#include "NES/Mappers/Unlicensed/Subor166.h"
#include "NES/Mappers/Unlicensed/Super40in1Ws.h"
#include "NES/Mappers/Unlicensed/Supervision.h"
#include "NES/Mappers/Unlicensed/T230.h"
#include "NES/Mappers/Unlicensed/T262.h"
#include "NES/Mappers/Unlicensed/Unl255in1.h"
#include "NES/Mappers/Unlicensed/UnlD1038.h"
#include "NES/Mappers/Unlicensed/UnlPci556.h"
#include "NES/Mappers/Unlicensed/UnlPuzzle.h"
#include "NES/Mappers/Unlicensed/Yoko.h"
#include "NES/Mappers/VsSystem/VsSystem.h"
#include "NES/Mappers/Waixing/Bs5.h"
#include "NES/Mappers/Waixing/Fk23C.h"
#include "NES/Mappers/Waixing/Mapper242.h"
#include "NES/Mappers/Waixing/Mapper253.h"
#include "NES/Mappers/Waixing/Waixing162.h"
#include "NES/Mappers/Waixing/Waixing164.h"
#include "NES/Mappers/Waixing/Waixing178.h"
#include "NES/Mappers/Waixing/Waixing252.h"
#include "NES/Mappers/Whirlwind/Lh10.h"
#include "NES/Mappers/Whirlwind/Lh32.h"
#include "NES/Mappers/Whirlwind/Lh51.h"
#include "NES/Mappers/Whirlwind/Mapper40.h"
#include "NES/Mappers/Whirlwind/Smb2j.h"

BaseMapper* MapperFactory::GetMapperFromID(RomData &romData)
{
	switch(romData.Info.MapperID) {
		case 0: return new NROM();
		case 1: return new MMC1();
		case 2: return new UNROM();
		case 3: return new CNROM(false);
		case 4: 
			if(romData.Info.SubMapperID == 3) {
				return new McAcc();
			} else {
				return new MMC3();
			}
		case 5: return new MMC5();
		case 6: return new FrontFareast();
		case 7: return new AXROM();
		case 8: return new FrontFareast();
		case 9: return new MMC2();
		case 10: return new MMC4();
		case 11: return new ColorDreams();
		case 12: return new MMC3_12();
		case 13: return new CpRom();
		case 14: return new MMC3_14();
		case 15: return new Mapper15();
		case 16: return new BandaiFcg();
		case 17: return new FrontFareast();
		case 18: return new JalecoSs88006();
		case 19: return new Namco163();
		case 21: return new VRC2_4();
		case 22: return new VRC2_4();
		case 23: return new VRC2_4();
		case 24: return new VRC6(VRCVariant::VRC6a);
		case 25: return new VRC2_4();
		case 26: return new VRC6(VRCVariant::VRC6b);
		case 27: return new VRC2_4();
		case 28: return new Action53();
		case 29: return new SealieComputing();
		case 30: return new UnRom512();
		case 31: return new NsfCart31();
		case 32: return new IremG101();
		case 33: return new TaitoTc0190();
		case 34: 
			switch(romData.Info.SubMapperID) {
				case 0: return (romData.ChrRom.size() > 0) ? (BaseMapper*)new Nina01() : (BaseMapper*)new BnRom(); //BnROM uses CHR RAM (so no CHR rom in the .NES file)
				case 1: return new Nina01();
				case 2: return new BnRom();
			}
			break;

		case 35: return new Mapper35();
		case 36: return new Txc22000();
		case 37: return new MMC3_37();
		case 38: return new UnlPci556();
		case 39: return new Mapper39();
		case 40: return new Mapper40();
		case 41: return new Caltron41();
		case 42: return new Mapper42();
		case 43: return new Mapper43();
		case 44: return new MMC3_44();
		case 45: return new MMC3_45();
		case 46: return new ColorDreams46();
		case 47: return new MMC3_47();
		case 48: return new TaitoTc0690();
		case 49: return new MMC3_49();
		case 50: return new Mapper50();
		case 51: return new Bmc51();
		case 52: return new MMC3_52();
		case 53: return new Supervision();
		case 54: return new NovelDiamond();
		case 56: return new Kaiser202();
		case 57: return new Mapper57();
		case 58: return new Mapper58();
		case 59: return new UnlD1038();
		case 60: return new Mapper60();
		case 61: return new Mapper61();
		case 62: return new Mapper62();
		case 63: return new Bmc63();
		case 64: return new Rambo1();
		case 65: return new IremH3001();
		case 66: return new GxRom();
		case 67: return new Sunsoft3();
		case 68: return new Sunsoft4();
		case 69: return new SunsoftFme7();
		case 70: return new Bandai74161_7432(false);
		case 71: return new BF909x();
		case 72: return new JalecoJf17_19(false);
		case 73: return new VRC3();
		case 74: return new MMC3_ChrRam(0x08, 0x09, 2);
		case 75: return new VRC1();
		case 76: return new Namco108_76();
		case 77: return new IremLrog017();
		case 78: return new JalecoJf16();
		case 79: return new Nina03_06(false);
		case 80: return new TaitoX1005(false);
		case 82: return new TaitoX1017();
		case 83: return new Mapper83();
		case 85: return new VRC7();
		case 86: return new JalecoJf13();
		case 87: return new JalecoJfxx(false);
		case 88: return new Namco108_88();
		case 89: return new Sunsoft89();
		case 90: return new JyCompany();
		case 91: return new Mapper91();
		case 92: return new JalecoJf17_19(true);
		case 93: return new Sunsoft93();
		case 94: return new UnRom_94();
		case 95: return new Namco108_95();
		case 96: return new OekaKids();
		case 97: return new IremTamS1();
		case 99: return new VsSystem();
		case 101: return new JalecoJfxx(true);
		case 103: return new Mapper103();
		case 104: return new GoldenFive();
		case 105: return new MMC1_105();
		case 106: return new Mapper106();
		case 107: return new Mapper107();
		case 108: return new Bb();
		case 111: return new Cheapocabra();
		case 112: return new Mapper112();
		case 113: return new Nina03_06(true);
		case 114: return new MMC3_114();
		case 115: return new MMC3_115();
		case 116: return new Mapper116();
		case 117: return new Mapper117();
		case 118: return new TxSRom();
		case 119: return new MMC3_ChrRam(0x40, 0x7F, 8);
		case 120: return new Mapper120();
		case 121: return new MMC3_121();
		case 123: return new MMC3_123();
		case 125: return new Lh32();
		case 126: return new MMC3_126();
		case 132: return new Txc22211A();
		case 133: return new Sachen_133();
		case 134: return new MMC3_134();
		case 136: return new Sachen_136();
		case 137: return new Sachen8259(Sachen8259Variant::Sachen8259D);
		case 138: return new Sachen8259(Sachen8259Variant::Sachen8259B);
		case 139: return new Sachen8259(Sachen8259Variant::Sachen8259C);
		case 140: return new JalecoJf11_14();
		case 141: return new Sachen8259(Sachen8259Variant::Sachen8259A);
		case 142: return new Kaiser202();
		case 143: return new Sachen_143();
		case 144: return new ColorDreams();
		case 145: return new Sachen_145();
		case 146: return new Nina03_06(false);
		case 147: return new Sachen_147();
		case 148: return new Sachen_148();
		case 149: return new Sachen_149();
		case 150: return new Sachen74LS374N();
		case 151: return new VRC1();
		case 152: return new Bandai74161_7432(true);
		case 153: return new BandaiFcg();
		case 154: return new Namco108_154();
		case 155: return new MMC1_155();
		case 156: return new DaouInfosys();
		case 157: return new BandaiFcg();
		case 158: return new Rambo1_158();
		case 159: return new BandaiFcg();
		case 162: return new Waixing162();
		case 163: return new Nanjing();
		case 164: return new Waixing164();
		case 165: return new MMC3_165();
		case 166: return new Subor166();
		case 167: return new Subor166();
		case 168: return new Racermate();
		case 170: return new Mapper170();
		case 171: return new Kaiser7058();
		case 172: return new Txc22211B();
		case 173: return new Txc22211C();
		case 174: return new Mapper174();
		case 175: return new Kaiser7022();
		case 176: return new Fk23C();
		case 177: return new Henggedianzi177();
		case 178: return new Waixing178();
		case 179: return new Henggedianzi179();
		case 180: return new UnRom_180();
		case 182: return new MMC3_182();
		case 183: return new Mapper183();
		case 184: return new Sunsoft184();
		case 185: return new CNROM(true);
		case 186: break; //The study box is handled as a bios file, not a iNES rom
		case 187: return new MMC3_187();
		case 188: return new BandaiKaraoke();
		case 189: return new MMC3_189();
		case 190: return new MagicKidGooGoo();
		case 191: return new MMC3_ChrRam(0x80, 0xFF, 2);
		case 192: return new MMC3_ChrRam(0x08, 0x0B, 4);
		case 193: return new NtdecTc112();
		case 194: return new MMC3_ChrRam(0x00, 0x01, 2);
		case 195: return new MMC3_ChrRam(0x00, 0x03, 4);
		case 196: return new MMC3_196();
		case 197: return new MMC3_197();
		case 198: return new MMC3_198();
		case 199: return new MMC3_199();
		case 200: return new Mapper200();
		case 201: return new NovelDiamond();
		case 202: return new Mapper202();
		case 203: return new Mapper203();
		case 204: return new Mapper204();
		case 205: return new MMC3_205();
		case 206: return new Namco108();
		case 207: return new TaitoX1005(true);
		case 208: return new MMC3_208();
		case 209: return new JyCompany();
		case 210: return new Namco163();
		case 211: return new JyCompany();
		case 212: return new Mapper212();
		case 213: return new Mapper213();
		case 214: return new Mapper214();
		case 215: return new MMC3_215();
		case 216: return new Mapper216();
		case 217: return new MMC3_217();
		case 218: return new MagicFloor218();
		case 219: return new MMC3_219();
		case 221: return new Mapper221();
		case 222: return new Mapper222();
		case 224: return new MMC3_224();
		case 225: return new Mapper225();
		case 226: return new Mapper226();
		case 227: return new Mapper227();
		case 228: return new ActionEnterprises();
		case 229: return new Mapper229();
		case 230: return new Mapper230();
		case 231: return new Mapper231();
		case 232: return new BF9096();
		case 233: return new Mapper233();
		case 234: return new Mapper234();
		case 235: return new Bmc235();
		case 236: return new Bmc70in1();
		case 238: return new MMC3_238();
		case 240: return new Mapper240();
		case 241: return new Mapper241();
		case 242: return new Mapper242();
		case 243: return new Sachen74LS374N();
		case 244: return new Mapper244();
		case 245: return new MMC3_245();
		case 246: return new Mapper246();
		case 249: return new MMC3_249();
		case 250: return new MMC3_250();
		case 252: return new Waixing252();
		case 253: return new Mapper253();
		case 254: return new MMC3_254();
		case 255: return new Bmc255();

		case 256: break; //ONEBUS
		case 257: break; //PEC-586
		case 258: return new Unl158B();
		case 259: return new MMC3_BmcF15();
		case 260: return new BmcHpxx();
		case 261: return new Bmc810544CA1();
		case 262: return new MMC3_StreetHeroes();
		case 263: return new MMC3_Kof97();
		case 264: return new Yoko();
		case 265: return new T262();
		case 266: return new CityFighter();
		//267
		case 268: return new MMC3_Coolboy();
		//269-270
		case 271: break; //22026
		//272-273
		case 274: return new Bmc80013B();
		//275-282
		case 283: return new Gs2004();
		case 284: return new UnlDripGame();
		case 285: return new A65AS();
		case 286: return new Bs5();
		case 287: return new MMC3_Bmc411120C(); //+ K-3088
		case 288: return new Gkcx1();
		case 289: return new Bmc60311C();
		case 290: return new BmcNtd03();
		//291
		case 292: return new DragonFighter();
		//293-294
		case 295: break; //13IN1JY110
		//296-297
		case 298: return new Tf1201();
		case 299: return new Bmc11160();
		case 300: return new Bmc190in1();
		case 301: return new Bmc8157();
		case 302: return new Kaiser7057();
		case 303: return new Kaiser7017();
		case 304: return new Smb2j();
		case 305: return new Kaiser7031();
		case 306: return new Kaiser7016();
		case 307: return new Kaiser7037();
		//case 308: break; //TH2131-1
		case 309: return new Lh51();
		//310-311
		case 312: return new Kaiser7013B();
		case 313: return new ResetTxrom();
		case 314: return new Bmc64in1NoRepeat();
		case 315: break; //830134C
		//316-318
		case 319: return new Hp898f();
		case 320: return new Bmc830425C4391T();
		//321
		case 322: break; //K-3033
		case 323: return new FaridSlrom();
		case 324: return new FaridUnrom();
		case 325: return new MMC3_MaliSB();
		case 327: break; //10-24-C-A1
		case 328: return new Rt01();
		case 329: return new Edu2000();
		//330
		case 331: return new Bmc12in1();
		case 332: return new Super40in1Ws();
		case 333: return new Bmc8in1(); // + NEWSTAR-GRM070-8IN1
		//334
		case 335: break; //CTC-09
		case 336: return new BmcK3046();
		case 337: break; //CTC-12IN1
		case 338: break; //SA005-A
		case 339: break; //K-3006
		case 340: break; //K-3036
		case 341: break; //TJ-03
		case 342: break; //COOLGIRL
		//343
		case 344: break; //GN26
		case 345: break; //L6IN1
		case 346: return new Kaiser7012();
		//347
		case 348: return new Bmc830118C();
		case 349: return new BmcG146();
		case 350: break; //891227

		case 366: return new BmcGn45();

		case 513: return new Sachen9602();
		//514-517
		case 518: return new Dance2000();
		case 519: return new Eh8813A();
		//520
		case 521: return new DreamTech01();
		case 522: return new Lh10();
		//523
		case 524: break; //900218
		case 525: break; //KS7021A
		case 526: break; //BJ56
		case 527: break; //AX40G
		case 528: break; //831128C
		case 529: return new T230();
		case 530: return new Ax5705();
		
		case 682: return new Rainbow();

		case UnifBoards::Ac08: return new Ac08(); //mapper 42?
		case UnifBoards::Cc21: return new Cc21();
		case UnifBoards::Ghostbusters63in1: return new Ghostbusters63in1(); //mapper 226?
		case UnifBoards::Gs2013: return new Gs2013();
		case UnifBoards::Malee: return new Malee(); //mapper 42?
		case UnifBoards::SssNrom256: return new FamicomBox();
		case UnifBoards::Unl255in1: return new Unl255in1();
		case UnifBoards::Unl8237A: return new Unl8237A(); //mapper 215.1
		case UnifBoards::UnlPuzzle: return new UnlPuzzle();

		case MapperFactory::FamicomNetworkSystemMapperID: return new FnsMmc1();
		case MapperFactory::StudyBoxMapperID: return new StudyBox();
		case MapperFactory::NsfMapperID: return new NsfMapper();
		case MapperFactory::FdsMapperID: return new Fds();
	}

	if(romData.Info.MapperID != UnifBoards::UnknownBoard) {
		MessageManager::DisplayMessage("Error", "UnsupportedMapper", "iNES #" + std::to_string(romData.Info.MapperID));
	}
	return nullptr;
}

unique_ptr<BaseMapper> MapperFactory::InitializeFromFile(NesConsole* console, VirtualFile &romFile, RomData &romData, LoadRomResult& result)
{
	romData = {};
	bool databaseEnabled = !console->GetNesConfig().DisableGameDatabase;
	if(RomLoader::LoadFile(romFile, romData, databaseEnabled)) {
		unique_ptr<BaseMapper> mapper(GetMapperFromID(romData));
		if(mapper) {
			result = LoadRomResult::Success;
			mapper->Initialize(console, romData);
			return mapper;
		} else {
			//File is a valid NES file, but it couldn't be loaded
			result = LoadRomResult::Failure;
			return nullptr;
		}
	}
	result = LoadRomResult::UnknownType;
	return nullptr;
}

