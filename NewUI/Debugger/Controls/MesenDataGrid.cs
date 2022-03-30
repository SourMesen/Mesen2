using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Media;
using Avalonia.Styling;
using System;
using System.Collections;

namespace Mesen.Debugger.Controls
{
	public class MesenDataGrid : DataGrid, IStyleable
	{
		Type IStyleable.StyleKey => typeof(DataGrid);

		private bool _isEditing = false;

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

		protected override void OnPointerPressed(PointerPressedEventArgs e)
		{
			base.OnPointerPressed(e);
			
			//Clear selection when clicking in blank area outside rows
			SelectedItem = null;
			
			//Give focus to ensure any context menu actions work properly
			Focus();
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

		protected override void OnBeginningEdit(DataGridBeginningEditEventArgs e)
		{
			base.OnBeginningEdit(e);
			_isEditing = true;
		}

		protected override void OnCellEditEnded(DataGridCellEditEndedEventArgs e)
		{
			base.OnCellEditEnded(e);
			_isEditing = false;
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);

			if(!_isEditing && (e.KeyModifiers == KeyModifiers.Control || e.KeyModifiers == KeyModifiers.Alt)) {
				//Disable all alt/ctrl DataGrid shortcuts to use our own shortcuts instead
				e.Handled = true;
			}

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
