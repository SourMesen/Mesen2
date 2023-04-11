using Avalonia.Controls;
using System;

namespace Mesen;

public class MesenWindow : Window
{
	protected override void OnClosed(EventArgs e)
	{
		base.OnClosed(e);

		//This fixes (or just dramatically reduces?) a memory leak
		//Most windows don't get GCed properly if this isn't done, leading to large memory leaks
		DataContext = null;
	}
}