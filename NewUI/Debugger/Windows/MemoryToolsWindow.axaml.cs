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

		public MemoryToolsWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		ByteColorProvider colorProvider = new ByteColorProvider(
			SnesMemoryType.CpuMemory,
			true,
			true,
			true,
			60,
			false,
			false,
			false,
			false,
			true,
			true,
			true,
			true			
		);

		HexEditor _editor;
		MemoryToolsViewModel _model;
		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			//Renderer.DrawFps = true;

			_listener = new NotificationListener();
			_listener.OnNotification += listener_OnNotification;
			_editor = this.FindControl<HexEditor>("Hex");
			_editor.ByteColorProvider = colorProvider;
			_editor.ByteUpdated += editor_ByteUpdated;
		}

		private void editor_ByteUpdated(object? sender, ByteUpdatedEventArgs e)
		{
			DebugApi.SetMemoryValue(_model.MemoryType, (uint)e.ByteOffset, e.Value);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			base.OnDataContextChanged(e);
			_model = this.DataContext as MemoryToolsViewModel;
		}

		private void listener_OnNotification(NotificationEventArgs e)
		{
			if(e.NotificationType != ConsoleNotificationType.PpuFrameDone) {
				return;
			}

			byte[] data = DebugApi.GetMemoryState(_model.MemoryType);
			Dispatcher.UIThread.Post(() => {
				_editor.Data = data;
			});
		}
	}
}
