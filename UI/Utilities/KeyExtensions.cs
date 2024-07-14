using Avalonia.Input;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class KeyExtensions
	{
		public static bool IsSpecialKey(this Key key)
		{
			//Some keys only trigger a KeyUp event without a matching KeyDown event
			//And some trigger both events at the same time, causing the emulator to never see the key press.
			switch(key) {
				case Key.PrintScreen:
				case Key.Pause:
				case Key.Scroll:
					return true;

				default:
					return false;
			}
		}
	}
}
