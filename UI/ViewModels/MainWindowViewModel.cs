using Avalonia;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Reactive;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class MainWindowViewModel : ViewModelBase
	{
		public static MainWindowViewModel Instance { get; private set; } = null!;

		[Reactive] public MainMenuViewModel MainMenu { get; set; }
		[Reactive] public RomInfo RomInfo { get; set; }
		[Reactive] public AudioPlayerViewModel? AudioPlayer { get; private set; }
		[Reactive] public RecentGamesViewModel RecentGames { get; private set; }

		[Reactive] public string WindowTitle { get; private set; } = "Mesen";
		[Reactive] public Size RendererSize { get; set; }

		[Reactive] public bool IsMenuVisible { get; set; }
		
		[Reactive] public bool IsNativeRendererVisible { get; set; }
		[Reactive] public bool IsSoftwareRendererVisible { get; set; }

		public SoftwareRendererViewModel SoftwareRenderer { get; } = new();

		public Configuration Config { get; }

		public MainWindowViewModel()
		{
			Instance = this;

			Config = ConfigManager.Config;
			MainMenu = new MainMenuViewModel(this);
			RomInfo = new RomInfo();
			RecentGames = new RecentGamesViewModel();

			IsMenuVisible = !Config.Preferences.AutoHideMenu;
		}

		public void Init(MainWindow wnd)
		{
			MainMenu.Initialize(wnd);
			RecentGames.Init(GameScreenMode.RecentGames);

			this.WhenAnyValue(x => x.RecentGames.Visible, x => x.SoftwareRenderer.FrameSurface).Subscribe(x => {
				IsNativeRendererVisible = !RecentGames.Visible && SoftwareRenderer.FrameSurface == null;
				IsSoftwareRendererVisible = !RecentGames.Visible && SoftwareRenderer.FrameSurface != null;
			});
			
			this.WhenAnyValue(x => x.RomInfo).Subscribe(x => {
				bool showAudioPlayer = x.Format == RomFormat.Nsf || x.Format == RomFormat.Spc || x.Format == RomFormat.Gbs || x.Format == RomFormat.PceHes;
				if(AudioPlayer == null && showAudioPlayer) {
					AudioPlayer = new AudioPlayerViewModel();
				} else if(!showAudioPlayer) {
					AudioPlayer = null;
				}
			});

			this.WhenAnyValue(
				x => x.RomInfo,
				x => x.RendererSize,
				x => x.Config.Preferences.ShowTitleBarInfo,
				x => x.Config.Video.AspectRatio,
				x => x.Config.Video.VideoFilter
			).Subscribe(x => {
				UpdateWindowTitle();
			});

			UpdateWindowTitle();
		}

		private void UpdateWindowTitle()
		{
			string title = "Mesen";
			string romName = RomInfo.GetRomName();
			if(!string.IsNullOrWhiteSpace(romName)) {
				title += " - " + romName;
				if(ConfigManager.Config.Preferences.ShowTitleBarInfo) {
					FrameInfo baseSize = EmuApi.GetBaseScreenSize();
					double scale = (double)RendererSize.Height / baseSize.Height;
					title += string.Format(" - {0}x{1} ({2:0.###}x, {3})",
						Math.Round(RendererSize.Width),
						Math.Round(RendererSize.Height),
						scale,
						ResourceHelper.GetEnumText(ConfigManager.Config.Video.VideoFilter));
				}
			}
			WindowTitle = title;
		}
	}
}
