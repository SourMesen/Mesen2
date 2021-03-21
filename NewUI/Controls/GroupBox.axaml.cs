using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Controls
{
	public class GroupBox : ItemsControl
	{
		public static readonly StyledProperty<string> HeaderProperty = AvaloniaProperty.Register<GroupBox, string>(nameof(Header));

		public string Header
		{
			get { return GetValue(HeaderProperty); }
			set { SetValue(HeaderProperty, value); }
		}

		public GroupBox()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
