using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using System.Collections.Generic;

namespace Mesen.Windows
{
	public class MovieRecordWindow : MesenWindow
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

			string? filename = await FileDialogHelper.SaveFile(ConfigManager.MovieFolder, EmuApi.GetRomInfo().GetRomName() + "." + FileDialogHelper.MesenMovieExt, VisualRoot, FileDialogHelper.MesenMovieExt);
			if(filename != null) {
				model.SavePath = filename;
			}
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			MovieRecordConfigViewModel model = (MovieRecordConfigViewModel)DataContext!;
			model.SaveConfig();

			RecordApi.MovieRecord(new RecordMovieOptions(model.SavePath, model.Config.Author, model.Config.Description, model.Config.RecordFrom));

			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}