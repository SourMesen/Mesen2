using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using System.Collections.Generic;

namespace Mesen.Windows
{
	public class MovieRecordWindow : Window
	{
		public MovieRecordWindow()
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

		private async void OnBrowseClick(object sender, RoutedEventArgs e)
		{
			MovieRecordConfigViewModel model = (MovieRecordConfigViewModel)DataContext!;

			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Filters = new List<FileDialogFilter> {
				new FileDialogFilter() { Name = "Mesen Movies", Extensions = { "msm" } },
				new FileDialogFilter() { Name = "All files", Extensions = { "*" } }
			};
			sfd.Directory = ConfigManager.MovieFolder;
			sfd.InitialFileName = EmuApi.GetRomInfo().GetRomName() + ".msm";
			string? filename = await sfd.ShowAsync(VisualRoot as Window);
			if(filename != null) {
				model.SavePath = filename;
			}
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);

			MovieRecordConfigViewModel model = (MovieRecordConfigViewModel)DataContext!;
			model.SaveConfig();

			RecordApi.MovieRecord(new RecordMovieOptions(model.SavePath, model.Config.Author, model.Config.Description, model.Config.RecordFrom));
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}