using Avalonia.Controls;
using Avalonia.Input;
using Mesen.Config;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	internal static class DebugShortcutManager
	{
		public static void RegisterActions(Window wnd, IInputElement focusParent, IEnumerable<object> actions)
		{
			foreach(object obj in actions) {
				if(obj is ContextMenuAction action) {
					RegisterAction(wnd, focusParent, action);
				}
			}
		}

		public static void RegisterActions(Window wnd, IInputElement focusParent, IEnumerable<ContextMenuAction> actions)
		{
			foreach(ContextMenuAction action in actions) {
				RegisterAction(wnd, focusParent, action);
			}
		}

		public static void RegisterAction(Window wnd, IInputElement focusParent, ContextMenuAction action)
		{
			WeakReference<IInputElement> weakFocusParent = new WeakReference<IInputElement>(focusParent);
			WeakReference<ContextMenuAction> weakAction = new WeakReference<ContextMenuAction>(action);
			WeakReference<Window> weakWnd = new WeakReference<Window>(wnd);

			EventHandler<KeyEventArgs>? handler = null;
			handler = (s, e) => {
				if(weakFocusParent.TryGetTarget(out IInputElement? elem) && weakAction.TryGetTarget(out ContextMenuAction? act)) {
					if(act.Shortcut != null) {
						DbgShortKeys keys = act.Shortcut();
						if(elem.IsKeyboardFocusWithin && e.Key == keys.ShortcutKey && e.KeyModifiers == keys.Modifiers) {
							if(act.IsEnabled == null || act.IsEnabled()) {
								act.OnClick();
							}
						}
					}
				} else {
					if(weakWnd.TryGetTarget(out Window? window)) {
						window.KeyDown -= handler;
					}
				}
			};

			wnd.KeyDown += handler;
		}
	}
}
