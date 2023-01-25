using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using System;

namespace Mesen.Controls
{
	public class PathSelector : UserControl
	{
		public static readonly StyledProperty<string> DisabledPathProperty = AvaloniaProperty.Register<PathSelector, string>(nameof(DisabledPath));
		public static readonly StyledProperty<string> PathProperty = AvaloniaProperty.Register<PathSelector, string>(nameof(Path), "", false, Avalonia.Data.BindingMode.TwoWay);
		public static readonly StyledProperty<bool> EditableProperty = AvaloniaProperty.Register<PathSelector, bool>(nameof(Editable));

		public string DisabledPath
		{
			get { return GetValue(DisabledPathProperty); }
			set { SetValue(DisabledPathProperty, value); }
		}

		public string Path
		{
			get { return GetValue(PathProperty); }
			set { SetValue(PathProperty, value); }
		}

		public bool Editable
		{
			get { return GetValue(EditableProperty); }
			set { SetValue(EditableProperty, value); }
		}

		public PathSelector()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private async void btnBrowse_OnClick(object sender, RoutedEventArgs e)
		{
			string? folderName = await FileDialogHelper.OpenFolder(VisualRoot);
			if(folderName?.Length > 0) {
				this.Path = folderName;
			}
		}
	}
}
