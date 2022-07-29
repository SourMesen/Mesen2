using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.ViewModels;

namespace Mesen.Views
{
	public class MainMenuView : UserControl
	{
		public Menu MainMenu { get; }

		public MainMenuView()
		{
			InitializeComponent();
			MainMenu = this.FindControl<Menu>("MainMenu");
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void mnuTools_Opened(object sender, RoutedEventArgs e)
		{
			if(DataContext is MainMenuViewModel model) {
				model.UpdateNetplayMenu();
			}
		}
	}
}
