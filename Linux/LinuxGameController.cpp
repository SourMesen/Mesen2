#include "Core/Shared/MessageManager.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"
#include "LinuxGameController.h"

#include "libevdev/libevdev.h"
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>

std::shared_ptr<LinuxGameController> LinuxGameController::GetController(Emulator* emu, int deviceID, bool logInformation)
{
	std::string deviceName = "/dev/input/event" + std::to_string(deviceID);
	struct stat buffer;   
	if(stat(deviceName.c_str(), &buffer) == 0) { 	
		int fd = open(deviceName.c_str(), O_RDWR | O_NONBLOCK);
		if(fd < 0) {
			if(logInformation) {
				MessageManager::Log("[Input] " + deviceName + "  error: " + std::to_string(errno) + " " + strerror(errno));
			}
			return nullptr;
		}
		
		libevdev* device = nullptr;
		int rc = libevdev_new_from_fd(fd, &device);
		if(rc < 0) {
			if(logInformation) {
				MessageManager::Log("[Input] " + deviceName + "  error: " + std::to_string(errno) + " " + strerror(errno));
			}
			close(fd);
			return nullptr;
		}

		if((libevdev_has_event_type(device, EV_KEY) && libevdev_has_event_code(device, EV_KEY, BTN_GAMEPAD)) ||
			(libevdev_has_event_type(device, EV_ABS) && libevdev_has_event_code(device, EV_ABS, ABS_X))) {
			MessageManager::Log(std::string("[Input Connected] Name: ") + libevdev_get_name(device) + " Vendor: " + std::to_string(libevdev_get_id_vendor(device)) + " Product: " + std::to_string(libevdev_get_id_product(device)));
			return std::shared_ptr<LinuxGameController>(new LinuxGameController(emu, deviceID, fd, device));
		} else {
			MessageManager::Log(std::string("[Input] Device ignored (Not a gamepad) - Name: ") + libevdev_get_name(device) + " Vendor: " + std::to_string(libevdev_get_id_vendor(device)) + " Product: " + std::to_string(libevdev_get_id_product(device)));
			close(fd);			
		}
	}	
	return nullptr;
}

LinuxGameController::LinuxGameController(Emulator* emu, int deviceID, int fileDescriptor, libevdev* device)
{
	_emu = emu;
	_deviceID = deviceID;
	_stopFlag = false;
	_device = device;
	_fd = fileDescriptor;
	memset(_axisDefaultValue, 0, sizeof(_axisDefaultValue));

	_rumbleEffect.reset(new ff_effect());
	memset(_rumbleEffect.get(), 0, sizeof(ff_effect));
	_rumbleEffect->type = FF_RUMBLE;
	_rumbleEffect->id = -1;
	_rumbleEffect->u.rumble.strong_magnitude = 0x8000;
	_rumbleEffect->u.rumble.weak_magnitude = 0x8000;
	_rumbleEffect->replay.length = 2000;
	_rumbleEffect->replay.delay = 0;

	int rc = ioctl(_fd, EVIOCSFF, _rumbleEffect.get());
	if(rc < 0) {
		MessageManager::Log("Could not initialize force feedback effect");
		_rumbleEffect.reset();
	}

	_eventThread = std::thread([=]() {
		int rc;
		bool calibrate = true;

		do {
			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(_fd, &readSet);

			//Timeout after 0.1 seconds (to allow thread to be terminated quickly)
			timeval timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;

			rc = select((int)_fd+1, &readSet, nullptr, nullptr, &timeout);
			if(rc) {
				do {
					struct input_event ev;
					rc = libevdev_next_event(_device, LIBEVDEV_READ_FLAG_NORMAL, &ev);
					if(rc == LIBEVDEV_READ_STATUS_SYNC) {
						while (rc == LIBEVDEV_READ_STATUS_SYNC) {
							rc = libevdev_next_event(_device, LIBEVDEV_READ_FLAG_SYNC, &ev);
						}
					} else if(rc == LIBEVDEV_READ_STATUS_SUCCESS) {
						//print_event(&ev);
					}
				} while(rc == LIBEVDEV_READ_STATUS_SYNC || rc == LIBEVDEV_READ_STATUS_SUCCESS);
			} 
			
			if(rc != LIBEVDEV_READ_STATUS_SYNC && rc != LIBEVDEV_READ_STATUS_SUCCESS && rc != -EAGAIN && rc != EWOULDBLOCK) {
				//Device was disconnected
				MessageManager::Log("[Input Device] Disconnected");
				break;
			}

			if(calibrate) {
				std::this_thread::sleep_for(std::chrono::duration<int, std::milli>(100));
				Calibrate();
				calibrate = false;
			}			
		} while(!_stopFlag);

		_disconnected = true;
	});
}

LinuxGameController::~LinuxGameController()
{
	_stopFlag = true;	
	_eventThread.join();

	libevdev_free(_device);
	close(_fd);
}

void LinuxGameController::Calibrate()
{
	int axes[14] = { ABS_X, ABS_Y, ABS_Z, ABS_RX, ABS_RY, ABS_RZ, ABS_HAT0X, ABS_HAT0Y, ABS_HAT1X, ABS_HAT1Y, ABS_HAT2X, ABS_HAT2Y, ABS_HAT3X, ABS_HAT3Y };
	for(int axis : axes) {
		_axisDefaultValue[axis] = libevdev_get_event_value(_device, EV_ABS, axis);
		//std::cout << "center values: " << std::to_string(_axisDefaultValue[axis]) << std::endl;
	}
}

bool LinuxGameController::CheckAxis(unsigned int code, bool forPositive)
{
	double deadZoneRatio = _emu->GetSettings()->GetControllerDeadzoneRatio();
	int deadZoneNegative = (_axisDefaultValue[code] - libevdev_get_abs_minimum(_device, code)) * 0.400 * deadZoneRatio;
	int deadZonePositive = (libevdev_get_abs_maximum(_device, code) - _axisDefaultValue[code]) * 0.400 * deadZoneRatio;
	
	if(forPositive) {
		return libevdev_get_event_value(_device, EV_ABS, code) - _axisDefaultValue[code] > deadZonePositive;
	} else {
		return libevdev_get_event_value(_device, EV_ABS, code) - _axisDefaultValue[code] < -deadZoneNegative;
	}
}

bool LinuxGameController::IsButtonPressed(int buttonNumber)
{
	bool pressed = false;
	switch(buttonNumber) {
		case 0: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_A) == 1; break;
		case 1: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_B) == 1; break;
		case 2: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_C) == 1; break;
		case 3: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_X) == 1; break;
		case 4: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_Y) == 1; break;
		case 5: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_Z) == 1; break;
		case 6: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TL) == 1; break;
		case 7: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TR) == 1; break;
		case 8: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TL2) == 1; break;
		case 9: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TR2) == 1; break;
		case 10: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_SELECT) == 1; break;
		case 11: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_START) == 1; break;
		case 12: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_THUMBL) == 1; break;
		case 13: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_THUMBR) == 1; break;

		case 14: pressed = CheckAxis(ABS_X, true); break;
		case 15: pressed = CheckAxis(ABS_X, false); break;
		case 16: pressed = CheckAxis(ABS_Y, true); break;
		case 17: pressed = CheckAxis(ABS_Y, false); break;
		case 18: pressed = CheckAxis(ABS_Z, true); break;
		case 19: pressed = CheckAxis(ABS_Z, false); break;
		case 20: pressed = CheckAxis(ABS_RX, true); break;
		case 21: pressed = CheckAxis(ABS_RX, false); break;
		case 22: pressed = CheckAxis(ABS_RY, true); break;
		case 23: pressed = CheckAxis(ABS_RY, false); break;
		case 24: pressed = CheckAxis(ABS_RZ, true); break;
		case 25: pressed = CheckAxis(ABS_RZ, false); break;

		case 26: pressed = CheckAxis(ABS_HAT0X, true); break;
		case 27: pressed = CheckAxis(ABS_HAT0X, false); break;
		case 28: pressed = CheckAxis(ABS_HAT0Y, true); break;
		case 29: pressed = CheckAxis(ABS_HAT0Y, false); break;
		case 30: pressed = CheckAxis(ABS_HAT1X, true); break;
		case 31: pressed = CheckAxis(ABS_HAT1X, false); break;
		case 32: pressed = CheckAxis(ABS_HAT1Y, true); break;
		case 33: pressed = CheckAxis(ABS_HAT1Y, false); break;
		case 34: pressed = CheckAxis(ABS_HAT2X, true); break;
		case 35: pressed = CheckAxis(ABS_HAT2X, false); break;
		case 36: pressed = CheckAxis(ABS_HAT2Y, true); break;
		case 37: pressed = CheckAxis(ABS_HAT2Y, false); break;
		case 38: pressed = CheckAxis(ABS_HAT3X, true); break;
		case 39: pressed = CheckAxis(ABS_HAT3X, false); break;
		case 40: pressed = CheckAxis(ABS_HAT3Y, true); break;
		case 41: pressed = CheckAxis(ABS_HAT3Y, false); break;

		case 42: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TRIGGER) == 1; break;
		case 43: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_THUMB) == 1; break;
		case 44: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_THUMB2) == 1; break;
		case 45: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TOP) == 1; break;
		case 46: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_TOP2) == 1; break;
		case 47: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_PINKIE) == 1; break;
		case 48: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_BASE) == 1; break;
		case 49: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_BASE2) == 1; break;
		case 50: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_BASE3) == 1; break;
		case 51: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_BASE4) == 1; break;
		case 52: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_BASE5) == 1; break;
		case 53: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_BASE6) == 1; break;
		case 54: pressed = libevdev_get_event_value(_device, EV_KEY, BTN_DEAD) == 1; break;
	}

	_enableForceFeedback |= pressed;

	return pressed;
}

optional<int16_t> LinuxGameController::GetAxisPosition(int axis)
{
	axis -= 55;

	unsigned int code;
	switch(axis) {
		default: return std::nullopt;
		case 0: code = ABS_Y; break;
		case 1: code = ABS_X; break;
		case 2: code = ABS_RY; break;
		case 3: code = ABS_RX; break;
		case 4: code = ABS_Z; break;
		case 5: code = ABS_RZ; break;
	}

	int min = libevdev_get_abs_minimum(_device, code);
	int max = libevdev_get_abs_maximum(_device, code);

	int value = libevdev_get_event_value(_device, EV_ABS, code);

	int range = max - min;
	int offset = value - min;
	double ratio = (double)offset / range;

	int16_t axisValue = (ratio - 0.5) * 2 * INT16_MAX;
	return axis & 0x01 ? axisValue : -axisValue;
}

void LinuxGameController::SetForceFeedback(uint16_t magnitude)
{
	if(!_rumbleEffect || !_enableForceFeedback) {
		return;
	}

	_rumbleEffect->u.rumble.strong_magnitude = magnitude;
	_rumbleEffect->u.rumble.weak_magnitude = magnitude;
	int rc = ioctl(_fd, EVIOCSFF, _rumbleEffect.get());
	if(rc < 0) {
		//MessageManager::Log("Could not update force feedback effect.");
		return;
	}

	struct input_event play = {};
	play.type = EV_FF;
	play.code = _rumbleEffect->id;
	play.value = 1;
	
	rc = write(_fd, (const void*)&play, sizeof(play));
	if(rc < 0) {
		//MessageManager::Log("Could not play force feedback effect.");
	}
}

bool LinuxGameController::IsDisconnected()
{
	return _disconnected;
}

int LinuxGameController::GetDeviceID()
{
	return _deviceID;
}

/*
static int print_event(struct input_event *ev)
{
	if (ev->type == EV_SYN)
		printf("Event: time %ld.%06ld, ++++++++++++++++++++ %s +++++++++++++++\n",
				ev->time.tv_sec,
				ev->time.tv_usec,
				libevdev_event_type_get_name(ev->type));
	else
		printf("Event: time %ld.%06ld, type %d (%s), code %d (%s), value %d\n",
			ev->time.tv_sec,
			ev->time.tv_usec,
			ev->type,
			libevdev_event_type_get_name(ev->type),
			ev->code,
			libevdev_event_code_get_name(ev->type, ev->code),
			ev->value);
	return 0;
}
*/
