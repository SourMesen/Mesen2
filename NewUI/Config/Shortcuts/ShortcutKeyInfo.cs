using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config.Shortcuts
{
	public struct ShortcutKeyInfo
	{
		public EmulatorShortcut Shortcut;
		public KeyCombination KeyCombination;
		
		public ShortcutKeyInfo(EmulatorShortcut key, KeyCombination keyCombination)
		{
			Shortcut = key;
			KeyCombination = keyCombination;
		}
	}
}
