using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Utilities;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using System.Threading.Tasks;

namespace Mesen.Windows
{
	public partial class UpdatePromptWindow : MesenWindow
	{
		private UpdatePromptViewModel _model;

		[Obsolete("For designer only")]
		public UpdatePromptWindow() : this(new(new())) { }

		public UpdatePromptWindow(UpdatePromptViewModel model)
		{
			_model = model;
			DataContext = model;

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			if(_model.IsUpdating) {
				e.Cancel = true;
			}
		}

		private void OnUpdateClick(object sender, RoutedEventArgs e)
		{
			_model.Progress = 0;
			_model.IsUpdating = true;

			Task.Run(async () => {
				bool result;
				try {
					result = await _model.UpdateMesen();
				} catch(Exception ex) {
					result = false;
					Dispatcher.UIThread.Post(() => {
						_model.IsUpdating = false;
						MesenMsgBox.ShowException(ex);
					});
					return;
				}

				Dispatcher.UIThread.Post(() => {
					_model.IsUpdating = false;
					if(result) {
						Close(true);
					}
				});
			});
		}

		private void OnCancelClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}
