using Avalonia.Controls;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	internal class SaveRomActionHelper
	{
		public static ContextMenuAction GetSaveRomAction(Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.SaveRom,
				IsEnabled = () => IsSaveRomSupported() && !((ResourcePath)MainWindowViewModel.Instance.RomInfo.RomPath).Compressed,
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveRom),
				OnClick = () => SaveRom(wnd)
			};
		}

		public static ContextMenuAction GetSaveRomAsAction(Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.SaveRomAs,
				IsEnabled = () => IsSaveRomSupported(),
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveRomAs),
				OnClick = () => SaveRomAs(wnd, false)
			};
		}

		public static ContextMenuAction GetSaveEditsAsIpsAction(Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.SaveEditsAsIps,
				IsEnabled = () => IsSaveRomSupported(),
				Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.SaveEditAsIps),
				OnClick = () => SaveRomAs(wnd, true)
			};
		}

		private static bool IsSaveRomSupported()
		{
			return MainWindowViewModel.Instance.RomInfo.Format switch {
				RomFormat.Sfc => true,
				RomFormat.Gb => true,
				RomFormat.Gbs => true,
				RomFormat.iNes => true,
				RomFormat.VsDualSystem => true,
				RomFormat.VsSystem => true,
				RomFormat.Pce => true,
				RomFormat.Sms => true,
				RomFormat.Sg => true,
				RomFormat.ColecoVision => true,
				RomFormat.Gba => true,
				RomFormat.Ws => true,
				_ => false
			};
		}

		private static async void SaveRom(Window wnd)
		{
			string romName = MainWindowViewModel.Instance.RomInfo.RomPath;
			if(!DebugApi.SaveRomToDisk(romName, false, CdlStripOption.StripNone)) {
				await MesenMsgBox.Show(wnd, "FileSaveError", Mesen.Windows.MessageBoxButtons.OK, Mesen.Windows.MessageBoxIcon.Error);
			}
		}

		public static async void SaveRomAs(Window wnd, bool saveAsIps, CdlStripOption cdlOption = CdlStripOption.StripNone)
		{
			string romName = Path.GetFileName(MainWindowViewModel.Instance.RomInfo.RomPath);
			string ext = saveAsIps ? FileDialogHelper.IpsExt : Path.GetExtension(romName).Substring(1);
			romName = Path.ChangeExtension(romName, ext);

			string? filename = await FileDialogHelper.SaveFile(null, romName, wnd, ext);
			if(filename != null) {
				if(!DebugApi.SaveRomToDisk(filename, saveAsIps, cdlOption)) {
					await MesenMsgBox.Show(wnd, "FileSaveError", Mesen.Windows.MessageBoxButtons.OK, Mesen.Windows.MessageBoxIcon.Error);
				}
			}
		}
	}
}
