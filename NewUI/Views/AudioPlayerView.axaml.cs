using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.GUI.Config;
using Avalonia.Interactivity;
using Mesen.GUI;
using Mesen.GUI.Config.Shortcuts;

namespace Mesen.Views
{
	public class AudioPlayerView : UserControl
	{
		public AudioPlayerView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void OnPlayClick(object sender, RoutedEventArgs e)
		{
			if(EmuApi.IsPaused()) {
				EmuApi.Resume();
			} else {
				EmuApi.Pause();
			}
			
		}

		private void OnPrevTrackClick(object sender, RoutedEventArgs e)
		{
			EmuApi.ProcessAudioPlayerAction(new AudioPlayerActionParams() { Action = AudioPlayerAction.PrevTrack });
		}

		private void OnNextTrackClick(object sender, RoutedEventArgs e)
		{
			EmuApi.ProcessAudioPlayerAction(new AudioPlayerActionParams() { Action = AudioPlayerAction.NextTrack });
		}

		private void OnToggleShuffleClick(object sender, RoutedEventArgs e)
		{
			ConfigManager.Config.AudioPlayer.Shuffle = !ConfigManager.Config.AudioPlayer.Shuffle;
			ConfigManager.Config.ApplyConfig();
			ConfigManager.SaveConfig();
		}

		private void OnToggleRepeatClick(object sender, RoutedEventArgs e)
		{
			ConfigManager.Config.AudioPlayer.Repeat = !ConfigManager.Config.AudioPlayer.Repeat;
			ConfigManager.Config.ApplyConfig();
			ConfigManager.SaveConfig();
		}
	}
}
