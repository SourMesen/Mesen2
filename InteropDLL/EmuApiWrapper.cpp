#include "stdafx.h"
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
#else
	#include "Linux/SdlRenderer.h"
	#include "Linux/SdlSoundManager.h"
	#include "Linux/LinuxKeyManager.h"
#endif

unique_ptr<IRenderingDevice> _renderer;
unique_ptr<IAudioDevice> _soundManager;
unique_ptr<IKeyManager> _keyManager;
unique_ptr<Emulator> _emu(new Emulator());

static void* _windowHandle = nullptr;
static void* _viewerHandle = nullptr;

static InteropNotificationListeners _listeners;

struct InteropRomInfo
{
	char RomPath[2000];
	char PatchPath[2000];
	RomFormat Format;
	ConsoleType Console;
	CpuType CpuTypes[5];
	uint32_t CpuTypeCount;
};

extern "C" {
	DllExport bool __stdcall TestDll()
	{
		return true;
	}

	DllExport uint32_t __stdcall GetMesenVersion() { return _emu->GetSettings()->GetVersion(); }

	DllExport void __stdcall InitDll()
	{
		_emu->Initialize();
		KeyManager::SetSettings(_emu->GetSettings());
	}

	DllExport void __stdcall InitializeEmu(const char* homeFolder, void *windowHandle, void *viewerHandle, bool noAudio, bool noVideo, bool noInput)
	{
		FolderUtilities::SetHomeFolder(homeFolder);

		if(windowHandle != nullptr && viewerHandle != nullptr) {
			_windowHandle = windowHandle;
			_viewerHandle = viewerHandle;

			if(!noVideo) {
				#ifdef _WIN32
					_renderer.reset(new Renderer(_emu.get(), (HWND)_viewerHandle, true));
				#else 
					_renderer.reset(new SdlRenderer(_emu.get(), _viewerHandle, true));
				#endif
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
				#else 
					_keyManager.reset(new LinuxKeyManager(_emu.get()));
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

		vector<CpuType> cpuTypes = _emu->GetCpuTypes();
		info.CpuTypeCount = std::min<uint32_t>((uint32_t)cpuTypes.size(), 5);
		for(size_t i = 0; i < 5 && i < cpuTypes.size(); i++) {
			info.CpuTypes[i] = cpuTypes[i];
		}
	}

	DllExport TimingInfo __stdcall GetTimingInfo()
	{
		return _emu->GetTimingInfo();
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
	
	DllExport void __stdcall ClearCheats() { _emu->GetCheatManager()->ClearCheats(); }
	DllExport void __stdcall SetCheats(CheatCode codes[], uint32_t length) { _emu->GetCheatManager()->SetCheats(codes, length); }
	DllExport bool __stdcall GetConvertedCheat(CheatCode input, InternalCheatCode& output) { return _emu->GetCheatManager()->GetConvertedCheat(input, output); }

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

	DllExport void __stdcall PgoRunTest(vector<string> testRoms, bool enableDebugger)
	{
		FolderUtilities::SetHomeFolder("../PGOMesenHome");

		for(size_t i = 0; i < testRoms.size(); i++) {
			std::cout << "Running: " << testRoms[i] << std::endl;

			KeyManager::SetSettings(_emu->GetSettings());
			_emu->Initialize();
			GameboyConfig cfg = _emu->GetSettings()->GetGameboyConfig();
			cfg.Model = GameboyModel::GameboyColor;
			_emu->GetSettings()->SetGameboyConfig(cfg);
			_emu->GetSettings()->SetFlag(EmulationFlags::MaximumSpeed);
			_emu->LoadRom((VirtualFile)testRoms[i], VirtualFile());

			if(enableDebugger) {
				//turn on debugger to profile the debugger's code too
				_emu->GetDebugger(true);
			}
				
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(5000));
			_emu->Stop(false);
			_emu->Release();
		}
	}
}