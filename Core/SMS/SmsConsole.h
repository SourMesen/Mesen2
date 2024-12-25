#pragma once
#include "pch.h"
#include "Shared/Interfaces/IConsole.h"
#include "Shared/SettingTypes.h"
#include "SMS/SmsTypes.h"

class Emulator;
class VirtualFile;
class BaseControlManager;

class SmsCpu;
class SmsVdp;
class SmsPsg;
class SmsFmAudio;
class SmsCart;
class SmsControlManager;
class SmsMemoryManager;

class SmsConsole final : public IConsole
{
private:
	Emulator* _emu = nullptr;
	unique_ptr<SmsCpu> _cpu;
	unique_ptr<SmsVdp> _vdp;
	unique_ptr<SmsControlManager> _controlManager;
	unique_ptr<SmsMemoryManager> _memoryManager;
	unique_ptr<SmsPsg> _psg;
	unique_ptr<SmsFmAudio> _fmAudio;
	unique_ptr<SmsCart> _cart;
	RomFormat _romFormat = RomFormat::Sms;
	SmsModel _model = SmsModel::Sms;
	ConsoleRegion _region = ConsoleRegion::Ntsc;
	string _filename;
	
	void UpdateRegion(bool forceUpdate);

public:
	static vector<string> GetSupportedExtensions() { return { ".sms", ".gg", ".sg", ".col" }; }
	static vector<string> GetSupportedSignatures() { return { }; }

	SmsConsole(Emulator* emu);
	virtual ~SmsConsole();

	Emulator* GetEmulator() { return _emu; }
	SmsCpu* GetCpu() { return _cpu.get(); }
	SmsVdp* GetVdp() { return _vdp.get(); }
	SmsMemoryManager* GetMemoryManager() { return _memoryManager.get(); }

	SmsModel GetModel() { return _model; }
	SmsRevision GetRevision();

	LoadRomResult LoadRom(VirtualFile& romFile) override;

	bool HasBios();

	void InitCart(vector<uint8_t>& romData);

	void Reset() override;
	void RunFrame() override;
	void ProcessEndOfFrame();

	void SaveBattery() override;

	bool IsPsgAudioMuted();

	BaseControlManager* GetControlManager() override;
	ConsoleRegion GetRegion() override;
	ConsoleType GetConsoleType() override;
	vector<CpuType> GetCpuTypes() override;
	RomFormat GetRomFormat() override;
	double GetFps() override;
	PpuFrameInfo GetPpuFrame() override;
	BaseVideoFilter* GetVideoFilter(bool getDefaultFilter) override;

	uint64_t GetMasterClock() override;
	uint32_t GetMasterClockRate() override;

	AudioTrackInfo GetAudioTrackInfo() override;
	void ProcessAudioPlayerAction(AudioPlayerActionParams p) override;

	void RefreshRamCheats();

	AddressInfo GetAbsoluteAddress(uint32_t relAddress);
	AddressInfo GetAbsoluteAddress(AddressInfo& relAddress) override;
	AddressInfo GetRelativeAddress(AddressInfo& absAddress, CpuType cpuType) override;
	
	SmsState GetState();
	void GetConsoleState(BaseState& state, ConsoleType consoleType) override;

	void InitializeRam(void* data, uint32_t length);

	void Serialize(Serializer& s) override;
};