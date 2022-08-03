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
using Mesen.Utilities;
using System;
using System.ComponentModel;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class ScriptWindow : Window, INotificationHandler
	{
		private static XshdSyntaxDefinition _syntaxDef;
		private IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		private TextBox _txtScriptLog;
		private DispatcherTimer _timer;
		private ScriptWindowViewModel _model;

		static ScriptWindow()
		{
			using XmlReader reader = XmlReader.Create(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.HighlightLua.xshd")!);
			_syntaxDef = HighlightingLoader.LoadXshd(reader);
		}

		[Obsolete("For designer only")]
		public ScriptWindow() : this(new()) { }

		public ScriptWindow(ScriptWindowViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			UpdateSyntaxDef();
			_highlighting = HighlightingLoader.Load(_syntaxDef, HighlightingManager.Instance);

			_model = model;
			DataContext = model;
			_textEditor = this.FindControl<MesenTextEditor>("Editor");
			_txtScriptLog = this.FindControl<TextBox>("txtScriptLog");
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(200), DispatcherPriority.Normal, (s, e) => UpdateLog());

			if(Design.IsDesignMode) {
				return;
			}

			_model.InitActions(this);
			_model.Config.LoadWindowSettings(this);

			_textEditor.SyntaxHighlighting = _highlighting;
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
			_model.Config.SaveWindowSettings(this);
		}
		
		private void UpdateSyntaxDef()
		{
			Color[] colors = new Color[] { Colors.Green, Colors.SteelBlue, Colors.Blue, Colors.DarkMagenta, Colors.DarkRed, Colors.Black, Colors.Indigo };
			for(int i = 0; i < 7; i++) {
				((XshdColor)_syntaxDef.Elements[i]).Foreground = new SimpleHighlightingBrush(ColorHelper.GetColor(colors[i]));
			}
		}

		private void UpdateLog()
		{
			if(_model.ScriptId >= 0) {
				string log = DebugApi.GetScriptLog(_model.ScriptId);
				if(log != _model.Log) {
					_model.Log = log;
					_txtScriptLog.CaretIndex = Int32.MaxValue;
				}
			}
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					bool wasRunning = _model.ScriptId >= 0;
					_model.StopScript();
					if(wasRunning && _model.Config.AutoRestartScriptAfterPowerCycle) {
						_model.RunScript();
					}
					break;
			}
		}
	}
}
