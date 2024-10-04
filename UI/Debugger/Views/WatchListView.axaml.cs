using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using System.Linq;
using Mesen.Debugger.ViewModels;
using Avalonia.Input;
using System;
using Avalonia.Interactivity;
using DataBoxControl;
using Avalonia.Styling;
using Avalonia.LogicalTree;
using Avalonia.Controls.Selection;
using Avalonia.Threading;
using Mesen.Utilities;

namespace Mesen.Debugger.Views
{
	public class WatchListView : UserControl
	{
		public WatchListViewModel Model => (WatchListViewModel)DataContext!;

		private DataBox _grid;

		public WatchListView()
		{
			InitializeComponent();

			_grid = this.GetControl<DataBox>("WatchList");
			_grid.AddHandler(WatchListView.KeyDownEvent, OnGridKeyDown, RoutingStrategies.Tunnel, true);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is WatchListViewModel model) {
				model.InitContextMenu(this);
				model.Selection.SelectionChanged += Selection_SelectionChanged;
			}
			base.OnDataContextChanged(e);
		}

		bool inSelectionChanged = false;
		private void Selection_SelectionChanged(object? sender, SelectionModelSelectionChangedEventArgs<WatchValueInfo> e)
		{
			if(inSelectionChanged || Model == null || !IsKeyboardFocusWithin) {
				return;
			}

			inSelectionChanged = true;
			if(Model.Selection.Count == 1) {
				Dispatcher.UIThread.Post(() => {
					int index = Model.Selection.SelectedIndex;
					if(_grid.GetRow(index)?.IsKeyboardFocusWithin == false) {
						_grid.GetRow(index)?.Focus();
					}
				});
			} else if(Model.Selection.Count == 0) {
				Dispatcher.UIThread.Post(() => {
					_grid.GetRow(0)?.Focus();
					Model.Selection.Select(0);
				});
			}
			inSelectionChanged = false;
		}

		private void OnEntryContextRequested(object? sender, ContextRequestedEventArgs e)
		{
			//Select row when the textbox is right-clicked
			if(sender is TextBox txt && txt.DataContext is WatchValueInfo entry) {
				int index = Model.WatchEntries.IndexOf(entry);
				if(index >= 0) {
					if(!Model.Selection.IsSelected(index)) {
						Model.Selection.BeginBatchUpdate();
						Model.Selection.Clear();
						Model.Selection.Select(index);
						Model.Selection.EndBatchUpdate();
					}
					txt.FindLogicalAncestorOfType<DataBoxRow>()?.Focus();
				}
			}
		}

		private void OnEntryLostFocus(object? sender, RoutedEventArgs e)
		{
			//Update watch entry/list when textbox loses focus
			if(sender is TextBox txt && txt.DataContext is WatchValueInfo entry) {
				Model.EditWatch(Model.WatchEntries.IndexOf(entry), entry.Expression);
			}
		}

		private void OnEntryKeyDown(object? sender, KeyEventArgs e)
		{
			//Commit/undo modifications on enter/esc
			if(sender is TextBox txt && txt.DataContext is WatchValueInfo entry) {
				if(e.Key == Key.Enter) {
					e.Handled = true;
					txt.FindLogicalAncestorOfType<DataBoxRow>()?.Focus();
					Model.EditWatch(Model.WatchEntries.IndexOf(entry), entry.Expression);
				} else if(e.Key == Key.Escape) {
					//Undo
					e.Handled = true;
					Model.UpdateWatch();
					txt.FindLogicalAncestorOfType<DataBoxRow>()?.Focus();
				}
			}
		}

		private static bool IsTextKey(Key key)
		{
			return key >= Key.A && key <= Key.Z || key >= Key.D0 && key <= Key.D9 || key >= Key.NumPad0 && key <= Key.Divide || key >= Key.OemSemicolon && key <= Key.Oem102;
		}

		private void OnGridKeyDown(object? sender, KeyEventArgs e)
		{
			//Start editing textbox if a text key is pressed while not focused on the textbox
			//(except when control is held, to allow Ctrl+A select all to work properly)
			if(e.Source is DataBoxRow row && e.KeyModifiers != KeyModifiers.Control && IsTextKey(e.Key)) {
				WatchListTextBox? txt = row.CellsPresenter?.GetControl<WatchListTextBox>(0);
				txt?.FocusAndSelectAll();
			}
		}
	}

	public class WatchListTextBox : TextBox
	{
		protected override Type StyleKeyOverride => typeof(TextBox);

		private WatchListView? _listView;
		private bool _inOnGotFocus = false;

		protected override void OnAttachedToVisualTree(VisualTreeAttachmentEventArgs e)
		{
			base.OnAttachedToVisualTree(e);
			_listView = this.FindLogicalAncestorOfType<WatchListView>();
		}

		protected override void OnGotFocus(GotFocusEventArgs e)
		{
			base.OnGotFocus(e);

			if(_inOnGotFocus) {
				return;
			}

			_inOnGotFocus = true;
			DataBox? grid = this.FindLogicalAncestorOfType<DataBox>();
			if(grid != null && DataContext is WatchValueInfo watch && _listView != null) {
				//When clicking the textbox, select the row, too
				ISelectionModel selection = _listView.Model.Selection;
				if(!selection.SelectedItems.Contains(watch) || selection.SelectedItems.Count > 1) {
					selection.BeginBatchUpdate();
					if(e.KeyModifiers != KeyModifiers.Shift && e.KeyModifiers != KeyModifiers.Control) {
						selection.Clear();
					}
					selection.Select(_listView.Model.WatchEntries.IndexOf(watch));
					selection.EndBatchUpdate();
					Focus();
				}
			}
			_inOnGotFocus = false;
		}
	}
}
