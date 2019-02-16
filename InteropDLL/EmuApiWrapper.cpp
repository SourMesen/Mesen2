#include "stdafx.h"
#include "../Core/Console.h"
#include "../Core/MessageManager.h"
#include "../Core/INotificationListener.h"
#include "../Core/KeyManager.h"
#include "../Utilities/ArchiveReader.h"
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
//unique_ptr<ShortcutKeyHandler> _shortcutKeyHandler;

void* _windowHandle = nullptr;
void* _viewerHandle = nullptr;
string _returnString;
string _logString;
shared_ptr<Console> _console;
InteropNotificationListeners _listeners;

namespace InteropEmu {
	extern "C" {
		DllExport bool __stdcall TestDll()
		{
			return true;
		}

		DllExport uint32_t __stdcall GetMesenVersion() { return 0x00000100; }

		DllExport void __stdcall InitDll()
		{
			_console.reset(new Console());
			_console->Initialize();
		}

		DllExport void __stdcall InitializeEmu(const char* homeFolder, void *windowHandle, void *viewerHandle, bool noAudio, bool noVideo, bool noInput)
		{
			FolderUtilities::SetHomeFolder(homeFolder);
			//_shortcutKeyHandler.reset(new ShortcutKeyHandler(_console));

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

		DllExport void __stdcall LoadRom(char* filename, char* patchFile) { _console->LoadRom((VirtualFile)filename, (VirtualFile)patchFile); }
		//DllExport void __stdcall AddKnownGameFolder(char* folder) { FolderUtilities::AddKnownGameFolder(folder); }
		//DllExport void __stdcall SetFolderOverrides(char* saveFolder, char* saveStateFolder, char* screenshotFolder) { FolderUtilities::SetFolderOverrides(saveFolder, saveStateFolder, screenshotFolder); }

		DllExport const char* __stdcall GetArchiveRomList(char* filename) { 
			std::ostringstream out;
			shared_ptr<ArchiveReader> reader = ArchiveReader::GetReader(filename);
			if(reader) {
				for(string romName : reader->GetFileList({ ".sfc" })) {
					out << romName << "[!|!]";
				}
			}
			_returnString = out.str();
			return _returnString.c_str();
		}

		DllExport void __stdcall SetMousePosition(double x, double y) { KeyManager::SetMousePosition(x, y); }
		DllExport void __stdcall SetMouseMovement(int16_t x, int16_t y) { KeyManager::SetMouseMovement(x, y); }

		DllExport void __stdcall UpdateInputDevices() { if(_keyManager) { _keyManager->UpdateDevices(); } }
		DllExport void __stdcall GetPressedKeys(uint32_t *keyBuffer) { 
			vector<uint32_t> pressedKeys = KeyManager::GetPressedKeys();
			for(size_t i = 0; i < pressedKeys.size() && i < 3; i++) {
				keyBuffer[i] = pressedKeys[i];
			}
		}
		DllExport void __stdcall DisableAllKeys(bool disabled) {
			if(_keyManager) {
				_keyManager->SetDisabled(disabled);
			}
		}
		DllExport void __stdcall SetKeyState(int32_t scanCode, bool state) { 
			if(_keyManager) { 
				_keyManager->SetKeyState(scanCode, state); 
				//_shortcutKeyHandler->ProcessKeys();
			} 
		}
		DllExport void __stdcall ResetKeyState() { if(_keyManager) { _keyManager->ResetKeyState(); } }
		DllExport const char* __stdcall GetKeyName(uint32_t keyCode) 
		{
			_returnString = KeyManager::GetKeyName(keyCode);
			return _returnString.c_str();
		}
		DllExport uint32_t __stdcall GetKeyCode(char* keyName) { 
			if(keyName) {
				return KeyManager::GetKeyCode(keyName);
			} else {
				return 0;
			}
		}

		DllExport void __stdcall Run()
		{
			if(_console) {
				_console->Run();
			}
		}

		DllExport void __stdcall Stop()
		{
			if(_console) {
				_console->Stop();
			}
		}
		
		DllExport void __stdcall Release()
		{
			//_shortcutKeyHandler.reset();
			
			_console->Stop();

			_renderer.reset();
			_soundManager.reset();
			_keyManager.reset();

			//_console->Release(true);
			_console.reset();
			
			//_shortcutKeyHandler.reset();
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

		DllExport void __stdcall WriteLogEntry(char* message) { MessageManager::Log(message); }
	}
}