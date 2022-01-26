using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Avalonia.Interactivity;
using Avalonia.Threading;
using Mesen.ViewModels;
using Mesen.Interop;

namespace Mesen.Views
{
	public class AudioPlayerView : UserControl
	{
		private DispatcherTimer? _timer;

		public AudioPlayerView()
		{
			InitializeComponent();
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			_timer = new DispatcherTimer(System.TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => ((AudioPlayerViewModel)DataContext!).UpdatePauseFlag());
			_timer.Start();
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			_timer?.Stop();
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
		}

		private void OnToggleRepeatClick(object sender, RoutedEventArgs e)
		{
			ConfigManager.Config.AudioPlayer.Repeat = !ConfigManager.Config.AudioPlayer.Repeat;
			ConfigManager.Config.ApplyConfig();
		}
	}
}
