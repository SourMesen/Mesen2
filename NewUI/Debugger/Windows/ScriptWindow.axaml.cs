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
using Mesen.Interop;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class ScriptWindow : Window
	{
		private static IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		
		static ScriptWindow()
		{
			using XmlReader reader = XmlReader.Create(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.HighlightLua.xshd")!);
			XshdSyntaxDefinition xshd = HighlightingLoader.LoadXshd(reader);
			_highlighting = HighlightingLoader.Load(xshd, HighlightingManager.Instance);
		}

		public ScriptWindow()
		{
			InitializeComponent();
#if DEBUG
            this.AttachDevTools();
#endif

			_textEditor = this.FindControl<MesenTextEditor>("Editor");
			_textEditor.SyntaxHighlighting = _highlighting;
		}

		private int _scriptId = -1;
		private void OnRunClick(object sender, RoutedEventArgs e)
		{
			_scriptId = DebugApi.LoadScript("test", _textEditor.Text);
		}

		private void OnStopClick(object sender, RoutedEventArgs e)
		{
			if(_scriptId >= 0) {
				DebugApi.RemoveScript(_scriptId);
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
