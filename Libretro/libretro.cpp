#include <string>
#include <sstream>
#include <algorithm>
#include "LibretroRenderer.h"
#include "LibretroSoundManager.h"
#include "LibretroKeyManager.h"
#include "LibretroMessageManager.h"
#include "libretro.h"
#include "../Core/Console.h"
#include "../Core/BaseCartridge.h"
#include "../Core/MemoryManager.h"
#include "../Core/VideoDecoder.h"
#include "../Core/VideoRenderer.h"
#include "../Core/EmuSettings.h"
#include "../Core/SaveStateManager.h"
#include "../Utilities/snes_ntsc.h"
#include "../Utilities/FolderUtilities.h"
#include "../Utilities/HexUtilities.h"

#define DEVICE_AUTO               RETRO_DEVICE_JOYPAD
#define DEVICE_GAMEPAD            RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_JOYPAD, 0)
#define DEVICE_SNESMOUSE          RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 2)

static retro_log_printf_t logCallback = nullptr;
static retro_environment_t retroEnv = nullptr;
static unsigned _inputDevices[5] = { DEVICE_GAMEPAD, DEVICE_GAMEPAD, DEVICE_AUTO, DEVICE_AUTO, DEVICE_AUTO };
static string _mesenVersion = "";
static int32_t _saveStateSize = -1;

static std::shared_ptr<Console> _console;
static std::unique_ptr<LibretroRenderer> _renderer;
static std::unique_ptr<LibretroSoundManager> _soundManager;
static std::unique_ptr<LibretroKeyManager> _keyManager;
static std::unique_ptr<LibretroMessageManager> _messageManager;

static constexpr char* MesenNtscFilter = "mesen-s_ntsc_filter";
static constexpr char* MesenRegion = "mesen-s_region";
static constexpr char* MesenAspectRatio = "mesen-s_aspect_ratio";
static constexpr char* MesenOverscanVertical = "mesen-s_overscan_vertical";
static constexpr char* MesenOverscanHorizontal = "mesen-s_overscan_horizontal";
static constexpr char* MesenRamState = "mesen-s_ramstate";

extern "C" {
	void logMessage(retro_log_level level, const char* message)
	{
		if(logCallback) {
			logCallback(level, message);
		}
	}

	RETRO_API unsigned retro_api_version()
	{
		return RETRO_API_VERSION;
	}

	RETRO_API void retro_init()
	{
		struct retro_log_callback log;
		if(retroEnv(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log)) {
			logCallback = log.log;
		} else {
			logCallback = nullptr;
		}

		_console.reset(new Console());
		_console->Initialize();

		_renderer.reset(new LibretroRenderer(_console, retroEnv));
		_soundManager.reset(new LibretroSoundManager(_console));
		_keyManager.reset(new LibretroKeyManager(_console));
		_messageManager.reset(new LibretroMessageManager(logCallback, retroEnv));

		AudioConfig audioConfig = _console->GetSettings()->GetAudioConfig();
		audioConfig.SampleRate = 32000;
		_console->GetSettings()->SetAudioConfig(audioConfig);

		PreferencesConfig preferences = _console->GetSettings()->GetPreferences();
		preferences.RewindBufferSize = 0;
		_console->GetSettings()->SetPreferences(preferences);
	}

	RETRO_API void retro_deinit()
	{
		_renderer.reset();
		_soundManager.reset();
		_keyManager.reset();
		_messageManager.reset();

		//_console->SaveBatteries();
		_console->Release();
		_console.reset();
	}

	RETRO_API void retro_set_environment(retro_environment_t env)
	{
		retroEnv = env;

		static constexpr struct retro_variable vars[] = {
			{ MesenNtscFilter, "NTSC filter; Disabled|Composite (Blargg)|S-Video (Blargg)|RGB (Blargg)|Monochrome (Blargg)" },
			{ MesenRegion, "Region; Auto|NTSC|PAL" },
			{ MesenOverscanVertical, "Vertical Overscan; None|8px|16px" },
			{ MesenOverscanHorizontal, "Horizontal Overscan; None|8px|16px" },
			{ MesenAspectRatio, "Aspect Ratio; Auto|No Stretching|NTSC|PAL|4:3|16:9" },
			{ MesenRamState, "Default power-on state for RAM; Random Values (Default)|All 0s|All 1s" },
			{ NULL, NULL },
		};

		static constexpr struct retro_controller_description pads1[] = {
			//{ "Auto", DEVICE_AUTO },
			{ "SNES Controller", DEVICE_GAMEPAD },
			{ "SNES Mouse", DEVICE_SNESMOUSE },
			{ NULL, 0 },
		};

		static constexpr struct retro_controller_description pads2[] = {
			//{ "Auto", DEVICE_AUTO },
			{ "SNES Controller", DEVICE_GAMEPAD },
			{ "SNES Mouse", DEVICE_SNESMOUSE },
			{ NULL, 0 },
		};
		
		static constexpr struct retro_controller_info ports[] = {
			{ pads1, 2 },
			{ pads2, 2 },
			{ 0 },
		};

		retroEnv(RETRO_ENVIRONMENT_SET_VARIABLES, (void*)vars);
		retroEnv(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, (void*)ports);
	}

	RETRO_API void retro_set_video_refresh(retro_video_refresh_t sendFrame)
	{
		_renderer->SetVideoCallback(sendFrame);
	}

	RETRO_API void retro_set_audio_sample(retro_audio_sample_t sendAudioSample)
	{
		_soundManager->SetSendAudioSample(sendAudioSample);
	}

	RETRO_API void retro_set_audio_sample_batch(retro_audio_sample_batch_t audioSampleBatch)
	{
	}

	RETRO_API void retro_set_input_poll(retro_input_poll_t pollInput)
	{	
		_keyManager->SetPollInput(pollInput);
	}

	RETRO_API void retro_set_input_state(retro_input_state_t getInputState)
	{
		_keyManager->SetGetInputState(getInputState);
	}

	RETRO_API void retro_reset()
	{
		_console->Reset();
	}

	bool readVariable(const char* key, retro_variable &var)
	{
		var.key = key;
		var.value = nullptr;
		if(retroEnv(RETRO_ENVIRONMENT_GET_VARIABLE, &var) && var.value != nullptr) {
			return true;
		}
		return false;
	}

	void update_settings()
	{
		struct retro_variable var = { };
		VideoConfig video = _console->GetSettings()->GetVideoConfig();
		EmulationConfig emulation = _console->GetSettings()->GetEmulationConfig();
		InputConfig input = _console->GetSettings()->GetInputConfig();
		video.Brightness = 0;
		video.Contrast = 0;
		video.Hue = 0;
		video.Saturation = 0;
		video.ScanlineIntensity = 0;

		if(readVariable(MesenNtscFilter, var)) {
			string value = string(var.value);
			if(value == "Disabled") {
				video.VideoFilter = VideoFilterType::None;
			} else if(value == "Composite (Blargg)") {
				video.VideoFilter = VideoFilterType::NTSC;
				video.NtscArtifacts = 0;
				video.NtscBleed = 0;
				video.NtscFringing = 0;
				video.NtscGamma = 0;
				video.NtscResolution = 0;
				video.NtscSharpness = 0;
				video.NtscMergeFields = false;
			} else if(value == "S-Video (Blargg)") {
				video.VideoFilter = VideoFilterType::NTSC;
				video.NtscArtifacts = -1.0;
				video.NtscBleed = 0;
				video.NtscFringing = -1.0;
				video.NtscGamma = 0;
				video.NtscResolution = 0.2;
				video.NtscSharpness = 0.2;
				video.NtscMergeFields = false;
			} else if(value == "RGB (Blargg)") {
				video.VideoFilter = VideoFilterType::NTSC;
				video.NtscArtifacts = -1.0;
				video.NtscBleed = -1.0;
				video.NtscFringing = -1.0;
				video.NtscGamma = 0;
				video.NtscResolution = 0.7;
				video.NtscSharpness = 0.2;
				video.NtscMergeFields = false;
			} else if(value == "Monochrome (Blargg)") {
				video.VideoFilter = VideoFilterType::NTSC;
				video.Saturation = -1.0;
				video.NtscArtifacts = -0.2;
				video.NtscBleed = -1.0;
				video.NtscFringing = -0.2;
				video.NtscGamma = 0;
				video.NtscResolution = 0.2;
				video.NtscSharpness = 0.2;
				video.NtscMergeFields = false;
			}
		}

		int overscanHorizontal = 0;
		int overscanVertical = 0;		
		if(readVariable(MesenOverscanHorizontal, var)) {
			string value = string(var.value);
			if(value == "8px") {
				overscanHorizontal = 8;
			} else if(value == "16px") {
				overscanHorizontal = 16;
			}
		}

		if(readVariable(MesenOverscanVertical, var)) {
			string value = string(var.value);
			if(value == "8px") {
				overscanVertical = 8;
			} else if(value == "16px") {
				overscanVertical = 16;
			}
		}
		
		video.OverscanLeft = overscanHorizontal;
		video.OverscanRight = overscanHorizontal;
		video.OverscanTop = std::max(0, overscanVertical - 1);
		video.OverscanBottom = overscanVertical;

		if(readVariable(MesenAspectRatio, var)) {
			string value = string(var.value);
			if(value == "Auto") {
				video.AspectRatio = VideoAspectRatio::Auto;
			} else if(value == "No Stretching") {
				video.AspectRatio = VideoAspectRatio::NoStretching;
			} else if(value == "NTSC") {
				video.AspectRatio = VideoAspectRatio::NTSC;
			} else if(value == "PAL") {
				video.AspectRatio = VideoAspectRatio::PAL;
			} else if(value == "4:3") {
				video.AspectRatio = VideoAspectRatio::Standard;
			} else if(value == "16:9") {
				video.AspectRatio = VideoAspectRatio::Widescreen;
			}
		}

		if(readVariable(MesenRegion, var)) {
			string value = string(var.value);
			if(value == "Auto") {
				emulation.Region = ConsoleRegion::Auto;
			} else if(value == "NTSC") {
				emulation.Region = ConsoleRegion::Ntsc;
			} else if(value == "PAL") {
				emulation.Region = ConsoleRegion::Pal;
			}
		}

		if(readVariable(MesenRamState, var)) {
			string value = string(var.value);
			if(value == "Random Values (Default)") {
				emulation.RamPowerOnState = RamState::Random;
			} else if(value == "All 0s ") {
				emulation.RamPowerOnState = RamState::AllZeros;
			} else if(value == "All 1s") {
				emulation.RamPowerOnState = RamState::AllOnes;
			}
		}

		auto getKeyCode = [=](int port, int retroKey) {
			return (port << 8) | (retroKey + 1);
		};

		auto getKeyBindings = [=](int port) {
			KeyMappingSet keyMappings;
			keyMappings.TurboSpeed = 0;
			keyMappings.Mapping1.L = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_L);
			keyMappings.Mapping1.R = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_R);
			keyMappings.Mapping1.A = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_A);
			keyMappings.Mapping1.B = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_B);
			keyMappings.Mapping1.X = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_X);
			keyMappings.Mapping1.Y = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_Y);

			keyMappings.Mapping1.Start = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_START);
			keyMappings.Mapping1.Select = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_SELECT);

			keyMappings.Mapping1.Up = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_UP);
			keyMappings.Mapping1.Down = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_DOWN);
			keyMappings.Mapping1.Left = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_LEFT);
			keyMappings.Mapping1.Right = getKeyCode(port, RETRO_DEVICE_ID_JOYPAD_RIGHT);

			return keyMappings;
		};

		input.Controllers[0].Keys = getKeyBindings(0);
		input.Controllers[1].Keys = getKeyBindings(1);
		input.Controllers[2].Keys = getKeyBindings(2);
		input.Controllers[3].Keys = getKeyBindings(3);

		_console->GetSettings()->SetVideoConfig(video);
		_console->GetSettings()->SetEmulationConfig(emulation);
		_console->GetSettings()->SetInputConfig(input);

		retro_system_av_info avInfo = {};
		_renderer->GetSystemAudioVideoInfo(avInfo);
		retroEnv(RETRO_ENVIRONMENT_SET_GEOMETRY, &avInfo);
	}

	RETRO_API void retro_run()
	{
		bool updated = false;
		if(retroEnv(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &updated) && updated) {
			update_settings();
		}

		_console->RunSingleFrame();

		if(updated) {
			//Update geometry after running the frame, in case the console's region changed (affects "auto" aspect ratio)
			retro_system_av_info avInfo = {};
			_renderer->GetSystemAudioVideoInfo(avInfo);
			retroEnv(RETRO_ENVIRONMENT_SET_GEOMETRY, &avInfo);
		}
	}

	RETRO_API size_t retro_serialize_size()
	{
		return _saveStateSize;
	}

	RETRO_API bool retro_serialize(void *data, size_t size)
	{
		std::stringstream ss;
		_console->Serialize(ss);
		
		string saveStateData = ss.str();
		memset(data, 0, size);
		memcpy(data, saveStateData.c_str(), std::min(size, saveStateData.size()));

		return true;
	}

	RETRO_API bool retro_unserialize(const void *data, size_t size)
	{
		std::stringstream ss;
		ss.write((const char*)data, size);
		_console->Deserialize(ss, SaveStateManager::FileFormatVersion);
		return true;
	}

	RETRO_API void retro_cheat_reset()
	{
	}

	RETRO_API void retro_cheat_set(unsigned index, bool enabled, const char *codeStr)
	{
	}

	void update_input_descriptors()
	{
		vector<retro_input_descriptor> desc;

		auto addDesc = [&desc](unsigned port, unsigned button, const char* name) {
			retro_input_descriptor d = { port, RETRO_DEVICE_JOYPAD, 0, button, name };
			desc.push_back(d);
		};

		auto setupPlayerButtons = [addDesc](int port) {
			unsigned device = _inputDevices[port];
			if(device == DEVICE_AUTO) {
				if(port <= 3) {
					switch(_console->GetSettings()->GetInputConfig().Controllers[port].Type) {
						case ControllerType::SnesController: device = DEVICE_GAMEPAD; break;
						case ControllerType::SnesMouse: device = DEVICE_SNESMOUSE; break;
						default: return;
					}
				}
			}

			if(device == DEVICE_GAMEPAD) {
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_LEFT, "D-Pad Left");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_UP, "D-Pad Up");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_DOWN, "D-Pad Down");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_RIGHT, "D-Pad Right");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_A, "A");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_B, "B");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_X, "X");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_Y, "Y");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_L, "L");
				addDesc(port, RETRO_DEVICE_ID_JOYPAD_R, "R");
			}
		};

		setupPlayerButtons(0);
		setupPlayerButtons(1);

		retro_input_descriptor end = { 0 };
		desc.push_back(end);

		retroEnv(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, desc.data());
	}

	void update_core_controllers()
	{
		InputConfig input = _console->GetSettings()->GetInputConfig();
		for(int port = 0; port < 2; port++) {
			ControllerType type = ControllerType::SnesController;
			switch(_inputDevices[port]) {
				case RETRO_DEVICE_NONE: type = ControllerType::None; break;
				case DEVICE_GAMEPAD: type = ControllerType::SnesController; break;
				case DEVICE_SNESMOUSE: type = ControllerType::SnesMouse; break;
			}
			input.Controllers[port].Type = type;
		}
		_console->GetSettings()->SetInputConfig(input);
	}
	
	void retro_set_memory_maps()
	{
	}

	RETRO_API void retro_set_controller_port_device(unsigned port, unsigned device)
	{
		if(port < 2 && _inputDevices[port] != device) {
			_inputDevices[port] = device;
			update_core_controllers();
			update_input_descriptors();
		}
	}

	RETRO_API bool retro_load_game(const struct retro_game_info *game)
	{
		char *systemFolder;
		if(!retroEnv(RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY, &systemFolder) || !systemFolder) {
			return false;
		}

		char *saveFolder;
		if(!retroEnv(RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY, &saveFolder)) {
			logMessage(RETRO_LOG_ERROR, "Could not find save directory.\n");
		}

		enum retro_pixel_format fmt = RETRO_PIXEL_FORMAT_XRGB8888;
		if(!retroEnv(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt)) {
			logMessage(RETRO_LOG_ERROR, "XRGB8888 is not supported.\n");
			return false;
		}

		//Expect the following structure:
		// /saves/*.sav
		FolderUtilities::SetHomeFolder(systemFolder);
		FolderUtilities::SetFolderOverrides(saveFolder, "", "");

		update_settings();

		//Plug in 2 standard controllers by default, game database will switch the controller types for recognized games
		/*
		_console->GetSettings()->SetControllerType(0, ControllerType::SnesController);
		_console->GetSettings()->SetControllerType(1, ControllerType::SnesController);
		_console->GetSettings()->SetControllerType(2, ControllerType::None);
		_console->GetSettings()->SetControllerType(3, ControllerType::None);*/

		VirtualFile romData(game->data, game->size, game->path);
		VirtualFile patch;
		bool result = _console->LoadRom(romData, patch);

		if(result) {
			update_core_controllers();
			update_input_descriptors();

			//Savestates in Mesen may change size over time
			//Retroarch doesn't like this for netplay or rewinding - it requires the states to always be the exact same size
			//So we need to send a large enough size to Retroarch to ensure Mesen's state will always fit within that buffer.
			std::stringstream ss;
			_console->Serialize(ss);

			//Round up to the next 1kb multiple
			_saveStateSize = ((ss.str().size() * 2) + 0x400) & ~0x3FF;
			retro_set_memory_maps();
		}

		return result;
	}

	RETRO_API bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
	{
		return false;
	}

	RETRO_API void retro_unload_game()
	{
		_console->Stop(false);
	}

	RETRO_API unsigned retro_get_region()
	{
		ConsoleRegion region = _console->GetRegion();
		return region == ConsoleRegion::Ntsc ? RETRO_REGION_NTSC : RETRO_REGION_PAL;
	}

	RETRO_API void retro_get_system_info(struct retro_system_info *info)
	{
		if(!_console) {
			_console.reset(new Console());
			_console->Initialize();
		}
		_mesenVersion = _console->GetSettings()->GetVersionString();

		info->library_name = "Mesen-S";
		info->library_version = _mesenVersion.c_str();
		info->need_fullpath = false;
		info->valid_extensions = "sfc|smc|fig|swc";
		info->block_extract = false;
	}

	RETRO_API void retro_get_system_av_info(struct retro_system_av_info *info)
	{
		_renderer->GetSystemAudioVideoInfo(*info, SNES_NTSC_OUT_WIDTH(256), 239 * 2);
	}

	RETRO_API void *retro_get_memory_data(unsigned id)
	{
		switch(id) {
			case RETRO_MEMORY_SAVE_RAM: return _console->GetCartridge()->DebugGetSaveRam();
			case RETRO_MEMORY_SYSTEM_RAM: return _console->GetMemoryManager()->DebugGetWorkRam();
		}
		return nullptr;
	}

	RETRO_API size_t retro_get_memory_size(unsigned id)
	{
		switch(id) {
			case RETRO_MEMORY_SAVE_RAM: _console->GetCartridge()->DebugGetSaveRamSize(); break;
			case RETRO_MEMORY_SYSTEM_RAM: return MemoryManager::WorkRamSize;
		}
		return 0;
	}
}
