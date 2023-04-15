using Mesen.Config;
using Mesen.Interop;
using System;

namespace Mesen.Debugger.Utilities
{
	internal class MarkSelectionHelper
	{
		public static ContextMenuAction GetAction(Func<MemoryType> getMemType, Func<int> getSelStart, Func<int> getSelEnd, Action refreshView, Func<bool>? isVisible = null)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.MarkSelectionAs,
				HintText = () => GetAddressRange(getSelStart(), getSelEnd()),
				IsVisible = isVisible,
				SubActions = new() {
					new ContextMenuAction() {
						ActionType = ActionType.MarkAsCode,
						IsEnabled = () => GetMarkStartEnd(getMemType(), getSelStart(), getSelEnd(), out _, out _, out _),
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MarkAsCode),
						OnClick = () => MarkSelectionAs(getMemType(), getSelStart(), getSelEnd(), refreshView, CdlFlags.Code)
					},
					new ContextMenuAction() {
						ActionType = ActionType.MarkAsData,
						IsEnabled = () => GetMarkStartEnd(getMemType(), getSelStart(), getSelEnd(), out _, out _, out _),
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MarkAsData),
						OnClick = () => MarkSelectionAs(getMemType(), getSelStart(), getSelEnd(), refreshView, CdlFlags.Data)
					},
					new ContextMenuAction() {
						ActionType = ActionType.MarkAsUnidentified,
						IsEnabled = () => GetMarkStartEnd(getMemType(), getSelStart(), getSelEnd(), out _, out _, out _),
						Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.MarkAsUnidentified),
						OnClick = () => MarkSelectionAs(getMemType(), getSelStart(), getSelEnd(), refreshView, CdlFlags.None)
					}
				}
			};
		}

		private static string GetAddressRange(int start, int end)
		{
			string address = "$" + start.ToString("X2");
			if(end > start) {
				address += "-$" + end.ToString("X2");
			}
			return address;
		}

		private static bool GetMarkStartEnd(MemoryType memType, int selStart, int selEnd, out int start, out int end, out MemoryType startMemType)
		{
			start = selStart;
			end = selEnd;
			startMemType = memType;

			if(memType.IsRelativeMemory()) {
				AddressInfo startAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = start, Type = memType });
				AddressInfo endAddr = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = end, Type = memType });
				if(startAddr.Type == endAddr.Type && startAddr.Type.SupportsCdl() && !startAddr.Type.IsPpuMemory() && endAddr.Address - startAddr.Address == end - start) {
					startMemType = startAddr.Type;
					start = startAddr.Address;
					end = endAddr.Address;
				} else {
					return false;
				}
			} else if(!memType.SupportsCdl() || memType.IsPpuMemory()) {
				return false;
			}

			return start >= 0 && end >= 0 && start <= end;
		}

		private static void MarkSelectionAs(MemoryType memType, int selStart, int selEnd, Action refreshView, CdlFlags type)
		{
			if(GetMarkStartEnd(memType, selStart, selEnd, out int start, out int end, out MemoryType startMemType)) {
				DebugApi.MarkBytesAs(startMemType, (UInt32)start, (UInt32)end, type);
				refreshView();
			}
		}
	}
}
