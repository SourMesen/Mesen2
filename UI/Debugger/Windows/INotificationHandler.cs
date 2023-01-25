using Mesen.Interop;

namespace Mesen.Debugger.Windows
{
	public interface INotificationHandler
	{
		void ProcessNotification(NotificationEventArgs e);
	}
}