#pragma once
#include <thread>
#include <atomic>

struct libevdev;
class Console;

class LinuxGameController
{
private:
	int _fd = -1;
	int _deviceID = -1;
	libevdev *_device = nullptr;
	bool _disconnected = false;
	std::thread _eventThread;
	std::atomic<bool> _stopFlag;
	shared_ptr<Console> _console;
	int _axisDefaultValue[0x100];

	LinuxGameController(shared_ptr<Console> console, int deviceID, int fileDescriptor, libevdev *device);
	bool CheckAxis(unsigned int code, bool forPositive);
	void Calibrate();	

public:
	~LinuxGameController();

	static std::shared_ptr<LinuxGameController> GetController(shared_ptr<Console> console, int deviceID, bool logInformation);

	bool IsDisconnected();
	int GetDeviceID();
	bool IsButtonPressed(int buttonNumber);
};