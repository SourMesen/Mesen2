using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using System.ComponentModel;

namespace Mesen.Windows
{
	public class HistoryViewerRangePickerWindow : MesenWindow
	{
		public int MinValue { get; set; }
		public int MaxValue { get; set; }

		public int StartTime { get; set; }
		public int EndTime { get; set; }

		[Obsolete("For designer only")]
		public HistoryViewerRangePickerWindow() : this(new(), new()) { }

		public HistoryViewerRangePickerWindow(TimeSpan start, TimeSpan end)
		{
			MinValue = (int)start.TotalSeconds;
			MaxValue = (int)end.TotalSeconds;
			StartTime = MinValue;
			EndTime = MaxValue;

			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}