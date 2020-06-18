#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/EmuSettings.h"
#include "../Core/VideoDecoder.h"
#include "../Core/ControlManager.h"
#include "../Core/BaseCartridge.h"
#include "../Core/SystemActionManager.h"
#include "../Core/MessageManager.h"
#include "../Core/SaveStateManager.h"
#include "../Core/INotificationListener.h"
#include "../Core/KeyManager.h"
#include "../Core/ShortcutKeyHandler.h"
#include "../Core/CheatManager.h"
#include "../Core/GameClient.h"
#include "../Core/GameServer.h"
#include "../Utilities/ArchiveReader.h"
#include "../Utilities/FolderUtilities.h"
#include "InteropNotificationListeners.h"

#ifdef _WIN32
	#include "../Windows/Renderer.h"
	#include "../Windows/SoundManager.h"
	#include "../Windows/WindowsKeyManager.h"
#else
	#include "../Linux/SdlRenderer.h"
	#include "../Linux/SdlSoundManager.h"
	#include "../Linux/LinuxKeyManager.h"
#endif

unique_ptr<IRenderingDevice> _renderer;
unique_ptr<IAudioDevice> _soundManager;
unique_ptr<IKeyManager> _keyManager;
unique_ptr<ShortcutKeyHandler> _shortcutKeyHandler;

void* _windowHandle = nullptr;
void* _viewerHandle = nullptr;
string _returnString;
string _logString;
shared_ptr<Console> _console;
InteropNotificationListeners _listeners;

struct InteropRomInfo
{
	const char* RomPath;
	const char* PatchPath;
	CoprocessorType Coprocessor;
	SnesCartInformation Header;
	char Sha1[40];
};

string _romPath;
string _patchPath;

extern "C" {
	DllExport bool __stdcall TestDll()
	{
		return true;
	}

	DllExport uint32_t __stdcall GetMesenVersion() { return _console->GetSettings()->GetVersion(); }

	DllExport void __stdcall InitDll()
	{
		_console.reset(new Console());
		KeyManager::SetSettings(_console->GetSettings().get());
	}

	DllExport void __stdcall InitializeEmu(const char* homeFolder, void *windowHandle, void *viewerHandle, bool noAudio, bool noVideo, bool noInput)
	{
		_console->Initialize();

		FolderUtilities::SetHomeFolder(homeFolder);
		_shortcutKeyHandler.reset(new ShortcutKeyHandler(_console));

		if(windowHandle != nullptr && viewerHandle != nullptr) {
			_windowHandle = windowHandle;
			_viewerHandle = viewerHandle;

			if(!noVideo) {
				#ifdef _WIN32
					_renderer.reset(new Renderer(_console, (HWND)_viewerHandle, true));
				#else 
					_renderer.reset(new SdlRenderer(_console, _viewerHandle, true));
				#endif
			} 

			if(!noAudio) {
				#ifdef _WIN32
					_soundManager.reset(new SoundManager(_console, (HWND)_windowHandle));
				#else
					_soundManager.reset(new SdlSoundManager(_console));
				#endif
			}

			if(!noInput) {
				#ifdef _WIN32
					_keyManager.reset(new WindowsKeyManager(_console, (HWND)_windowHandle));
				#else 
					_keyManager.reset(new LinuxKeyManager(_console));
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
		return _console->LoadRom((VirtualFile)filename, patchFile ? (VirtualFile)patchFile : VirtualFile());
	}

	DllExport void __stdcall AddKnownGameFolder(char* folder) { FolderUtilities::AddKnownGameFolder(folder); }

	DllExport void __stdcall GetRomInfo(InteropRomInfo &info)
	{
		RomInfo romInfo = {};
		string sha1;
		if(_console->GetCartridge()) {
			romInfo = _console->GetCartridge()->GetRomInfo();
			sha1 = _console->GetCartridge()->GetSha1Hash();
		}

		_romPath = romInfo.RomFile;
		_patchPath = romInfo.PatchFile;

		info.Header = romInfo.Header;
		info.RomPath = _romPath.c_str();
		info.PatchPath = _patchPath.c_str();
		info.Coprocessor = romInfo.Coprocessor;

		memcpy(info.Sha1, sha1.c_str(), sha1.size());
	}
	
	DllExport void __stdcall TakeScreenshot() { _console->GetVideoDecoder()->TakeScreenshot(); }

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
		return _console->IsRunning();
	}

	DllExport void __stdcall Stop()
	{
		GameClient::Disconnect();
		_console->Stop(true);
	}

	DllExport void __stdcall Pause()
	{
		if(!GameClient::Connected()) {
			_console->Pause();
		}
	}

	DllExport void __stdcall Resume()
	{
		if(!GameClient::Connected()) {
			_console->Resume();
		}
	}

	DllExport bool __stdcall IsPaused()
	{
		shared_ptr<Console> console = _console;
		if(console) {
			return console->IsPaused();
		}
		return true;
	}

	DllExport void __stdcall Reset()
	{
		if(!GameClient::Connected()) {
			_console->GetControlManager()->GetSystemActionManager()->Reset();
		}
	}

	DllExport void __stdcall PowerCycle()
	{
		if(!GameClient::Connected()) {
			_console->GetControlManager()->GetSystemActionManager()->PowerCycle();
		}
	}

	DllExport void __stdcall ReloadRom()
	{
		if(!GameClient::Connected()) {
			_console->ReloadRom(false);
		}
	}

	DllExport void __stdcall Release()
	{
		GameClient::Disconnect();
		GameServer::StopServer();

		_shortcutKeyHandler.reset();
		
		_console->Stop(true);
		
		_console->Release();
		_console.reset();			

		_renderer.reset();
		_soundManager.reset();
		_keyManager.reset();
	}

	DllExport INotificationListener* __stdcall RegisterNotificationCallback(NotificationListenerCallback callback)
	{
		return _listeners.RegisterNotificationCallback(callback, _console);
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

	DllExport ScreenSize __stdcall GetScreenSize(bool ignoreScale)
	{
		return _console->GetVideoDecoder()->GetScreenSize(ignoreScale);
	}
	
	DllExport void __stdcall ClearCheats() { _console->GetCheatManager()->ClearCheats(); }
	DllExport void __stdcall SetCheats(uint32_t codes[], uint32_t length) { _console->GetCheatManager()->SetCheats(codes, length); }

	DllExport void __stdcall WriteLogEntry(char* message) { MessageManager::Log(message); }

	DllExport void __stdcall SaveState(uint32_t stateIndex) { _console->GetSaveStateManager()->SaveState(stateIndex); }
	DllExport void __stdcall LoadState(uint32_t stateIndex) { _console->GetSaveStateManager()->LoadState(stateIndex); }
	DllExport void __stdcall SaveStateFile(char* filepath) { _console->GetSaveStateManager()->SaveState(filepath); }
	DllExport void __stdcall LoadStateFile(char* filepath) { _console->GetSaveStateManager()->LoadState(filepath); }
	DllExport void __stdcall LoadRecentGame(char* filepath, bool resetGame) { _console->GetSaveStateManager()->LoadRecentGame(filepath, resetGame); }
	DllExport int32_t __stdcall GetSaveStatePreview(char* saveStatePath, uint8_t* pngData) { return _console->GetSaveStateManager()->GetSaveStatePreview(saveStatePath, pngData); }

	DllExport void __stdcall PgoRunTest(vector<string> testRoms, bool enableDebugger)
	{
		FolderUtilities::SetHomeFolder("../PGOMesenHome");

		for(size_t i = 0; i < testRoms.size(); i++) {
			std::cout << "Running: " << testRoms[i] << std::endl;

			_console.reset(new Console());
			KeyManager::SetSettings(_console->GetSettings().get());
			_console->Initialize();
			EmulationConfig cfg = _console->GetSettings()->GetEmulationConfig();
			cfg.GbModel = GameboyModel::GameboyColor;
			_console->GetSettings()->SetEmulationConfig(cfg);
			_console->LoadRom((VirtualFile)testRoms[i], VirtualFile());

			if(enableDebugger) {
				//turn on debugger to profile the debugger's code too
				_console->GetDebugger();
			}
				
			std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(5000));
			_console->Stop(false);
			_console->Release();
		}
	}
}