using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Localization;
using System;
using System.Collections.Generic;
using System.Linq;

namespace Mesen.Views
{
	public class VideoConfigOverrideView : UserControl
	{
		public static readonly StyledProperty<bool> ShowBisqwitFilterProperty = AvaloniaProperty.Register<VideoConfigOverrideView, bool>(nameof(ShowBisqwitFilter), false);
		public static readonly StyledProperty<string> HeaderProperty = AvaloniaProperty.Register<VideoConfigOverrideView, string>(nameof(Header), ResourceHelper.GetViewLabel(nameof(VideoConfigOverrideView), "lblHeader"));
		public static readonly StyledProperty<Enum[]> AvailableValuesProperty = AvaloniaProperty.Register<VideoConfigOverrideView, Enum[]>(nameof(ShowBisqwitFilter), Array.Empty<Enum>());

		public bool ShowBisqwitFilter
		{
			get { return GetValue(ShowBisqwitFilterProperty); }
			set { SetValue(ShowBisqwitFilterProperty, value); }
		}

		public string Header
		{
			get { return GetValue(HeaderProperty); }
			set { SetValue(HeaderProperty, value); }
		}

		public Enum[] AvailableValues
		{
			get { return GetValue(AvailableValuesProperty); }
			set { SetValue(AvailableValuesProperty, value); }
		}

		public VideoConfigOverrideView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			AvailableValues = Enum.GetValues<VideoFilterType>().Where(x => x != VideoFilterType.NtscBisqwit || ShowBisqwitFilter).Cast<Enum>().ToArray();
			base.OnAttachedToVisualTree(e);
		}
	}
}
