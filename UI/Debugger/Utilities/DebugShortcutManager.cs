using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Rendering;
using Mesen.Config;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	internal static class DebugShortcutManager
	{
		public static IEnumerable CreateContextMenu(Control ctrl, IEnumerable actions)
		{
			if(!(ctrl is IInputElement)) {
				throw new Exception("Invalid control");
			}

			ctrl.ContextMenu = new ContextMenu();
			ctrl.ContextMenu.Name = "ActionMenu";
			ctrl.ContextMenu.ItemsSource = actions;
			RegisterActions(ctrl, actions);

			return actions;
		}

		public static void RegisterActions(IInputElement focusParent, IEnumerable actions)
		{
			foreach(object obj in actions) {
				if(obj is ContextMenuAction action) {
					RegisterAction(focusParent, action);
				}
			}
		}

		public static void RegisterActions(IInputElement focusParent, IEnumerable<ContextMenuAction> actions)
		{
			foreach(ContextMenuAction action in actions) {
				RegisterAction(focusParent, action);
			}
		}

		public static void RegisterAction(IInputElement focusParent, ContextMenuAction action)
		{
			WeakReference<IInputElement> weakFocusParent = new WeakReference<IInputElement>(focusParent);
			WeakReference<ContextMenuAction> weakAction = new WeakReference<ContextMenuAction>(action);

			if(action.SubActions != null) {
				RegisterActions(focusParent, action.SubActions);
			}

			EventHandler<KeyEventArgs>? handler = null;
			handler = (s, e) => {
				if(weakFocusParent.TryGetTarget(out IInputElement? elem)) {
					if(weakAction.TryGetTarget(out ContextMenuAction? act)) {
						if(act.Shortcut != null) {
							DbgShortKeys keys = act.Shortcut();
							if(e.Key != Key.None && e.Key == keys.ShortcutKey && e.KeyModifiers == keys.Modifiers) {
								if(action.RoutingStrategy == RoutingStrategies.Bubble && e.Source is Control ctrl && ctrl.Classes.Contains("PreventShortcuts")) {
									return;
								}

								if(act.IsEnabled == null || act.IsEnabled()) {
									act.OnClick();
									e.Handled = true;
								}
							}
						}
					} else {
						elem.RemoveHandler(InputElement.KeyDownEvent, handler!);
					}
				}
			};

			focusParent.AddHandler(InputElement.KeyDownEvent, handler, action.RoutingStrategy, handledEventsToo: true);
		}
	}
}
