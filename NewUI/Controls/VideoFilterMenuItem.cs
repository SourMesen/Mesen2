using Avalonia;
using Avalonia.Controls;
using System;
using Mesen.Config;
using Mesen.Localization;
using Avalonia.Interactivity;
using Avalonia.Styling;
using System.Drawing;
using Mesen.Utilities;

namespace Mesen.Controls
{
	public class VideoFilterMenuItem : MenuItem, IStyleable
	{
		Type IStyleable.StyleKey => typeof(MenuItem);

		public static readonly StyledProperty<VideoFilterType> FilterProperty = AvaloniaProperty.Register<VideoFilterMenuItem, VideoFilterType>(nameof(Filter));

		public VideoFilterType Filter
		{
			get { return GetValue(FilterProperty); }
			set { SetValue(FilterProperty, value); }
		}

		public VideoFilterMenuItem()
		{
		}

		protected override void OnInitialized()
		{
			if(Parent is MenuItem parent) {
				Action updateShortcut = () => {
					Header = ResourceHelper.GetEnumText(Filter);
					Icon = (ConfigManager.Config.Video.VideoFilter == Filter ? ImageUtilities.FromAsset("Assets/MenuItemChecked.png") : null)!;
				};

				updateShortcut();

				//Update item state when its parent opens
				parent.SubmenuOpened += (object? sender, RoutedEventArgs e) => {
					updateShortcut();
				};

				Click += (object? sender, RoutedEventArgs e) => {
					ConfigManager.Config.Video.VideoFilter = Filter;
					ConfigManager.Config.Video.ApplyConfig();
				};
			}
		}
	}
}
