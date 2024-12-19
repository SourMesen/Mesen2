using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media.Imaging;
using Mesen.Utilities;

namespace Mesen.Controls
{
	public class OptionSection : ItemsControl
	{
		public static readonly StyledProperty<string> HeaderProperty = AvaloniaProperty.Register<OptionSection, string>(nameof(Header));
		public static readonly StyledProperty<string> IconProperty = AvaloniaProperty.Register<OptionSection, string>(nameof(Icon), "");
		public static readonly StyledProperty<Bitmap?> IconSourceProperty = AvaloniaProperty.Register<OptionSection, Bitmap?>(nameof(IconSource));

		public string Header
		{
			get { return GetValue(HeaderProperty); }
			set { SetValue(HeaderProperty, value); }
		}

		public string Icon
		{
			get { return GetValue(IconProperty); }
			set { SetValue(IconProperty, value); }
		}

		public Bitmap? IconSource
		{
			get { return GetValue(IconSourceProperty); }
			set { SetValue(IconSourceProperty, value); }
		}

		static OptionSection()
		{
			IconProperty.Changed.AddClassHandler<OptionSection>((x, e) => {
				x.IconSource = ImageUtilities.BitmapFromAsset(x.Icon);
			});
		}

		public OptionSection()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
