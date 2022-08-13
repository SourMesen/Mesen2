using Avalonia.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	internal class MenuHelper
	{
		private static bool IsPointerInItem(MenuItem item)
		{
			if(item.IsSubMenuOpen) {
				foreach(var container in item.ItemContainerGenerator.Containers) {
					if(container.ContainerControl is MenuItem subItem && IsPointerInItem(subItem)) {
						return true;
					} else if(container.ContainerControl.IsPointerOver) {
						return true;
					}
				}
			}

			return item.IsPointerOver;
		}

		public static bool IsPointerInMenu(Menu menu)
		{
			if(menu.IsPointerOver) {
				return true;
			}

			if(menu.Items != null) {
				foreach(MenuItem item in menu.Items) {
					if(IsPointerInItem(item)) {
						return true;
					}
				}
			}

			return false;
		}
	}
}
