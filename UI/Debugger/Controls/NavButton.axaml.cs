using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Debugger.ViewModels;
using Avalonia.Media.Imaging;
using Mesen.Localization;

namespace Mesen.Debugger.Controls;

public class NavButton : UserControl
{
	public static readonly StyledProperty<NavType> NavProperty = AvaloniaProperty.Register<NavButton, NavType>(nameof(NavProperty));
	public static readonly StyledProperty<string> TooltipTextProperty = AvaloniaProperty.Register<NavButton, string>(nameof(TooltipText), "");
	public static readonly StyledProperty<Bitmap?> IconProperty = AvaloniaProperty.Register<NavButton, Bitmap?>(nameof(Icon));

	public Bitmap? Icon
	{
		get { return GetValue(IconProperty); }
		set { SetValue(IconProperty, value); }
	}

	public NavType Nav
	{
		get { return GetValue(NavProperty); }
		set { SetValue(NavProperty, value); }
	}

	public string TooltipText
	{
		get { return GetValue(TooltipTextProperty); }
		set { SetValue(TooltipTextProperty, value); }
	}

	static NavButton()
	{
		NavProperty.Changed.AddClassHandler<NavButton>((x, e) => {
			x.Icon = ImageUtilities.BitmapFromAsset("Assets/" + x.Nav.ToString() + ".png");
			x.TooltipText = ResourceHelper.GetEnumText(x.Nav);
		});
	}

	public NavButton()
	{
		InitializeComponent();
		Icon = ImageUtilities.BitmapFromAsset("Assets/" + Nav.ToString() + ".png");
		TooltipText = ResourceHelper.GetEnumText(Nav);
	}

	private void InitializeComponent()
	{
		AvaloniaXamlLoader.Load(this);
	}

	private void OnClick(object sender, RoutedEventArgs e)
	{
		if(DataContext is MemoryToolsDisplayOptionsViewModel model) {
			model.MemoryTools.NavigateTo(Nav);
		}
	}
}

public enum NavType
{
	NextCode,
	NextData,
	NextExec,
	NextRead,
	NextUnknown,
	NextWrite,

	PrevCode,
	PrevData,
	PrevExec,
	PrevRead,
	PrevUnknown,
	PrevWrite
}
