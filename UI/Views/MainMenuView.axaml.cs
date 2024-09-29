using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.ViewModels;
using System.Collections;

namespace Mesen.Views
{
	public class MainMenuView : UserControl
	{
		public Menu MainMenu { get; }

		public MainMenuView()
		{
			InitializeComponent();
			MainMenu = this.GetControl<Menu>("ActionMenu");
			
			MainMenu.Closed += (s, e) => {
				//When an option is selected in the menu (e.g with enter or mouse click)
				//steal focus away from the menu to ensure pressing e.g left/right goes to the
				//game only and doesn't re-activate the main menu
				ApplicationHelper.GetMainWindow()?.Focus();
			};

			Panel panel = this.GetControl<Panel>("MenuPanel");
			panel.PointerPressed += (s, e) => {
				if(s == panel) {
					//Close the menu when the blank space on the right is clicked
					MainMenu.Close();
					ApplicationHelper.GetMainWindow()?.Focus();
				}
			};
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void mnuTools_Opened(object sender, RoutedEventArgs e)
		{
			if(DataContext is MainMenuViewModel model) {
				if(model.UpdateNetplayMenu() && e.Source is MenuItem item) {
					//Force a refresh of the tools menu to ensure
					//the "Select controller" submenu gets updated
					IEnumerable? items = item.ItemsSource;
					item.ItemsSource = null;
					item.ItemsSource = items;
				}
			}
		}
	}
}
