#include "stdafx.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"
#include "Core/Shared/Video/VideoDecoder.h"
#include "Core/Shared/Interfaces/IControlManager.h"
#include "Core/Shared/SystemActionManager.h"
#include "Core/Shared/MessageManager.h"
#include "Core/Shared/SaveStateManager.h"
#include "Core/Shared/Interfaces/INotificationListener.h"
#include "Core/Shared/KeyManager.h"
#include "Core/Shared/ShortcutKeyHandler.h"
#include "Core/Shared/CheatManager.h"
#include "Core/Netplay/GameClient.h"
#include "Core/Netplay/GameServer.h"
#include "Utilities/ArchiveReader.h"
#include "Utilities/FolderUtilities.h"
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
unique_ptr<ShortcutKeyHandler> _shortcutKeyHandler;
shared_ptr<Emulator> _emu;

static void* _windowHandle = nullptr;
static void* _viewerHandle = nullptr;
static string _returnString;
static string _logString;
static InteropNotificationListeners _listeners;

struct InteropRomInfo
{
	const char* RomPath;
	const char* PatchPath;
	RomFormat Format;
	//CoprocessorType Coprocessor;
	//SnesCartInformation Header;
	char Sha1[40];
};

static string _romPath;
static string _patchPath;

extern "C" {
	DllExport bool __stdcall TestDll()
	{
		return true;
	}

	DllExport uint32_t __stdcall GetMesenVersion() { return _emu->GetSettings()->GetVersion(); }

	DllExport void __stdcall InitDll()
	{
		_emu.reset(new Emulator());
		KeyManager::SetSettings(_emu->GetSettings());
	}

	DllExport void __stdcall InitializeEmu(const char* homeFolder, void *windowHandle, void *viewerHandle, bool noAudio, bool noVideo, bool noInput)
	{
		_emu->Initialize();

		FolderUtilities::SetHomeFolder(homeFolder);
		_shortcutKeyHandler.reset(new ShortcutKeyHandler(_emu));

		if(windowHandle != nullptr && viewerHandle != nullptr) {
			_windowHandle = windowHandle;
			_viewerHandle = viewerHandle;

			if(!noVideo) {
				#ifdef _WIN32
					_renderer.reset(new Renderer(_emu, (HWND)_viewerHandle, true));
				#else 
					_renderer.reset(new SdlRenderer(_emu, _viewerHandle, true));
				#endif
			} 

			if(!noAudio) {
				#ifdef _WIN32
					_soundManager.reset(new SoundManager(_emu, (HWND)_windowHandle));
				#else
					_soundManager.reset(new SdlSoundManager(_emu));
				#endif
			}

			if(!noInput) {
				#ifdef _WIN32
					_keyManager.reset(new WindowsKeyManager(_emu, (HWND)_windowHandle));
				#else 
					_keyManager.reset(new LinuxKeyManager(_emu));
				#endif				
					
				KeyManager::RegisterKeyManager(_keyManager.get());
			}
		}
	}

	DllExport void __stdcall SetFullscreenMode(bool fullscreen, void *windowHandle, uint32_t monitorWidth, uint32_t monitorHeight)
	{
		if(_renderer) {
			_renderer->SetFullscreenMode(fullscreen, windowHandle, monitorWidth, monitorHeight);
		}
	}

	DllExport bool __stdcall LoadRom(char* filename, char* patchFile)
	{
		GameClient::Disconnect();
		return _emu->LoadRom((VirtualFile)filename, patchFile ? (VirtualFile)patchFile : VirtualFile());
	}

	DllExport void __stdcall AddKnownGameFolder(char* folder) { FolderUtilities::AddKnownGameFolder(folder); }

	DllExport void __stdcall GetRomInfo(InteropRomInfo &info)
	{
		RomInfo romInfo = _emu->GetRomInfo();
		string sha1 = _emu->GetHash(HashType::Sha1);

		_romPath = romInfo.RomFile;
		_patchPath = romInfo.PatchFile;

		info.RomPath = _romPath.c_str();
		info.PatchPath = _patchPath.c_str();
		info.Format = romInfo.Format;
		//TODO
		//info.Header = romInfo.Header;
		//info.Coprocessor = romInfo.Coprocessor;

		memcpy(info.Sha1, sha1.c_str(), sha1.size());
	}
	
	DllExport void __stdcall TakeScreenshot() { _emu->GetVideoDecoder()->TakeScreenshot(); }

	DllExport void __stdcall ProcessAudioPlayerAction(AudioPlayerActionParams p) { _emu->ProcessAudioPlayerAction(p); }

	DllExport const char* __stdcall GetArchiveRomList(char* filename) { 
		std::ostringstream out;
		shared_ptr<ArchiveReader> reader = ArchiveReader::GetReader(filename);
		if(reader) {
			for(string romName : reader->GetFileList(VirtualFile::RomExtensions)) {
				out << romName << "[!|!]";
			}
		}
		_returnString = out.str();
		return _returnString.c_str();
	}

	DllExport bool __stdcall IsRunning()
	{
		return _emu->IsRunning();
	}

	DllExport void __stdcall Stop()
	{
		GameClient::Disconnect();
		_emu->Stop(true);
	}

	DllExport void __stdcall Pause()
	{
		if(!GameClient::Connected()) {
			_emu->Pause();
		}
	}

	DllExport void __stdcall Resume()
	{
		if(!GameClient::Connected()) {
			_emu->Resume();
		}
	}

	DllExport bool __stdcall IsPaused()
	{
		shared_ptr<Emulator> emu = _emu;
		if(emu) {
			return emu->IsPaused();
		}
		return true;
	}

	DllExport void __stdcall Reset()
	{
		if(!GameClient::Connected()) {
			_emu->GetSystemActionManager()->Reset();
		}
	}

	DllExport void __stdcall PowerCycle()
	{
		if(!GameClient::Connected()) {
			_emu->GetSystemActionManager()->PowerCycle();
		}
	}

	DllExport void __stdcall ReloadRom()
	{
		if(!GameClient::Connected()) {
			_emu->ReloadRom(false);
		}
	}

	DllExport void __stdcall Release()
	{
		GameClient::Disconnect();
		GameServer::StopServer();

		_shortcutKeyHandler.reset();
		
		_emu->Stop(true);
		
		_emu->Release();
		_emu.reset();

		_renderer.reset();
		_soundManager.reset();
		_keyManager.reset();
	}

	DllExport INotificationListener* __stdcall RegisterNotificationCallback(NotificationListenerCallback callback)
	{
		return _listeners.RegisterNotificationCallback(callback, _emu);
	}

	DllExport void __stdcall UnregisterNotificationCallback(INotificationListener *listener)
	{
		_listeners.UnregisterNotificationCallback(listener);
	}

	DllExport void __stdcall DisplayMessage(char* title, char* message, char* param1) { MessageManager::DisplayMessage(title, message, param1 ? param1 : ""); }
	DllExport const char* __stdcall GetLog()
	{
		_logString = MessageManager::GetLog();
		return _logString.c_str();
	}

	DllExport double __stdcall GetAspectRatio()
	{
		double ratio = _emu->GetSettings()->GetAspectRatio(_emu->GetRegion());
		if(ratio == 0.0) {
			FrameInfo frame = _emu->GetVideoDecoder()->GetFrameInfo();
			return (double)frame.Width / frame.Height;
		}
		return ratio;
	}
	
	DllExport void __stdcall ClearCheats() { _emu->GetCheatManager()->ClearCheats(); }
	DllExport void __stdcall SetCheats(uint32_t codes[], uint32_t length) { _emu->GetCheatManager()->SetCheats(codes, length); }

	DllExport void __stdcall ExecuteShortcut(ExecuteShortcutParams params) { _emu->GetNotificationManager()->SendNotification(ConsoleNotificationType::ExecuteShortcut, &params); }
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

			_emu.reset(new Emulator());
			KeyManager::SetSettings(_emu->GetSettings());
			_emu->Initialize();
			GameboyConfig cfg = _emu->GetSettings()->GetGameboyConfig();
			cfg.Model = GameboyModel::GameboyColor;
			_emu->GetSettings()->SetGameboyConfig(cfg);
			_emu->LoadRom((VirtualFile)testRoms[i], VirtualFile());

			if(enableDebugger) {
				//turn on debugger to profile the debugger's code too
				_emu->GetDebugger();
			}
				
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(5000));
			_emu->Stop(false);
			_emu->Release();
		}
	}
}