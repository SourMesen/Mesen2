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

namespace Mesen.Debugger.Views
{
	public class WatchListView : UserControl
	{
		public WatchListViewModel Model => (WatchListViewModel)DataContext!;

		public WatchListView()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);

			DataBox grid = this.GetControl<DataBox>("WatchList");
			grid.AddHandler(WatchListView.KeyDownEvent, OnGridKeyDown, RoutingStrategies.Tunnel, true);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is WatchListViewModel model) {
				model.InitContextMenu(this);
			}
			base.OnDataContextChanged(e);
		}

		private void OnEntryContextRequested(object? sender, ContextRequestedEventArgs e)
		{
			//Select row when the textbox is right-clicked
			if(sender is TextBox txt && txt.DataContext is WatchValueInfo entry) {
				int index = Model.WatchEntries.IndexOf(entry);
				if(index >= 0) {
					if(!Model.Selection.IsSelected(index)) {
						Model.Selection.Clear();
						Model.Selection.Select(index);
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
					Model.EditWatch(Model.WatchEntries.IndexOf(entry), entry.Expression);
					txt.FindLogicalAncestorOfType<DataBoxRow>()?.Focus();
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
				txt?.SelectAll();
				txt?.Focus();
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
					if(e.KeyModifiers != KeyModifiers.Shift && e.KeyModifiers != KeyModifiers.Control) {
						selection.Clear();
					}

					selection.Select(_listView.Model.WatchEntries.IndexOf(watch));
					Focus();
				}
			}
			_inOnGotFocus = false;
		}
	}
}
