using Avalonia.Controls;
using Mesen.Interop;
using Mesen.Utilities;
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
				ActionType = ActionType.SaveRomAs,
				IsEnabled = () => IsSaveRomSupported(),
				OnClick = () => SaveRom(wnd, false)
			};
		}

		public static ContextMenuAction GetSaveEditsAsIpsAction(Window wnd)
		{
			return new ContextMenuAction() {
				ActionType = ActionType.SaveEditsAsIps,
				IsEnabled = () => IsSaveRomSupported(),
				OnClick = () => SaveRom(wnd, true)
			};
		}

		private static bool IsSaveRomSupported()
		{
			return EmuApi.GetRomInfo().Format switch {
				RomFormat.Sfc => true,
				RomFormat.Gb => true,
				RomFormat.Gbs => true,
				RomFormat.iNes => true,
				RomFormat.VsDualSystem => true,
				RomFormat.VsSystem => true,
				_ => false
			};
		}

		private static async void SaveRom(Window wnd, bool saveAsIps, CdlStripOption cdlOption = CdlStripOption.StripNone)
		{
			string romName = Path.GetFileName(EmuApi.GetRomInfo().RomPath);
			string ext = saveAsIps ? FileDialogHelper.IpsExt : Path.GetExtension(romName).Substring(1);
			romName = Path.ChangeExtension(romName, ext);

			string? filename = await FileDialogHelper.SaveFile(null, romName, wnd, ext);
			if(filename != null) {
				DebugApi.SaveRomToDisk(filename, saveAsIps, cdlOption);
			}
		}
	}
}
