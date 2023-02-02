#include "Core/Shared/MessageManager.h"
#include "Core/Shared/Emulator.h"
#include "Core/Shared/EmuSettings.h"
#include "LinuxGameController.h"

#ifdef __APPLE__
	bool LinuxGameController::IsButtonPressed(int buttonNumber) { return 0; }
	int LinuxGameController::GetDeviceID() { return _deviceID; }
	bool LinuxGameController::IsDisconnected() { return _disconnected; }
	std::shared_ptr<LinuxGameController> LinuxGameController::GetController(Emulator* emu, int deviceID, bool logInformation) { return nullptr; }
#else
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
		int fd = open(deviceName.c_str(), O_RDONLY | O_NONBLOCK);
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
	switch(buttonNumber) {
		case 0: return libevdev_get_event_value(_device, EV_KEY, BTN_A) == 1;
		case 1: return libevdev_get_event_value(_device, EV_KEY, BTN_B) == 1;
		case 2: return libevdev_get_event_value(_device, EV_KEY, BTN_C) == 1;
		case 3: return libevdev_get_event_value(_device, EV_KEY, BTN_X) == 1;
		case 4: return libevdev_get_event_value(_device, EV_KEY, BTN_Y) == 1;
		case 5: return libevdev_get_event_value(_device, EV_KEY, BTN_Z) == 1;
		case 6: return libevdev_get_event_value(_device, EV_KEY, BTN_TL) == 1;
		case 7: return libevdev_get_event_value(_device, EV_KEY, BTN_TR) == 1;
		case 8: return libevdev_get_event_value(_device, EV_KEY, BTN_TL2) == 1;
		case 9: return libevdev_get_event_value(_device, EV_KEY, BTN_TR2) == 1;
		case 10: return libevdev_get_event_value(_device, EV_KEY, BTN_SELECT) == 1;
		case 11: return libevdev_get_event_value(_device, EV_KEY, BTN_START) == 1;
		case 12: return libevdev_get_event_value(_device, EV_KEY, BTN_THUMBL) == 1;
		case 13: return libevdev_get_event_value(_device, EV_KEY, BTN_THUMBR) == 1;

		case 14: return CheckAxis(ABS_X, true);
		case 15: return CheckAxis(ABS_X, false);
		case 16: return CheckAxis(ABS_Y, true);
		case 17: return CheckAxis(ABS_Y, false); 
		case 18: return CheckAxis(ABS_Z, true);
		case 19: return CheckAxis(ABS_Z, false);		
		case 20: return CheckAxis(ABS_RX, true);
		case 21: return CheckAxis(ABS_RX, false);		
		case 22: return CheckAxis(ABS_RY, true);
		case 23: return CheckAxis(ABS_RY, false);				
		case 24: return CheckAxis(ABS_RZ, true);
		case 25: return CheckAxis(ABS_RZ, false);		

		case 26: return CheckAxis(ABS_HAT0X, true);
		case 27: return CheckAxis(ABS_HAT0X, false);		
		case 28: return CheckAxis(ABS_HAT0Y, true);
		case 29: return CheckAxis(ABS_HAT0Y, false);				
		case 30: return CheckAxis(ABS_HAT1X, true);
		case 31: return CheckAxis(ABS_HAT1X, false);		
		case 32: return CheckAxis(ABS_HAT1Y, true);
		case 33: return CheckAxis(ABS_HAT1Y, false);
		case 34: return CheckAxis(ABS_HAT2X, true);
		case 35: return CheckAxis(ABS_HAT2X, false);		
		case 36: return CheckAxis(ABS_HAT2Y, true);
		case 37: return CheckAxis(ABS_HAT2Y, false);
		case 38: return CheckAxis(ABS_HAT3X, true);
		case 39: return CheckAxis(ABS_HAT3X, false);		
		case 40: return CheckAxis(ABS_HAT3Y, true);
		case 41: return CheckAxis(ABS_HAT3Y, false);

		case 42: return libevdev_get_event_value(_device, EV_KEY, BTN_TRIGGER) == 1;
		case 43: return libevdev_get_event_value(_device, EV_KEY, BTN_THUMB) == 1;
		case 44: return libevdev_get_event_value(_device, EV_KEY, BTN_THUMB2) == 1;
		case 45: return libevdev_get_event_value(_device, EV_KEY, BTN_TOP) == 1;
		case 46: return libevdev_get_event_value(_device, EV_KEY, BTN_TOP2) == 1;
		case 47: return libevdev_get_event_value(_device, EV_KEY, BTN_PINKIE) == 1;
		case 48: return libevdev_get_event_value(_device, EV_KEY, BTN_BASE) == 1;
		case 49: return libevdev_get_event_value(_device, EV_KEY, BTN_BASE2) == 1;
		case 50: return libevdev_get_event_value(_device, EV_KEY, BTN_BASE3) == 1;
		case 51: return libevdev_get_event_value(_device, EV_KEY, BTN_BASE4) == 1;
		case 52: return libevdev_get_event_value(_device, EV_KEY, BTN_BASE5) == 1;
		case 53: return libevdev_get_event_value(_device, EV_KEY, BTN_BASE6) == 1;
		case 54: return libevdev_get_event_value(_device, EV_KEY, BTN_DEAD) == 1;		
	}

	return false;
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
#endif
