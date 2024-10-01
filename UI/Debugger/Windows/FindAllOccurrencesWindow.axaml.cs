using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Controls;
using Mesen.Utilities;
using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Debugger.Windows
{
	public class FindAllOccurrencesWindow : MesenWindow
	{
		private static string _lastSearch { get; set; } = "";
		private static bool _lastMatchCase { get; set; } = false;
		private static bool _lastMatchWholeWord { get; set; } = false;

		public string SearchString { get; set; }
		public bool MatchCase { get; set; }
		public bool MatchWholeWord { get; set; }

		public FindAllOccurrencesWindow()
		{
			SearchString = _lastSearch;
			MatchCase = _lastMatchCase;
			MatchWholeWord = _lastMatchWholeWord;

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			this.GetControl<TextBox>("txtSearch").FocusAndSelectAll();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_lastSearch = SearchString;
			_lastMatchCase = MatchCase;
			_lastMatchWholeWord = MatchWholeWord;
			Close(SearchString);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(null!);
		}
	}
}
