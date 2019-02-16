#pragma once
#include "stdafx.h"
#include "../Core/INotificationListener.h"
#include "../Core/NotificationManager.h"

typedef void(__stdcall *NotificationListenerCallback)(int, void*);

class InteropNotificationListener : public INotificationListener
{
	NotificationListenerCallback _callback;

public:
	InteropNotificationListener(NotificationListenerCallback callback)
	{
		_callback = callback;
	}

	virtual ~InteropNotificationListener()
	{
	}

	void ProcessNotification(ConsoleNotificationType type, void* parameter)
	{
		_callback((int)type, parameter);
	}
};