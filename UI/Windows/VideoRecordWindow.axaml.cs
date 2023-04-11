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
	public class VideoRecordWindow : MesenWindow
	{
		public VideoRecordWindow()
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
			VideoRecordConfigViewModel model = (VideoRecordConfigViewModel)DataContext!;
			bool isGif = model.Config.Codec == VideoCodec.GIF;

			string initFilename = EmuApi.GetRomInfo().GetRomName() + (isGif ? ".gif" : ".avi");
			string? filename = await FileDialogHelper.SaveFile(ConfigManager.AviFolder, initFilename, VisualRoot, isGif ? FileDialogHelper.GifExt : FileDialogHelper.AviExt);
			
			if(filename != null) {
				model.SavePath = filename;
			}
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			VideoRecordConfigViewModel model = (VideoRecordConfigViewModel)DataContext!;
			model.SaveConfig();

			RecordApi.AviRecord(model.SavePath, new RecordAviOptions() {
				Codec = model.Config.Codec,
				CompressionLevel = model.Config.CompressionLevel,
				RecordSystemHud = model.Config.RecordSystemHud,
				RecordInputHud = model.Config.RecordInputHud
			});

			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}