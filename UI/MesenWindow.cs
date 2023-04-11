using Avalonia.Controls;
using System;

namespace Mesen;

public class MesenWindow : Window
{
	protected override void OnClosed(EventArgs e)
	{
		base.OnClosed(e);

		if(DataContext is IDisposable disposable) {
			disposable.Dispose();
		}

		//This fixes (or just dramatically reduces?) a memory leak
		//Most windows don't get GCed properly if this isn't done, leading to large memory leaks
		DataContext = null;
	}
}