#pragma once
#include <thread>
#include <atomic>

struct libevdev;
struct ff_effect;
class Emulator;

class LinuxGameController
{
private:
	int _fd = -1;
	int _deviceID = -1;
	libevdev *_device = nullptr;
	bool _disconnected = false;
	std::thread _eventThread;
	std::atomic<bool> _stopFlag;
	Emulator* _emu = nullptr;

	unique_ptr<ff_effect> _rumbleEffect;
	bool _enableForceFeedback = false;
	int _axisDefaultValue[0x100] = {};

	LinuxGameController(Emulator* emu, int deviceID, int fileDescriptor, libevdev *device);
	bool CheckAxis(unsigned int code, bool forPositive);
	void Calibrate();	

public:
	~LinuxGameController();

	static std::shared_ptr<LinuxGameController> GetController(Emulator* emu, int deviceID, bool logInformation);

	bool IsDisconnected();
	int GetDeviceID();
	bool IsButtonPressed(int buttonNumber);
	optional<int16_t> GetAxisPosition(int axis);

	void SetForceFeedback(uint16_t magnitude);
};