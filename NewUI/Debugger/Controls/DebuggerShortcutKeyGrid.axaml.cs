using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Utilities;
using Mesen.Config;
using System.Collections.Generic;

namespace Mesen.Debugger.Controls
{
	public class DebuggerShortcutKeyGrid : UserControl
	{
		public static readonly StyledProperty<List<DebuggerShortcutInfo>> ShortcutsProperty = AvaloniaProperty.Register<DebuggerShortcutKeyGrid, List<DebuggerShortcutInfo>>(nameof(Shortcuts));

		public List<DebuggerShortcutInfo> Shortcuts
		{
			get { return GetValue(ShortcutsProperty); }
			set { SetValue(ShortcutsProperty, value); }
		}

		public DebuggerShortcutKeyGrid()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
