using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.IO;
using System.Reactive.Linq;

namespace Mesen.ViewModels
{
	public class MovieRecordConfigViewModel : ViewModelBase
	{
		[Reactive] public string SavePath { get; set; }
		[Reactive] public MovieRecordConfig Config { get; set; }
		
		public MovieRecordConfigViewModel()
		{
			Config = ConfigManager.Config.MovieRecord.Clone();

			SavePath = Path.Join(ConfigManager.MovieFolder, EmuApi.GetRomInfo().GetRomName() + "." + FileDialogHelper.MesenMovieExt);
		}

		public void SaveConfig()
		{
			ConfigManager.Config.MovieRecord = Config.Clone();
		}
   }
}
