using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using AvaloniaEdit;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class AssemblerWindow : Window
	{
		private static IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		private MesenTextEditor _hexView;

		static AssemblerWindow()
		{
			using XmlReader reader = XmlReader.Create(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.Highlight6502.xshd")!);
			XshdSyntaxDefinition xshd = HighlightingLoader.LoadXshd(reader);
			_highlighting = HighlightingLoader.Load(xshd, HighlightingManager.Instance);
		}

		public AssemblerWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif

			_textEditor = this.FindControl<MesenTextEditor>("Editor");
			_hexView = this.FindControl<MesenTextEditor>("HexView");
			_textEditor.TextChanged += textEditor_TextChanged;
			_textEditor.TemplateApplied += textEditor_TemplateApplied;
			_hexView.TemplateApplied += hexView_TemplateApplied;
			
			_textEditor.SyntaxHighlighting = _highlighting;
		}

		private void hexView_TemplateApplied(object? sender, Avalonia.Controls.Primitives.TemplateAppliedEventArgs e)
		{
			_hexView.ScrollChanged += hexView_ScrollChanged;
		}

		private void textEditor_TemplateApplied(object? sender, Avalonia.Controls.Primitives.TemplateAppliedEventArgs e)
		{
			_textEditor.ScrollChanged += textEditor_ScrollChanged;
		}

		private void hexView_ScrollChanged(object? sender, ScrollChangedEventArgs e)
		{
			_textEditor.VerticalScrollBarValue = _hexView.VerticalScrollBarValue;
		}

		private void textEditor_ScrollChanged(object? sender, ScrollChangedEventArgs e)
		{
			_hexView.VerticalScrollBarValue = _textEditor.VerticalScrollBarValue;
		}

		private void textEditor_TextChanged(object? sender, System.EventArgs e)
		{
			((AssemblerWindowViewModel)DataContext!).Code = _textEditor.Text;
		}

		private void OnCellPointerPressed(object? sender, DataGridCellPointerPressedEventArgs e)
		{
			int lineNumber = (((AssemblerError?)e.Row.DataContext)?.LineNumber ?? 0)- 1;
			if(lineNumber >= 0) {
				_textEditor.TextArea.Caret.Line = lineNumber;
				_textEditor.TextArea.Caret.Column = 0;
				_textEditor.ScrollLineToTop(lineNumber);
			}
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
