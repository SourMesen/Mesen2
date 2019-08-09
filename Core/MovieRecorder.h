#pragma once
#include "stdafx.h"
#include <deque>
#include <unordered_map>
#include "IInputRecorder.h"
#include "BatteryManager.h"
#include "INotificationListener.h"
#include "MovieTypes.h"

class ZipWriter;
class Console;
//class RewindData;
//struct CodeInfo;

class MovieRecorder : public INotificationListener, public IInputRecorder, public IBatteryRecorder, public IBatteryProvider, public std::enable_shared_from_this<MovieRecorder>
{
private:
	static const uint32_t MovieFormatVersion = 1;

	shared_ptr<Console> _console;
	string _filename;
	string _author;
	string _description;
	unique_ptr<ZipWriter> _writer;
	std::unordered_map<string, vector<uint8_t>> _batteryData;
	stringstream _inputData;
	bool _hasSaveState = false;
	stringstream _saveStateData;

	void GetGameSettings(stringstream &out);
	//void WriteCheat(stringstream &out, CodeInfo &code);
	void WriteString(stringstream &out, string name, string value);
	void WriteInt(stringstream &out, string name, uint32_t value);
	void WriteBool(stringstream &out, string name, bool enabled);

public:
	MovieRecorder(shared_ptr<Console> console);
	virtual ~MovieRecorder();

	bool Record(RecordMovieOptions options);
	bool Stop();

	// Inherited via IInputRecorder
	void RecordInput(vector<shared_ptr<BaseControlDevice>> devices) override;

	// Inherited via IBatteryRecorder
	void OnLoadBattery(string extension, vector<uint8_t> batteryData) override;
	vector<uint8_t> LoadBattery(string extension) override;

	// Inherited via INotificationListener
	void ProcessNotification(ConsoleNotificationType type, void *parameter) override;
};
