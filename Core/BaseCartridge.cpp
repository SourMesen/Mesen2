#include "stdafx.h"
#include "BaseCartridge.h"
#include "RamHandler.h"
#include "RomHandler.h"
#include "MemoryMappings.h"
#include "IMemoryHandler.h"
#include "BaseCoprocessor.h"
#include "MessageManager.h"
#include "Console.h"
#include "EmuSettings.h"
#include "BatteryManager.h"
#include "NecDsp.h"
#include "Sa1.h"
#include "Gsu.h"
#include "Sdd1.h"
#include "Cx4.h"
#include "Obc1.h"
#include "SpcFileData.h"
#include "../Utilities/HexUtilities.h"
#include "../Utilities/VirtualFile.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/Serializer.h"
#include "../Utilities/sha1.h"

BaseCartridge::~BaseCartridge()
{
	SaveBattery();

	delete[] _prgRom;
	delete[] _saveRam;
}

shared_ptr<BaseCartridge> BaseCartridge::CreateCartridge(Console* console, VirtualFile &romFile, VirtualFile &patchFile)
{
	if(romFile.IsValid()) {
		shared_ptr<BaseCartridge> cart(new BaseCartridge());
		if(patchFile.IsValid()) {
			cart->_patchPath = patchFile;
			if(romFile.ApplyPatch(patchFile)) {
				MessageManager::DisplayMessage("Patch", "ApplyingPatch", patchFile.GetFileName());
			}
		}

		vector<uint8_t> romData;
		romFile.ReadFile(romData);
		
		if(romData.size() < 0x8000) {
			return nullptr;
		}

		cart->_console = console;
		cart->_romPath = romFile;
		cart->_prgRomSize = (uint32_t)romData.size();
		cart->_prgRom = new uint8_t[cart->_prgRomSize];
		memcpy(cart->_prgRom, romData.data(), cart->_prgRomSize);

		if(memcmp(cart->_prgRom, "SNES-SPC700 Sound File Data", 27) == 0) {
			if(cart->_prgRomSize >= 0x10200) {
				//SPC files must be 0x10200 bytes long at minimum
				cart->LoadSpc();
			} else {
				return nullptr;
			}
		} else {
			cart->LoadRom();
		}

		return cart;
	} else {
		return nullptr;
	}
}

int32_t BaseCartridge::GetHeaderScore(uint32_t addr)
{
	//Try to figure out where the header is by using a scoring system
	if(_prgRomSize < addr + 0x7FFF) {
		return -1;
	}

	SnesCartInformation cartInfo;
	memcpy(&cartInfo, _prgRom + addr + 0x7FB0, sizeof(SnesCartInformation));
	
	uint32_t score = 0;
	uint8_t mode = (cartInfo.MapMode & ~0x10);
	if((mode == 0x20 || mode == 0x22) && addr < 0x8000) {
		score++;
	} else if((mode == 0x21 || mode == 0x25) && addr >= 0x8000) {
		score++;
	}

	if(cartInfo.RomType < 0x08) {
		score++;
	}
	if(cartInfo.RomSize < 0x10) {
		score++;
	}
	if(cartInfo.SramSize < 0x08) {
		score++;
	}

	uint16_t checksum = cartInfo.Checksum[0] | (cartInfo.Checksum[1] << 8);
	uint16_t complement = cartInfo.ChecksumComplement[0] | (cartInfo.ChecksumComplement[1] << 8);
	if(checksum + complement == 0xFFFF && checksum != 0 && complement != 0) {
		score += 8;
	}

	uint32_t resetVectorAddr = addr + 0x7FFC;
	uint32_t resetVector = _prgRom[resetVectorAddr] | (_prgRom[resetVectorAddr + 1] << 8);
	if(resetVector < 0x8000) {
		return -1;
	}
	
	uint8_t op = _prgRom[addr + (resetVector & 0x7FFF)];
	if(op == 0x18 || op == 0x78 || op == 0x4C || op == 0x5C || op == 0x20 || op == 0x22 || op == 0x9C) {
		//CLI, SEI, JMP, JML, JSR, JSl, STZ
		score += 8;
	} else if(op == 0xC2 || op == 0xE2 || op == 0xA9 || op == 0xA2 || op == 0xA0) {
		//REP, SEP, LDA, LDX, LDY
		score += 4;
	} else if(op == 0x00 || op == 0xFF || op == 0xCC) {
		//BRK, SBC, CPY
		score -= 8;
	}

	return std::max<int32_t>(0, score);
}

void BaseCartridge::LoadRom()
{
	//Find the best potential header among lorom/hirom + headerless/headered combinations
	vector<uint32_t> baseAddresses = { 0, 0x200, 0x8000, 0x8200, 0x408000, 0x408200 };
	int32_t bestScore = -1;
	bool hasHeader = false;
	bool isLoRom = true;
	bool isExRom = true;
	for(uint32_t baseAddress : baseAddresses) {
		int32_t score = GetHeaderScore(baseAddress);
		if(score >= 0 && score >= bestScore) {
			bestScore = score;
			isLoRom = (baseAddress & 0x8000) == 0;
			isExRom = (baseAddress & 0x400000) != 0;
			hasHeader = (baseAddress & 0x200) != 0;
			uint32_t headerOffset = std::min(baseAddress + 0x7FB0, _prgRomSize - 0x40);
			memcpy(&_cartInfo, _prgRom + headerOffset, sizeof(SnesCartInformation));
		}
	}

	uint32_t flags = 0;
	if(isLoRom) {
		if(hasHeader) {
			flags |= CartFlags::CopierHeader;
		}
		flags |= CartFlags::LoRom;
	} else {
		if(hasHeader) {
			flags |= CartFlags::CopierHeader;
		}
		flags |= isExRom ? CartFlags::ExHiRom : CartFlags::HiRom;
	}

	if(flags & CartFlags::CopierHeader) {
		//Remove the copier header
		memmove(_prgRom, _prgRom + 512, _prgRomSize - 512);
		_prgRomSize -= 512;
	}
	
	if((flags & CartFlags::HiRom) && (_cartInfo.MapMode & 0x27) == 0x25) {
		flags |= CartFlags::ExHiRom;
	} else if((flags & CartFlags::LoRom) && (_cartInfo.MapMode & 0x27) == 0x22) {
		flags |= CartFlags::ExLoRom;
	}

	if(_cartInfo.MapMode & 0x10) {
		flags |= CartFlags::FastRom;
	}
	_flags = (CartFlags::CartFlags)flags;

	_hasBattery = (_cartInfo.RomType & 0x0F) == 0x02 || (_cartInfo.RomType & 0x0F) == 0x05 || (_cartInfo.RomType & 0x0F) == 0x06 || (_cartInfo.RomType & 0x0F) == 0x09 || (_cartInfo.RomType & 0x0F) == 0x0A;
	_coprocessorType = GetCoprocessorType();

	if(_coprocessorType != CoprocessorType::None && _cartInfo.ExpansionRamSize > 0 && _cartInfo.ExpansionRamSize <= 7) {
		_coprocessorRamSize = _cartInfo.ExpansionRamSize > 0 ? 1024 * (1 << _cartInfo.ExpansionRamSize) : 0;
	}

	if(_coprocessorType == CoprocessorType::GSU && _coprocessorRamSize == 0) {
		//Use a min of 64kb by default for GSU games
		_coprocessorRamSize = 0x10000;
	}

	_saveRamSize = _cartInfo.SramSize > 0 ? 1024 * (1 << _cartInfo.SramSize) : 0;
	_saveRam = new uint8_t[_saveRamSize];
	_console->GetSettings()->InitializeRam(_saveRam, _saveRamSize);

	DisplayCartInfo();
}

CoprocessorType BaseCartridge::GetCoprocessorType()
{
	if((_cartInfo.RomType & 0x0F) >= 0x03) {
		switch((_cartInfo.RomType & 0xF0) >> 4) {
			case 0x00: return GetDspVersion(); break;
			case 0x01: return CoprocessorType::GSU; break;
			case 0x02: return CoprocessorType::OBC1; break;
			case 0x03: return CoprocessorType::SA1; break;
			case 0x04: return CoprocessorType::SDD1; break;
			case 0x05: return CoprocessorType::RTC; break;
			case 0x0E: return CoprocessorType::Satellaview; break;
			case 0x0F:
				switch(_cartInfo.CartridgeType) {
					case 0x00: return CoprocessorType::SPC7110; _hasBattery = true; break;
					case 0x01: return GetSt01xVersion(); _hasBattery = true; break;
					case 0x02: return CoprocessorType::ST018; _hasBattery = true; break;
					case 0x10: return CoprocessorType::CX4; break;
				}
				break;
		}
	}

	return CoprocessorType::None;
}

CoprocessorType BaseCartridge::GetSt01xVersion()
{
	string cartName = GetCartName();
	if(cartName == "2DAN MORITA SHOUGI") {
		return CoprocessorType::ST011;
	}

	return CoprocessorType::ST010;
}

CoprocessorType BaseCartridge::GetDspVersion()
{
	string cartName = GetCartName();
	if(cartName == "DUNGEON MASTER") {
		return CoprocessorType::DSP2;
	} if(cartName == "PILOTWINGS") {
		return CoprocessorType::DSP1;
	} else if(cartName == "SD\xB6\xDE\xDD\xC0\xDE\xD1GX") {
		//SD Gundam GX
		return CoprocessorType::DSP3;
	} else if(cartName == "PLANETS CHAMP TG3000" || cartName == "TOP GEAR 3000") {
		return CoprocessorType::DSP4;
	}
	
	//Default to DSP1B
	return CoprocessorType::DSP1B;
}

void BaseCartridge::Reset()
{
	if(_coprocessor) {
		_coprocessor->Reset();
	}
}

RomInfo BaseCartridge::GetRomInfo()
{
	RomInfo info;
	info.Header = _cartInfo;
	info.RomFile = static_cast<VirtualFile>(_romPath);
	info.PatchFile = static_cast<VirtualFile>(_patchPath);
	info.Coprocessor = _coprocessorType;
	return info;
}

string BaseCartridge::GetSha1Hash()
{
	return SHA1::GetHash(_prgRom, _prgRomSize);
}

CartFlags::CartFlags BaseCartridge::GetCartFlags()
{
	return _flags;
}

void BaseCartridge::LoadBattery()
{
	if(_saveRamSize > 0) {
		_console->GetBatteryManager()->LoadBattery(".srm", _saveRam, _saveRamSize);
	} else if(_coprocessor && _hasBattery) {
		_coprocessor->LoadBattery();
	}
}

void BaseCartridge::SaveBattery()
{
	if(_saveRamSize > 0) {
		_console->GetBatteryManager()->SaveBattery(".srm", _saveRam, _saveRamSize);
	} else if(_coprocessor && _hasBattery) {
		_coprocessor->SaveBattery();
	}
}

void BaseCartridge::Init(MemoryMappings &mm)
{
	for(uint32_t i = 0; i < _prgRomSize; i += 0x1000) {
		_prgRomHandlers.push_back(unique_ptr<RomHandler>(new RomHandler(_prgRom, i, _prgRomSize, SnesMemoryType::PrgRom)));
	}

	uint32_t power = (uint32_t)std::log2(_prgRomSize);
	if(_prgRomSize >(1u << power)) {
		//If size isn't a power of 2, mirror the part above the nearest (lower) power of 2 until the size reaches the next power of 2.
		uint32_t halfSize = 1 << power;
		uint32_t fullSize = 1 << (power + 1);
		uint32_t extraHandlers = std::max<uint32_t>((_prgRomSize - halfSize) / 0x1000, 1);

		while(_prgRomHandlers.size() < fullSize / 0x1000) {
			for(uint32_t i = 0; i < extraHandlers; i += 0x1000) {
				_prgRomHandlers.push_back(unique_ptr<RomHandler>(new RomHandler(_prgRom, halfSize + i, _prgRomSize, SnesMemoryType::PrgRom)));
			}
		}
	}

	for(uint32_t i = 0; i < _saveRamSize; i += 0x1000) {
		_saveRamHandlers.push_back(unique_ptr<RamHandler>(new RamHandler(_saveRam, i, _saveRamSize, SnesMemoryType::SaveRam)));
	}

	RegisterHandlers(mm);
	InitCoprocessor();
	LoadBattery();
}

void BaseCartridge::RegisterHandlers(MemoryMappings &mm)
{
	if(MapSpecificCarts(mm) || _coprocessorType == CoprocessorType::GSU || _coprocessorType == CoprocessorType::SDD1 || _coprocessorType == CoprocessorType::CX4) {
		return;
	}

	if(_flags & CartFlags::LoRom) {
		mm.RegisterHandler(0x00, 0x7D, 0x8000, 0xFFFF, _prgRomHandlers);
		mm.RegisterHandler(0x80, 0xFF, 0x8000, 0xFFFF, _prgRomHandlers);

		if(_saveRamSize > 0) {
			if(_prgRomSize >= 1024 * 1024 * 2) {
				//For games >= 2mb in size, put ROM at 70-7D/F0-FF:0000-7FFF (e.g: Fire Emblem: Thracia 776) 
				mm.RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, _saveRamHandlers);
				mm.RegisterHandler(0xF0, 0xFF, 0x0000, 0x7FFF, _saveRamHandlers);
			} else {
				//For games < 2mb in size, put save RAM at 70-7D/F0-FF:0000-FFFF (e.g: Wanderers from Ys) 
				mm.RegisterHandler(0x70, 0x7D, 0x0000, 0xFFFF, _saveRamHandlers);
				mm.RegisterHandler(0xF0, 0xFF, 0x0000, 0xFFFF, _saveRamHandlers);
			}
		}
	} else if(_flags & CartFlags::HiRom) {
		mm.RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, _prgRomHandlers, 8);
		mm.RegisterHandler(0x40, 0x7D, 0x0000, 0xFFFF, _prgRomHandlers, 0);
		mm.RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, _prgRomHandlers, 8);
		mm.RegisterHandler(0xC0, 0xFF, 0x0000, 0xFFFF, _prgRomHandlers, 0);

		mm.RegisterHandler(0x20, 0x3F, 0x6000, 0x7FFF, _saveRamHandlers);
		mm.RegisterHandler(0xA0, 0xBF, 0x6000, 0x7FFF, _saveRamHandlers);
	} else if(_flags & CartFlags::ExHiRom) {
		//First half is at the end
		mm.RegisterHandler(0xC0, 0xFF, 0x0000, 0xFFFF, _prgRomHandlers, 0);
		mm.RegisterHandler(0x80, 0xBF, 0x8000, 0xFFFF, _prgRomHandlers, 8); //mirror

		//Last part of the ROM is at the start
		mm.RegisterHandler(0x40, 0x7D, 0x0000, 0xFFFF, _prgRomHandlers, 0, 0x400);
		mm.RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, _prgRomHandlers, 8, 0x400); //mirror

		//Save RAM
		mm.RegisterHandler(0x20, 0x3F, 0x6000, 0x7FFF, _saveRamHandlers);
		mm.RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, _saveRamHandlers);
		mm.RegisterHandler(0xA0, 0xBF, 0x6000, 0x7FFF, _saveRamHandlers);
	}
}

void BaseCartridge::InitCoprocessor()
{
	_coprocessor.reset(NecDsp::InitCoprocessor(_coprocessorType, _console));
	_necDsp = dynamic_cast<NecDsp*>(_coprocessor.get());

	if(_coprocessorType == CoprocessorType::SA1) {
		_coprocessor.reset(new Sa1(_console));
		_sa1 = dynamic_cast<Sa1*>(_coprocessor.get());
	} else if(_coprocessorType == CoprocessorType::GSU) {
		_coprocessor.reset(new Gsu(_console, _coprocessorRamSize));
		_gsu = dynamic_cast<Gsu*>(_coprocessor.get());
	} else if(_coprocessorType == CoprocessorType::SDD1) {
		_coprocessor.reset(new Sdd1(_console));
	} else if(_coprocessorType == CoprocessorType::CX4) {
		_coprocessor.reset(new Cx4(_console));
		_cx4 = dynamic_cast<Cx4*>(_coprocessor.get());
	} else if(_coprocessorType == CoprocessorType::OBC1) {
		_coprocessor.reset(new Obc1(_console, _saveRam, _saveRamSize));
	}
}

bool BaseCartridge::MapSpecificCarts(MemoryMappings &mm)
{
	string name = GetCartName();
	string code = GetGameCode();
	if(GetCartName() == "DEZAEMON") {
		//LOROM with mirrored SRAM?
		mm.RegisterHandler(0x00, 0x7D, 0x8000, 0xFFFF, _prgRomHandlers);
		mm.RegisterHandler(0x80, 0xFF, 0x8000, 0xFFFF, _prgRomHandlers);

		mm.RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, _saveRamHandlers);
		mm.RegisterHandler(0xF0, 0xFF, 0x8000, 0xFFFF, _saveRamHandlers);

		//Mirrors
		mm.RegisterHandler(0x70, 0x7D, 0x8000, 0xFFFF, _saveRamHandlers);
		mm.RegisterHandler(0xF0, 0xFF, 0x0000, 0x7FFF, _saveRamHandlers);

		return true;
	} else if(code == "ZDBJ" || code == "ZR2J" || code == "ZSNJ") {
		//BSC-1A5M-02, BSC-1A7M-01
		//Games: Sound Novel Tsukuuru, RPG Tsukuuru, Derby Stallion 96
		mm.RegisterHandler(0x00, 0x3F, 0x8000, 0xFFFF, _prgRomHandlers);
		mm.RegisterHandler(0x80, 0x9F, 0x8000, 0xFFFF, _prgRomHandlers, 0, 0x200);
		mm.RegisterHandler(0xA0, 0xBF, 0x8000, 0xFFFF, _prgRomHandlers, 0, 0x100);
		if(_saveRamSize > 0) {
			mm.RegisterHandler(0x70, 0x7D, 0x0000, 0x7FFF, _saveRamHandlers);
			mm.RegisterHandler(0xF0, 0xFF, 0x0000, 0x7FFF, _saveRamHandlers);
		}
		return true;
	}
	return false;
}

void BaseCartridge::LoadSpc()
{
	_spcData.reset(new SpcFileData(_prgRom));
	
	//Setup a fake LOROM rom that runs STP right away to disable the main CPU
	_flags = CartFlags::LoRom;

	delete[] _prgRom;
	_prgRom = new uint8_t[0x8000];
	_prgRomSize = 0x8000;
	memset(_prgRom, 0, 0x8000);
	
	//Set reset vector to $8000
	_prgRom[0x7FFC] = 0x00;
	_prgRom[0x7FFD] = 0x80;

	//STP instruction at $8000
	_prgRom[0] = 0xDB;
}

void BaseCartridge::Serialize(Serializer &s)
{
	s.StreamArray(_saveRam, _saveRamSize);
	if(_coprocessor) {
		s.Stream(_coprocessor.get());
	}
}

string BaseCartridge::GetGameCode()
{
	string code;
	if(_cartInfo.GameCode[0] >= ' ') {
		code += _cartInfo.GameCode[0];
	}
	if(_cartInfo.GameCode[1] >= ' ') {
		code += _cartInfo.GameCode[1];
	}
	if(_cartInfo.GameCode[2] >= ' ') {
		code += _cartInfo.GameCode[2];
	}
	if(_cartInfo.GameCode[3] >= ' ') {
		code += _cartInfo.GameCode[3];
	}
	return code;
}

string BaseCartridge::GetCartName()
{
	int nameLength = 21;
	for(int i = 0; i < 21; i++) {
		if(_cartInfo.CartName[i] == 0) {
			nameLength = i;
			break;
		}
	}
	string name = string(_cartInfo.CartName, nameLength);

	size_t lastNonSpace = name.find_last_not_of(' ');
	if(lastNonSpace != string::npos) {
		return name.substr(0, lastNonSpace + 1);
	} else {
		return name;
	}
}

void BaseCartridge::DisplayCartInfo()
{
	MessageManager::Log("-----------------------------");
	MessageManager::Log("File: " + VirtualFile(_romPath).GetFileName());
	MessageManager::Log("Game: " + GetCartName());
	string gameCode = GetGameCode();
	if(!gameCode.empty()) {
		MessageManager::Log("Game code: " + gameCode);
	}
	if(_flags & CartFlags::ExHiRom) {
		MessageManager::Log("Type: ExHiROM");
	} else if(_flags & CartFlags::ExLoRom) {
		MessageManager::Log("Type: ExLoROM");
	} else if(_flags & CartFlags::HiRom) {
		MessageManager::Log("Type: HiROM");
	} else if(_flags & CartFlags::LoRom) {
		MessageManager::Log("Type: LoROM");
	}

	if(_coprocessorType != CoprocessorType::None) {
		string coProcMessage = "Coprocessor: ";
		switch(_coprocessorType) {
			case CoprocessorType::None: coProcMessage += "<none>"; break;
			case CoprocessorType::CX4: coProcMessage += "CX4"; break;
			case CoprocessorType::SDD1: coProcMessage += "S-DD1"; break;
			case CoprocessorType::DSP1: coProcMessage += "DSP1"; break;
			case CoprocessorType::DSP1B: coProcMessage += "DSP1B"; break;
			case CoprocessorType::DSP2: coProcMessage += "DSP2"; break;
			case CoprocessorType::DSP3: coProcMessage += "DSP3"; break;
			case CoprocessorType::DSP4: coProcMessage += "DSP4"; break;
			case CoprocessorType::GSU: coProcMessage += "Super FX (GSU1/2)"; break;
			case CoprocessorType::OBC1: coProcMessage += "OBC1"; break;
			case CoprocessorType::RTC: coProcMessage += "RTC"; break;
			case CoprocessorType::SA1: coProcMessage += "SA1"; break;
			case CoprocessorType::Satellaview: coProcMessage += "Satellaview"; break;
			case CoprocessorType::SPC7110: coProcMessage += "SPC7110"; break;
			case CoprocessorType::ST010: coProcMessage += "ST010"; break;
			case CoprocessorType::ST011: coProcMessage += "ST011"; break;
			case CoprocessorType::ST018: coProcMessage += "ST018"; break;
		}
		MessageManager::Log(coProcMessage);
	}

	if(_flags & CartFlags::FastRom) {
		MessageManager::Log("FastROM");
	}

	if(_flags & CartFlags::CopierHeader) {
		MessageManager::Log("Copier header found.");
	}

	MessageManager::Log("Map Mode: $" + HexUtilities::ToHex(_cartInfo.MapMode));
	MessageManager::Log("Rom Type: $" + HexUtilities::ToHex(_cartInfo.RomType));

	MessageManager::Log("File size: " + std::to_string(_prgRomSize / 1024) + " KB");
	MessageManager::Log("ROM size: " + std::to_string((0x400 << _cartInfo.RomSize) / 1024) + " KB");
	if(_saveRamSize > 0) {
		MessageManager::Log("SRAM size: " + std::to_string(_saveRamSize / 1024) + " KB" + (_hasBattery ? " (with battery)" : ""));
	}
	if(_coprocessorRamSize > 0) {
		MessageManager::Log("Coprocessor RAM size: " + std::to_string(_coprocessorRamSize / 1024) + " KB" + (_hasBattery ? " (with battery)" : ""));
	}
	MessageManager::Log("-----------------------------");
}

NecDsp* BaseCartridge::GetDsp()
{
	return _necDsp;
}

Sa1* BaseCartridge::GetSa1()
{
	return _sa1;
}

Cx4* BaseCartridge::GetCx4()
{
	return _cx4;
}

Gsu* BaseCartridge::GetGsu()
{
	return _gsu;
}

void BaseCartridge::RunCoprocessors()
{
	//These coprocessors are run at the end of the frame, or as needed
	if(_necDsp) {
		_necDsp->Run();
	}
}

BaseCoprocessor* BaseCartridge::GetCoprocessor()
{
	return _coprocessor.get();
}

vector<unique_ptr<IMemoryHandler>>& BaseCartridge::GetPrgRomHandlers()
{
	return _prgRomHandlers;
}

vector<unique_ptr<IMemoryHandler>>& BaseCartridge::GetSaveRamHandlers()
{
	return _saveRamHandlers;
}

SpcFileData* BaseCartridge::GetSpcData()
{
	return _spcData.get();
}