using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.ViewModels;
using System;

namespace Mesen.Debugger.Views
{
	public class QuickSearchView : UserControl
	{
		private QuickSearchViewModel? _model;
		private TextBox _txtSearch;

		public QuickSearchView()
		{
			InitializeComponent();

			_txtSearch = this.FindControl<TextBox>("txtSearch");
			_txtSearch.KeyDown += txtSearch_KeyDown;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is QuickSearchViewModel model) {
				_model = model;
				model.SetSearchBox(_txtSearch);
			}
			base.OnDataContextChanged(e);
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			if(_model?.IsSearchBoxVisible == true && e.Key == Key.Escape) {
				e.Handled = true;
				e.Key = Key.None; //Prevent other shortcuts from being triggered
				_model?.Close();
			}
		}

		private void txtSearch_KeyDown(object? sender, KeyEventArgs e)
		{
			if(e.Key == Key.Enter) {
				_model?.FindNext();
			}
		}
	}
}
