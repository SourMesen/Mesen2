#include "stdafx.h"
#include "Shared/NotificationManager.h"
#include "NES/MapperFactory.h"
#include "NES/NesConsole.h"
#include "NES/Loaders/RomLoader.h"
//#include "UnifBoards.h"
#include "NES/BaseMapper.h"
#include "NES/RomData.h"
#include "Utilities/VirtualFile.h"
/*
#include "A65AS.h"
#include "Ac08.h"
#include "Action53.h"
#include "ActionEnterprises.h"
#include "Ax5705.h"*/
#include "NES/Mappers/AXROM.h"
/*#include "Bandai74161_7432.h"
#include "BandaiFcg.h"
#include "BandaiKaraoke.h"
#include "Bb.h"*/
#include "NES/Mappers/BF909x.h"
/*#include "BF9096.h"
#include "Bmc11160.h"
#include "Bmc12in1.h"
#include "Bmc51.h"
#include "Bmc63.h"
#include "Bmc64in1NoRepeat.h"
#include "Bmc70in1.h"
#include "Bmc190in1.h"
#include "Bmc235.h"
#include "Bmc255.h"
#include "Bmc60311C.h"
#include "Bmc8157.h"
#include "Bmc80013B.h"
#include "Bmc810544CA1.h"
#include "Bmc830118C.h"
#include "Bmc830425C4391T.h"
#include "Bmc8in1.h"
#include "BmcG146.h"
#include "BmcGn45.h"
#include "BmcHpxx.h"
#include "BmcK3046.h"
#include "BmcNtd03.h"
#include "BnRom.h"
#include "Bs5.h"
#include "Caltron41.h"
#include "Cc21.h"
#include "Cheapocabra.h"
#include "CityFighter.h"*/
#include "NES/Mappers/CNROM.h"
/*#include "CpRom.h"
#include "ColorDreams.h"
#include "ColorDreams46.h"
#include "Dance2000.h"
#include "DaouInfosys.h"
#include "DragonFighter.h"
#include "DreamTech01.h"
#include "Edu2000.h"
#include "Eh8813A.h"
#include "FamicomBox.h"
#include "FaridSlrom.h"
#include "FaridUnrom.h"*/
#include "NES/Mappers/FDS/Fds.h"
#include "NES/Mappers/FDS/FdsAudio.h"
/*#include "Fk23C.h"
#include "FrontFareast.h"
#include "Ghostbusters63in1.h"
#include "Gkcx1.h"
#include "GoldenFive.h"
#include "Gs2004.h"
#include "Gs2013.h"
#include "GxRom.h"
#include "Henggedianzi177.h"
#include "Henggedianzi179.h"
#include "Hp898f.h"
#include "IremG101.h"
#include "IremH3001.h"
#include "IremLrog017.h"
#include "IremTamS1.h"
#include "JalecoJf11_14.h"
#include "JalecoJf13.h"
#include "JalecoJf16.h"
#include "JalecoJf17_19.h"
#include "JalecoJfxx.h"
#include "JalecoSs88006.h"
#include "JyCompany.h"
#include "Kaiser202.h"
#include "Kaiser7012.h"
#include "Kaiser7013B.h"
#include "Kaiser7016.h"
#include "Kaiser7017.h"
#include "Kaiser7022.h"
#include "Kaiser7031.h"
#include "Kaiser7037.h"
#include "Kaiser7057.h"
#include "Kaiser7058.h"
#include "Lh10.h"
#include "Lh32.h"
#include "Lh51.h"
#include "Malee.h"
#include "MagicKidGooGoo.h"
#include "Mapper15.h"
#include "Mapper35.h"
#include "Mapper39.h"
#include "Mapper40.h"
#include "Mapper42.h"
#include "Mapper43.h"
#include "Mapper50.h"
#include "Mapper57.h"
#include "Mapper58.h"
#include "Mapper60.h"
#include "Mapper61.h"
#include "Mapper62.h"
#include "Mapper83.h"
#include "Mapper91.h"
#include "Mapper103.h"
#include "Mapper106.h"
#include "Mapper107.h"
#include "Mapper112.h"
#include "Mapper116.h"
#include "Mapper117.h"
#include "Mapper120.h"
#include "Mapper170.h"
#include "Mapper174.h"
#include "Mapper183.h"
#include "Mapper200.h"
#include "Mapper202.h"
#include "Mapper203.h"
#include "Mapper204.h"
#include "Mapper212.h"
#include "Mapper213.h"
#include "Mapper214.h"
#include "Mapper216.h"
#include "Mapper218.h"
#include "Mapper221.h"
#include "Mapper222.h"
#include "Mapper225.h"
#include "Mapper226.h"
#include "Mapper227.h"
#include "Mapper229.h"
#include "Mapper230.h"
#include "Mapper231.h"
#include "Mapper233.h"
#include "Mapper234.h"
#include "Mapper240.h"
#include "Mapper241.h"
#include "Mapper242.h"
#include "Mapper244.h"
#include "Mapper246.h"
#include "Mapper253.h"
#include "McAcc.h"*/
#include "NES/Mappers/MMC1.h"
/*#include "MMC1_105.h"
#include "MMC1_155.h"
#include "MMC2.h"*/
#include "NES/Mappers/MMC3.h"
/*#include "MMC3_12.h"
#include "MMC3_14.h"
#include "MMC3_37.h"
#include "MMC3_44.h"
#include "MMC3_45.h"
#include "MMC3_47.h"
#include "MMC3_49.h"
#include "MMC3_52.h"
#include "MMC3_114.h"
#include "MMC3_115.h"
#include "MMC3_121.h"
#include "MMC3_123.h"
#include "MMC3_126.h"
#include "MMC3_134.h"
#include "MMC3_165.h"
#include "MMC3_182.h"
#include "MMC3_187.h"
#include "MMC3_189.h"
#include "MMC3_196.h"
#include "MMC3_197.h"
#include "MMC3_198.h"
#include "MMC3_199.h"
#include "MMC3_205.h"
#include "MMC3_208.h"
#include "MMC3_215.h"
#include "MMC3_217.h"
#include "MMC3_219.h"
#include "MMC3_224.h"
#include "MMC3_238.h"
#include "MMC3_245.h"
#include "MMC3_249.h"
#include "MMC3_250.h"
#include "MMC3_254.h"
#include "MMC3_Bmc411120C.h"
#include "MMC3_BmcF15.h"
#include "MMC3_ChrRam.h"
#include "MMC3_Coolboy.h"
#include "MMC3_Kof97.h"
#include "MMC3_MaliSB.h"
#include "MMC3_StreetHeroes.h"
#include "MMC4.h"*/
#include "NES/Mappers/MMC5.h"
/*#include "Namco108.h"
#include "Namco108_76.h"
#include "Namco108_88.h"
#include "Namco108_95.h"
#include "Namco108_154.h"
#include "Namco163.h"
#include "Nanjing.h"
#include "Nina01.h"
#include "Nina03_06.h"
#include "NovelDiamond.h"*/
#include "NES/Mappers/NROM.h"
/*#include "NsfCart31.h"*/
#include "NES/Mappers/NsfMapper.h"
//#include "NtdecTc112.h"
#include "NES/Mappers/OekaKids.h"
/*#include "Racermate.h"
#include "Rambo1.h"
#include "Rambo1_158.h"
#include "ResetTxrom.h"
#include "Rt01.h"
#include "Sachen_133.h"
#include "Sachen_136.h"
#include "Sachen_143.h"
#include "Sachen_145.h"
#include "Sachen_147.h"
#include "Sachen_148.h"
#include "Sachen_149.h"
#include "Sachen74LS374N.h"
#include "Sachen8259.h"
#include "Sachen9602.h"
#include "SealieComputing.h"
#include "Smb2j.h"
#include "StudyBox.h"
#include "Subor166.h"
#include "Sunsoft3.h"
#include "Sunsoft4.h"
#include "Sunsoft89.h"
#include "Sunsoft93.h"
#include "Sunsoft184.h"*/
#include "NES/Mappers/SunsoftFme7.h"
/*#include "Supervision.h"
#include "Super40in1Ws.h"
#include "T230.h"
#include "T262.h"
#include "TaitoTc0190.h"
#include "TaitoTc0690.h"
#include "TaitoX1005.h"
#include "TaitoX1017.h"
#include "Tf1201.h"
#include "Txc22000.h"
#include "Txc22211A.h"
#include "Txc22211B.h"
#include "Txc22211C.h"*/
#include "NES/Mappers/TxSRom.h"
/*#include "Unl158B.h"
#include "Unl255in1.h"
#include "Unl8237A.h"
#include "UnlD1038.h"
#include "UnlDripGame.h"
#include "UnlPci556.h"
#include "UnlPuzzle.h"*/
#include "NES/Mappers/UNROM.h"
/*#include "UnRom_94.h"
#include "UnRom_180.h"
#include "UnRom512.h"
#include "VRC1.h"
#include "VRC2_4.h"
#include "VRC3.h"
#include "VRC6.h"
#include "VRC7.h"*/
#include "NES/Mappers/VsSystem/VsSystem.h"
/*#include "Waixing162.h"
#include "Waixing164.h"
#include "Waixing178.h"
#include "Waixing252.h"
#include "Yoko.h"*/

/*
Supported mappers:  
... : bad mappers
--- : potentially bad mappers
=== : not supported by both Nestopia & FCEUX
??? : No known roms
-----------------------------------------------------------------
| 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13| 14| 15|
| 16| 17| 18| 19|...| 21| 22| 23| 24| 25| 26| 27| 28| 29| 30| 31|
| 32| 33| 34| 35| 36| 37| 38| 39| 40| 41| 42| 43| 44| 45| 46| 47|
| 48| 49| 50| 51| 52| 53| 54|???| 56| 57| 58| 59| 60| 61| 62| 63|
| 64| 65| 66| 67| 68| 69| 70| 71| 72| 73| 74| 75| 76| 77| 78| 79|
| 80|===| 82| 83|===| 85| 86| 87| 88| 89| 90| 91| 92| 93| 94| 95|
| 96| 97|===| 99|...|101|===|103|104|105|106|107|108|===|===|111|
|112|113|114|115|116|117|118|119|120|121|===|123|===|125|126|===|
|===|===|===|===|132|133|134|===|136|137|138|139|140|141|142|143|
|144|145|146|147|148|149|150|151|152|153|154|155|156|157|158|159|
|---|===|162|163|164|165|166|167|168|===|170|171|172|173|174|175|
|176|177|178|179|180|---|182|183|184|185|186|187|188|189|190|191|
|192|193|194|195|196|197|198|199|200|201|202|203|204|205|206|207|
|208|209|210|211|212|213|214|215|216|217|218|219|220|221|222|???|
|224|225|226|227|228|229|230|231|232|233|234|235|236|===|238|===|
|240|241|242|243|244|245|246|===|===|249|250|===|252|253|254|255|
-----------------------------------------------------------------
*/

BaseMapper* MapperFactory::GetMapperFromID(RomData &romData)
{
#ifdef _DEBUG
	MessageManager::DisplayMessage("GameInfo", "Mapper", std::to_string(romData.Info.MapperID), std::to_string(romData.Info.SubMapperID));
#endif

	switch(romData.Info.MapperID) {
		case 0: return new NROM();
		case 1: return new MMC1();
		case 2: return new UNROM();
		case 3: return new CNROM(false);
		case 4: 
			if(romData.Info.SubMapperID == 3) {
				//return new McAcc();
			} else {
				return new MMC3();
			}
		case 5: return new MMC5();
		//case 6: return new FrontFareast();
		case 7: return new AXROM();
		/*case 8: return new FrontFareast();
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
		case 68: return new Sunsoft4();*/
		case 69: return new SunsoftFme7();
		//case 70: return new Bandai74161_7432(false);
		case 71: return new BF909x();
		/*case 72: return new JalecoJf17_19(false);
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
		case 95: return new Namco108_95();*/
		case 96: return new OekaKids();
		//case 97: return new IremTamS1();
		case 99: return new VsSystem();
		/*case 101: return new JalecoJfxx(true);
		case 103: return new Mapper103();
		case 104: return new GoldenFive();
		case 105: return new MMC1_105(); break;
		case 106: return new Mapper106();
		case 107: return new Mapper107();
		case 108: return new Bb();
		case 111: return new Cheapocabra();
		case 112: return new Mapper112();
		case 113: return new Nina03_06(true);
		case 114: return new MMC3_114();
		case 115: return new MMC3_115();
		case 116: return new Mapper116();
		case 117: return new Mapper117();*/
		case 118: return new TxSRom();
		/*case 119: return new MMC3_ChrRam(0x40, 0x7F, 8);
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
		case 184: return new Sunsoft184();*/
		case 185: return new CNROM(true);
		/*case 186: break; //The study box is handled as a bios file, not a iNES rom
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
		case 218: return new Mapper218();
		case 219: return new MMC3_219();
		case 220: return new Kaiser7057();
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
		case 308: break; //TH2131-1
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

		case UnifBoards::Ac08: return new Ac08(); //mapper 42?
		case UnifBoards::Cc21: return new Cc21();
		case UnifBoards::Ghostbusters63in1: return new Ghostbusters63in1(); //mapper 226?
		case UnifBoards::Gs2013: return new Gs2013();
		case UnifBoards::Malee: return new Malee(); //mapper 42?
		case UnifBoards::SssNrom256: return new FamicomBox();
		case UnifBoards::Unl255in1: return new Unl255in1();
		case UnifBoards::Unl8237A: return new Unl8237A(); //mapper 215.1
		case UnifBoards::UnlPuzzle: return new UnlPuzzle();

		case MapperFactory::StudyBoxMapperID: return new StudyBox();*/
		case MapperFactory::NsfMapperID: return new NsfMapper();
		case MapperFactory::FdsMapperID: return new Fds();
	}

	/*if(romData.Info.MapperID != UnifBoards::UnknownBoard) {
		MessageManager::DisplayMessage("Error", "UnsupportedMapper", "iNES #" + std::to_string(romData.Info.MapperID));
	}*/
	return nullptr;
}

unique_ptr<BaseMapper> MapperFactory::InitializeFromFile(NesConsole* console, VirtualFile &romFile, RomData &romData, LoadRomResult& result)
{
	romData = {};
	bool databaseEnabled = !console->GetNesConfig().DisableGameDatabase;
	if(RomLoader::LoadFile(romFile, romData, databaseEnabled)) {
		if((romData.Info.IsInDatabase || romData.Info.IsNes20Header) && romData.Info.InputType != GameInputType::Unspecified) {
			//If in DB or a NES 2.0 file, auto-configure the inputs
			//TODO NES
			/*
			if(console->GetNesConfig()->CheckFlag(EmulationFlags::AutoConfigureInput)) {
				console->GetSettings()->InitializeInputDevices(romData.Info.InputType, romData.Info.System, false);
			}
			*/
		}

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

