using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Styling;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Controls
{
	public class MesenDataGrid : DataGrid, IStyleable
	{
		Type IStyleable.StyleKey => typeof(DataGrid);

		public MesenDataGrid()
		{
			CanUserSortColumns = true;
			CanUserResizeColumns = true;
			CanUserReorderColumns = true;
			SelectionMode = DataGridSelectionMode.Extended;
			FontFamily = new FontFamily("Microsoft Sans Serif");
			GridLinesVisibility = DataGridGridLinesVisibility.All;
			IsReadOnly = true;

			CellPointerPressed += MesenDataGrid_CellPointerPressed;
		}

		private void MesenDataGrid_CellPointerPressed(object? sender, DataGridCellPointerPressedEventArgs e)
		{
			if(e.PointerPressedEventArgs.GetCurrentPoint(null).Properties.IsRightButtonPressed) {
				object? item = e.Row.DataContext;
				if(!SelectedItems.Contains(item)) {
					SelectedIndex = e.Row.GetIndex();
				}
			}
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			if(e.Key == Key.End) {
				if(Items is IList list) {
					if(e.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
						int selectedIndex = SelectedIndex;
						SelectedItems.Clear();
						for(int i = selectedIndex; i < list.Count; i++) {
							SelectedItems.Add(list[i]);
						}
					} else {
						SelectedIndex = list.Count - 1;
					}
					ScrollIntoView(list[^1], Columns[0]);
				}
			} else if(e.Key == Key.Home) {
				if(Items is IList list) {
					if(e.KeyModifiers.HasFlag(KeyModifiers.Shift)) {
						int selectedIndex = SelectedIndex;
						SelectedItems.Clear();
						for(int i = selectedIndex; i >= 0; i--) {
							SelectedItems.Add(list[i]);
						}
					} else {
						SelectedIndex = 0;
					}
					ScrollIntoView(list[0], Columns[0]);
				}
			}
		}
	}
}
