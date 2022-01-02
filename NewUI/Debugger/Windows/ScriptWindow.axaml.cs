using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Threading;
using AvaloniaEdit;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using System;
using System.ComponentModel;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class ScriptWindow : Window
	{
		private static IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		private DispatcherTimer _timer;
		private ScriptWindowViewModel? _model;

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
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(100), DispatcherPriority.Normal, (s, e) => UpdateLog());
		}

		protected override void OnOpened(EventArgs e)
		{
			if(Design.IsDesignMode) {
				return;
			}

			_timer.Start();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);

			if(Design.IsDesignMode) {
				return;
			}

			_timer.Stop();
			_model?.Config.SaveWindowSettings(this);
			ConfigManager.Config.Save();
			DataContext = null;
		}

		protected override void OnDataContextChanged(EventArgs e)
		{
			if(DataContext is ScriptWindowViewModel model) {
				_model = model;
				_model.InitActions(this);
				_model.Config.LoadWindowSettings(this);
			}
		}

		private void UpdateLog()
		{
			if(_model?.ScriptId >= 0) {
				_model.Log = DebugApi.GetScriptLog(_model.ScriptId);
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}
}
