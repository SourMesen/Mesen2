using Avalonia;
using Avalonia.Controls;
using System;
using System.Linq;
using Avalonia.Styling;
using Avalonia.Interactivity;
using Mesen.Debugger.Utilities;
using System.Collections;

namespace Mesen.Controls
{
	public class MesenMenu : Menu
	{
		protected override Type StyleKeyOverride => typeof(Menu);

		private void SubmenuOpened(object? sender, RoutedEventArgs e) {
			MenuItem menuItem = (MenuItem)sender!;
			IEnumerable? source = menuItem.ItemsSource ?? menuItem.Items;
			if(source != null) {
				foreach(object subItemAction in source) {
					if(menuItem.ContainerFromItem(subItemAction) is MenuItem subMenuItem) {
						subMenuItem.SubmenuOpened -= SubmenuOpened;
						subMenuItem.SubmenuOpened += SubmenuOpened;
					}

					if(subItemAction is BaseMenuAction action) {
						action.Update();
					}
				}
			}
		}

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			if(Items != null) {
				foreach(object item in ItemsSource ?? Items) {
					if(item is MenuItem menuItem) {
						menuItem.SubmenuOpened += SubmenuOpened;
					}
				}
			}
		}

		protected override void OnDetachedFromVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnDetachedFromVisualTree(e);
			if(Items != null) {
				foreach(object item in ItemsSource ?? Items) {
					if(item is MenuItem menuItem) {
						menuItem.SubmenuOpened -= SubmenuOpened;
					}
				}
			}
		}
	}
}
