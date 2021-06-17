using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.LogicalTree;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Threading;
using AvaloniaEdit;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class TraceLoggerWindow : Window
	{
		private NotificationListener? _listener = null;
		private TraceLoggerViewModel Model => ((TraceLoggerViewModel)DataContext!);
		private int _refreshCounter = 0;

		public TraceLoggerWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			_listener?.Dispose();
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					Model.UpdateAvailableTabs();
					break;

				case ConsoleNotificationType.CodeBreak: {
					Dispatcher.UIThread.Post(() => {
						Model.UpdateLog();
					});
					break;
				}

				case ConsoleNotificationType.PpuFrameDone: {
					_refreshCounter++;
					if(_refreshCounter == 9) {
						//Refresh every 9 frames, ~7fps
						Dispatcher.UIThread.Post(() => {
							Model.UpdateLog();
						});
						_refreshCounter = 0;
					}
					break;
				}
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
