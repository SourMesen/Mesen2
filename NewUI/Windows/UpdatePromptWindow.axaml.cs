using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;
using System;

namespace Mesen.Windows
{
	public partial class UpdatePromptWindow : Window
	{
		private UpdatePromptViewModel _model;

		[Obsolete("For designer only")]
		public UpdatePromptWindow() : this(new(new Version(), "")) { }

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

		private void OnUpdateClick(object sender, RoutedEventArgs e)
		{
			if(_model.UpdateMesen()) {
				Close(true);
			}
		}

		private void OnCancelClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}
