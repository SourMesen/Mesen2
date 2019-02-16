#pragma once
#include "stdafx.h"
#include "INotificationListener.h"
#include "../Utilities/SimpleLock.h"

class NotificationManager
{
private:
	SimpleLock _lock;
	vector<weak_ptr<INotificationListener>> _listenersToAdd;
	vector<weak_ptr<INotificationListener>> _listeners;
	
	void CleanupNotificationListeners();

public:
	void RegisterNotificationListener(shared_ptr<INotificationListener> notificationListener);
	void SendNotification(ConsoleNotificationType type, void* parameter = nullptr);
};
