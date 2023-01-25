using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config.Shortcuts
{
	public class ShortcutKeyInfo : ReactiveObject
	{
		[Reactive] public EmulatorShortcut Shortcut { get; set; }
		[Reactive] public KeyCombination KeyCombination { get; set; } = new KeyCombination();
		[Reactive] public KeyCombination KeyCombination2 { get; set; } = new KeyCombination();
	}
}
