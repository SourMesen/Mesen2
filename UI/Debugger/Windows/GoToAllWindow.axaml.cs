using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Threading.Tasks;

namespace Mesen.Debugger.Windows
{
	public class GoToAllWindow : MesenWindow
	{
		private GoToAllViewModel _model;

		[Obsolete("For designer only")]
		public GoToAllWindow() : this(new()) { }

		public GoToAllWindow(GoToAllViewModel model)
		{
			_model = model;
			DataContext =  model;

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public static async Task<GoToDestination?> Open(Control parent, CpuType cpuType, GoToAllOptions options, ISymbolProvider? symbolProvider)
		{
			GoToAllViewModel model = new GoToAllViewModel(cpuType, options, symbolProvider);
			GoToAllWindow wnd = new GoToAllWindow(model);
			await wnd.ShowCenteredDialog(parent);
			return model.SelectedItem?.GetDestination();
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			ListBox list = this.GetControl<ListBox>("lstResults");
			list.DoubleTapped += lstResults_DoubleTapped;
			list.PointerReleased += List_PointerReleased;

			Dispatcher.UIThread.Post(() => {
				//Post this to run later, otherwise Linux seems to type the
				//comma (from the Ctrl+comma shortcut) inside the textbox
				this.GetControl<TextBox>("txtSearch").FocusAndSelectAll();
			});
		}

		bool _isDoubleTap = false;
		private void List_PointerReleased(object? sender, PointerReleasedEventArgs e)
		{
			if(_isDoubleTap) {
				SelectAndClose();
				_isDoubleTap = false;
			}
		}

		private void lstResults_DoubleTapped(object? sender, RoutedEventArgs e)
		{
			_isDoubleTap = true;
		}

		private void MoveSelection(int offset)
		{
			if(_model.SelectionModel.SelectedIndex + offset < 0) {
				_model.SelectionModel.SelectedIndex = 0;
			} else if(_model.SelectionModel.SelectedIndex + offset >= _model.SearchResults.Count) {
				_model.SelectionModel.SelectedIndex = _model.SearchResults.Count - 1;
			} else {
				_model.SelectionModel.SelectedIndex += offset;
			}
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			switch(e.Key) {
				case Key.Down: MoveSelection(1); e.Handled = true; break;
				case Key.Up: MoveSelection(-1); e.Handled = true; break;
				case Key.PageDown: MoveSelection(5); e.Handled = true; break;
				case Key.PageUp: MoveSelection(-5); e.Handled = true; break;
				case Key.Home: _model.SelectionModel.SelectedIndex = 0; e.Handled = true; break;
				case Key.End: _model.SelectionModel.SelectedIndex = _model.SearchResults.Count - 1; e.Handled = true; break;
			} 
			base.OnKeyDown(e);
		}

		private void SelectAndClose()
		{
			if(_model.SelectionModel.SelectedItem?.Disabled != true) {
				_model.SelectedItem = _model.SelectionModel.SelectedItem;
				Close();
			}
		}
		
		private void Select_OnClick(object sender, RoutedEventArgs e)
		{
			SelectAndClose();
		}

		private void Close_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}
	}
}
