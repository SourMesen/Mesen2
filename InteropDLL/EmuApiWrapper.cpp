#include "Common.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/Video/VideoDecoder.h"
#include "Core/Shared/Video/VideoRenderer.h"
#include "Core/Shared/SystemActionManager.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/SaveStateManager.h"
#include "Core/Shared/Interfaces/INotificationListener.h"
#include "Core/Shared/KeyManager.h"
#include "Core/Shared/ShortcutKeyHandler.h"
#include "Core/Shared/TimingInfo.h"
#include "Core/Shared/CheatManager.h"
#include "Core/Shared/DebuggerRequest.h"
#include "Core/Netplay/GameClient.h"
#include "Core/Netplay/GameServer.h"
#include "Utilities/ArchiveReader.h"
#include "Utilities/FolderUtilities.h"
#include "Utilities/StringUtilities.h"
#include "InteropNotificationListeners.h"

#ifdef _WIN32
	#include "Windows/Renderer.h"
	#include "Windows/SoundManager.h"
	#include "Windows/WindowsKeyManager.h"
	#include "Windows/WindowsMouseManager.h"
#elif __APPLE__
	#include "Sdl/SdlSoundManager.h"
	#include "MacOS/MacOSKeyManager.h"
	#include "MacOS/MacOSMouseManager.h"
#else
	#include "Sdl/SdlRenderer.h"
	#include "Sdl/SdlSoundManager.h"
	#include "Linux/LinuxKeyManager.h"
	#include "Linux/LinuxMouseManager.h"
#endif

#include "Shared/Video/SoftwareRenderer.h"

unique_ptr<IRenderingDevice> _renderer;
unique_ptr<IAudioDevice> _soundManager;
unique_ptr<IKeyManager> _keyManager;
unique_ptr<IMouseManager> _mouseManager;
unique_ptr<Emulator> _emu(new Emulator());
bool _softwareRenderer = false;

static void* _windowHandle = nullptr;
static void* _viewerHandle = nullptr;

static constexpr char* _buildDateTime = __DATE__ ", " __TIME__;

static InteropNotificationListeners _listeners;

struct InteropRomInfo
{
	char RomPath[2000];
	char PatchPath[2000];
	RomFormat Format;
	ConsoleType Console;
	DipSwitchInfo DipSwitches;
	CpuType CpuTypes[5];
	uint32_t CpuTypeCount;
};

extern "C" {
	DllExport bool __stdcall TestDll()
	{
		return true;
	}

	DllExport uint32_t __stdcall GetMesenVersion() { return _emu->GetSettings()->GetVersion(); }
	DllExport char* __stdcall GetMesenBuildDate() { return _buildDateTime; }

	DllExport void __stdcall InitDll()
	{
		_emu->Initialize();
		KeyManager::SetSettings(_emu->GetSettings());
	}

	DllExport void __stdcall InitializeEmu(const char* homeFolder, void *windowHandle, void *viewerHandle, bool softwareRenderer, bool noAudio, bool noVideo, bool noInput)
	{
		FolderUtilities::SetHomeFolder(homeFolder);

		if(windowHandle != nullptr && viewerHandle != nullptr) {
			_windowHandle = windowHandle;
			_viewerHandle = viewerHandle;
			_softwareRenderer = softwareRenderer;

			if(!noVideo) {
				if(softwareRenderer) {
					_renderer.reset(new SoftwareRenderer(_emu.get()));
				} else {
					#ifdef _WIN32
						_renderer.reset(new Renderer(_emu.get(), (HWND)_viewerHandle));
					#elif __APPLE__
						_renderer.reset(new SoftwareRenderer(_emu.get()));
					#else
						_renderer.reset(new SdlRenderer(_emu.get(), _viewerHandle));
					#endif
				}
			} 

			if(!noAudio) {
				#ifdef _WIN32
					_soundManager.reset(new SoundManager(_emu.get(), (HWND)_windowHandle));
				#else
					_soundManager.reset(new SdlSoundManager(_emu.get()));
				#endif
			}

			if(!noInput) {
				#ifdef _WIN32
					_keyManager.reset(new WindowsKeyManager(_emu.get(), (HWND)_windowHandle));
					_mouseManager.reset(new WindowsMouseManager());
				#elif __APPLE__
					_keyManager.reset(new MacOSKeyManager(_emu.get()));
					_mouseManager.reset(new MacOSMouseManager());
				#else 
					_keyManager.reset(new LinuxKeyManager(_emu.get()));
					_mouseManager.reset(new LinuxMouseManager(_windowHandle));
				#endif
					
				KeyManager::RegisterKeyManager(_keyManager.get());
			}
		}
	}

	DllExport void __stdcall SetExclusiveFullscreenMode(bool fullscreen, void *windowHandle)
	{
		if(_renderer) {
			_renderer->SetExclusiveFullscreenMode(fullscreen, windowHandle);
		}
	}

	DllExport bool __stdcall LoadRom(char* filename, char* patchFile)
	{
		_emu->GetGameClient()->Disconnect();
		return _emu->LoadRom((VirtualFile)filename, patchFile ? (VirtualFile)patchFile : VirtualFile());
	}

	DllExport void __stdcall AddKnownGameFolder(char* folder) { FolderUtilities::AddKnownGameFolder(folder); }

	DllExport void __stdcall GetRomInfo(InteropRomInfo &info)
	{
		RomInfo romInfo = _emu->GetRomInfo();

		string romPath = romInfo.RomFile;
		string patchPath = romInfo.PatchFile;

		memset(info.RomPath, 0, sizeof(info.RomPath));
		memset(info.PatchPath, 0, sizeof(info.PatchPath));

		memcpy(info.RomPath, romPath.c_str(), romPath.size());
		memcpy(info.PatchPath, patchPath.c_str(), patchPath.size());
		info.Format = romInfo.Format;
		info.Console = _emu->GetConsoleType();
		info.DipSwitches = romInfo.DipSwitches;

		vector<CpuType> cpuTypes = _emu->GetCpuTypes();
		info.CpuTypeCount = std::min<uint32_t>((uint32_t)cpuTypes.size(), 5);
		for(size_t i = 0; i < 5 && i < cpuTypes.size(); i++) {
			info.CpuTypes[i] = cpuTypes[i];
		}
	}

	DllExport TimingInfo __stdcall GetTimingInfo(CpuType cpuType)
	{
		return _emu->GetTimingInfo(cpuType);
	}

	DllExport void __stdcall TakeScreenshot() { _emu->GetVideoDecoder()->TakeScreenshot(); }

	DllExport void __stdcall ProcessAudioPlayerAction(AudioPlayerActionParams p) { _emu->ProcessAudioPlayerAction(p); }

	DllExport void __stdcall GetArchiveRomList(char* filename, char* outBuffer, uint32_t maxLength) { 
		std::ostringstream out;
		unique_ptr<ArchiveReader> reader = ArchiveReader::GetReader(filename);
		if(reader) {
			for(string romName : reader->GetFileList(VirtualFile::RomExtensions)) {
				out << romName << "[!|!]";
			}
		}

		StringUtilities::CopyToBuffer(out.str(), outBuffer, maxLength);
	}

	DllExport bool __stdcall IsRunning()
	{
		return _emu->IsRunning();
	}

	DllExport int32_t __stdcall GetStopCode()
	{
		return _emu->GetStopCode();
	}

	DllExport void __stdcall Stop()
	{
		_emu->GetGameClient()->Disconnect();
		_emu->Stop(true);
	}

	DllExport void __stdcall Pause()
	{
		if(!_emu->GetGameClient()->Connected()) {
			_emu->Pause();
		}
	}

	DllExport void __stdcall Resume()
	{
		if(!_emu->GetGameClient()->Connected()) {
			_emu->Resume();
		}
	}

	DllExport bool __stdcall IsPaused()
	{
		return _emu->IsPaused();
	}

	DllExport void __stdcall Release()
	{
		if(_emu) {
			_emu->Stop(true);
			_emu->Release();
		}

		_renderer.reset();
		_soundManager.reset();
		_keyManager.reset();
		_emu.reset();
	}

	DllExport INotificationListener* __stdcall RegisterNotificationCallback(NotificationListenerCallback callback)
	{
		return _listeners.RegisterNotificationCallback(callback, _emu.get());
	}

	DllExport void __stdcall UnregisterNotificationCallback(INotificationListener *listener)
	{
		_listeners.UnregisterNotificationCallback(listener);
	}

	DllExport void __stdcall DisplayMessage(char* title, char* message, char* param1) { MessageManager::DisplayMessage(title, message, param1 ? param1 : ""); }
	
	DllExport void __stdcall GetLog(char* outBuffer, uint32_t maxLength)
	{
		StringUtilities::CopyToBuffer(MessageManager::GetLog(), outBuffer, maxLength);
	}

	DllExport void __stdcall SetRendererSize(uint32_t width, uint32_t height)
	{
		if(_emu->GetVideoRenderer()) {
			_emu->GetVideoRenderer()->SetRendererSize(width, height);
		}
	}

	DllExport double __stdcall GetAspectRatio()
	{
		return _emu->GetSettings()->GetAspectRatio(_emu->GetRegion(), _emu->GetVideoDecoder()->GetBaseFrameInfo(true));
	}

	DllExport FrameInfo __stdcall GetBaseScreenSize()
	{
		if(_emu->GetVideoDecoder()) {
			return _emu->GetVideoDecoder()->GetBaseFrameInfo(true);
		}
		return { 256, 240 };
	}
	
	DllExport uint32_t __stdcall GetGameMemorySize(MemoryType type) { return _emu->GetMemory(type).Size; }

	DllExport void __stdcall ClearCheats() { _emu->GetCheatManager()->ClearCheats(); }
	DllExport void __stdcall SetCheats(CheatCode codes[], uint32_t length) { _emu->GetCheatManager()->SetCheats(codes, length); }
	DllExport bool __stdcall GetConvertedCheat(CheatCode input, InternalCheatCode& output) { return _emu->GetCheatManager()->GetConvertedCheat(input, output); }

	DllExport void __stdcall GetRomHash(HashType hashType, char* outBuffer, uint32_t maxLength)
	{
		StringUtilities::CopyToBuffer(_emu->GetHash(hashType), outBuffer, maxLength);
	}

	DllExport void __stdcall InputBarcode(uint64_t barcode, uint32_t digitCount) { _emu->InputBarcode(barcode, digitCount); }
	DllExport void __stdcall ProcessTapeRecorderAction(TapeRecorderAction action, char* filename) { _emu->ProcessTapeRecorderAction(action, filename); }

	DllExport void __stdcall ExecuteShortcut(ExecuteShortcutParams params) { _emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ExecuteShortcut, &params); }
	DllExport bool __stdcall IsShortcutAllowed(EmulatorShortcut shortcut, uint32_t shortcutParam) { return _emu->GetShortcutKeyHandler()->IsShortcutAllowed(shortcut, shortcutParam); }

	DllExport void __stdcall WriteLogEntry(char* message) { MessageManager::Log(message); }

	DllExport void __stdcall SaveState(uint32_t stateIndex) { _emu->GetSaveStateManager()->SaveState(stateIndex); }
	DllExport void __stdcall LoadState(uint32_t stateIndex) { _emu->GetSaveStateManager()->LoadState(stateIndex); }
	DllExport void __stdcall SaveStateFile(char* filepath) { _emu->GetSaveStateManager()->SaveState(filepath); }
	DllExport void __stdcall LoadStateFile(char* filepath) { _emu->GetSaveStateManager()->LoadState(filepath); }
	DllExport void __stdcall LoadRecentGame(char* filepath, bool resetGame) { _emu->GetSaveStateManager()->LoadRecentGame(filepath, resetGame); }
	DllExport int32_t __stdcall GetSaveStatePreview(char* saveStatePath, uint8_t* pngData) { return _emu->GetSaveStateManager()->GetSaveStatePreview(saveStatePath, pngData); }

	class PgoKeyManager : public IKeyManager
	{
	public:
		void RefreshState() {}
		void UpdateDevices() {}
		bool IsMouseButtonPressed(MouseButton button) { return false; }
		bool IsKeyPressed(uint16_t keyCode) { return keyCode == 10 && (_emu->GetFrameCount() % 7) <= 3; }

		vector<uint16_t> GetPressedKeys() { return {}; }
		string GetKeyName(uint16_t keyCode) { return ""; }
		uint16_t GetKeyCode(string keyName) { return 0; }

		bool SetKeyState(uint16_t scanCode, bool state) { return false; }
		void ResetKeyState() {}
		void SetDisabled(bool disabled) {}
	};

	DllExport void __stdcall PgoRunTest(vector<string> testRoms, bool enableDebugger)
	{
		FolderUtilities::SetHomeFolder("../PGOMesenHome");
		PgoKeyManager pgoKeyManager;
		KeyManager::RegisterKeyManager(&pgoKeyManager);

		for(size_t i = 0; i < testRoms.size(); i++) {
			std::cout << "Running: " << testRoms[i] << std::endl;

			KeyManager::SetSettings(_emu->GetSettings());
			_emu->Initialize();

			//Map key #10 to the start button for all consoles - this key is toggled on/off every 4 frames
			NesConfig& nesCfg = _emu->GetSettings()->GetNesConfig();
			nesCfg.Port1.Type = ControllerType::NesController;
			nesCfg.Port1.Keys.Mapping1.Start = 10;

			SnesConfig& snesCfg = _emu->GetSettings()->GetSnesConfig();
			snesCfg.Port1.Type = ControllerType::SnesController;
			snesCfg.Port1.Keys.Mapping1.Start = 10;

			GameboyConfig& gbCfg = _emu->GetSettings()->GetGameboyConfig();
			gbCfg.Model = GameboyModel::GameboyColor;
			gbCfg.Controller.Keys.Mapping1.Start = 10;

			PcEngineConfig& pceCfg = _emu->GetSettings()->GetPcEngineConfig();
			pceCfg.Port1.Type = ControllerType::PceController;
			pceCfg.Port1.Keys.Mapping1.Start = 10;

			_emu->GetSettings()->SetFlag(EmulationFlags::MaximumSpeed);
			_emu->LoadRom((VirtualFile)testRoms[i], VirtualFile());

			if(enableDebugger) {
				//turn on debugger to profile the debugger's code too
				_emu->GetDebugger(true);
			}
				
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(5000));
			std::cout << "Ran for " << _emu->GetFrameCount() << " frames" << std::endl;

			_emu->Stop(false);
			_emu->Release();
		}
	}
}