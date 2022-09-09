#pragma once
#include "pch.h"
#include "Shared/Interfaces/INotificationListener.h"
#include "Utilities/SimpleLock.h"

class NotificationManager
{
private:
	SimpleLock _lock;
	vector<weak_ptr<INotificationListener>> _listeners;
	
	void CleanupNotificationListeners();

public:
	void RegisterNotificationListener(shared_ptr<INotificationListener> notificationListener);
	void SendNotification(ConsoleNotificationType type, void* parameter = nullptr);
};
