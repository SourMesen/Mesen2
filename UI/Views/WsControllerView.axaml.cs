using Avalonia.Controls;
using Avalonia.Markup.Xaml;

namespace Mesen.Views;

public class WsControllerView : UserControl
{
	public WsControllerView()
	{
		InitializeComponent();
	}

	private void InitializeComponent()
	{
		AvaloniaXamlLoader.Load(this);
	}
}
