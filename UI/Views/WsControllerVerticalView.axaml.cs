using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Views;

public class WsControllerVerticalView : UserControl
{
	public WsControllerVerticalView()
	{
		InitializeComponent();
	}

	private void InitializeComponent()
	{
		AvaloniaXamlLoader.Load(this);
	}
}
