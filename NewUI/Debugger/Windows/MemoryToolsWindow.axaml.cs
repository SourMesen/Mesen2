#pragma warning disable CS8618 // Non-nullable field must contain a non-null value when exiting constructor. Consider declaring as nullable.

using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.GUI;
using ReactiveUI;
using System;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;

namespace Mesen.Debugger.Windows
{
	public class MemoryToolsWindow : Window
	{
		private NotificationListener _listener;
		private HexEditor _editor;
		private MemoryToolsViewModel _model;

		public MemoryToolsWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			if(Design.IsDesignMode) {
				return;
			}

			_editor = this.FindControl<HexEditor>("Hex");
			_editor.ByteUpdated += editor_ByteUpdated;

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void editor_ByteUpdated(object? sender, ByteUpdatedEventArgs e)
		{
			DebugApi.SetMemoryValue(_model.MemoryType, (uint)e.ByteOffset, e.Value);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(this.DataContext is MemoryToolsViewModel model) {
				_model = model;
			} else {
				throw new Exception("Invalid model");
			}
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.PpuFrameDone) {
				return;
			}

			Dispatcher.UIThread.Post(() => {
				_editor.InvalidateVisual();
			});
		}
	}
}
