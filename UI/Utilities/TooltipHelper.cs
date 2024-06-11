using Avalonia.Controls;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Text.Json.Serialization;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class TooltipHelper
	{
		public static void ShowTooltip(Control target, object? tooltipContent, int horizontalOffset)
		{
			try {
				ToolTip.SetShowDelay(target, 0);
				ToolTip.SetTip(target, tooltipContent);

				//Force tooltip to update its position
				ToolTip.SetHorizontalOffset(target, horizontalOffset - 1);
				ToolTip.SetHorizontalOffset(target, horizontalOffset);

				ToolTip.SetIsOpen(target, true);
			} catch(Exception) {
				HideTooltip(target);
			}
		}

		public static void HideTooltip(Control target)
		{
			try {
				ToolTip.SetTip(target, null);
				ToolTip.SetIsOpen(target, false);
			} catch(Exception) { }
		}
	}
}
