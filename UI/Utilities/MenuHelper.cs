using Avalonia.Controls;
using Avalonia.Controls.Primitives;
using Avalonia.Input;
using Avalonia.LogicalTree;
using Avalonia.VisualTree;
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
				bool checkPopup = true;
				foreach(var child in item.GetLogicalChildren()) {
					if(child is MenuItem subItem) {
						if(IsPointerInItem(subItem)) {
							return true;
						} else if(checkPopup) {
							if(subItem.GetVisualRoot() is PopupRoot root) {
								if(root.IsPointerOver) {
									return true;
								}
							}
							checkPopup = false;
						}
					} else if(child is InputElement inputElem && inputElem.IsPointerOver) {
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
				foreach(MenuItem? item in menu.Items) {
					if(item != null && IsPointerInItem(item)) {
						return true;
					}
				}
			}

			return false;
		}

		private static bool IsFocusInItem(MenuItem item)
		{
			if(item.IsSubMenuOpen) {
				bool checkPopup = true;
				foreach(var child in item.GetLogicalChildren()) {
					if(child is MenuItem subItem) {
						if(IsFocusInItem(subItem)) {
							return true;
						} else if(checkPopup) {
							if(subItem.GetVisualRoot() is PopupRoot root) {
								if(root.IsKeyboardFocusWithin) {
									return true;
								}
							}
							checkPopup = false;
						}
					} else if(child is InputElement inputElem && inputElem.IsKeyboardFocusWithin) {
						return true;
					}
				}
			}

			return item.IsKeyboardFocusWithin;
		}

		public static bool IsFocusInMenu(Menu menu)
		{
			if(menu.IsKeyboardFocusWithin) {
				return true;
			}

			if(menu.Items != null) {
				foreach(MenuItem? item in menu.Items) {
					if(item != null && IsFocusInItem(item)) {
						return true;
					}
				}
			}

			return false;
		}
	}
}
