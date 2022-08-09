using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Styling;
using Avalonia.Threading;
using AvaloniaEdit;
using AvaloniaEdit.CodeCompletion;
using AvaloniaEdit.Document;
using AvaloniaEdit.Editing;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Views;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
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
			_textEditor.TextArea.TextEntered += TextArea_TextEntered;
			_textEditor.TextArea.TextEntering += TextArea_TextEntering;

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

			_textEditor.Focus();
			_textEditor.TextArea.Focus();
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


		private CompletionWindow? _completionWindow;
		private void TextArea_TextEntering(object? sender, TextInputEventArgs e)
		{
			if(e.Text?.Length > 0 && _completionWindow != null) {
				if(!char.IsLetterOrDigit(e.Text[0])) {
					// Whenever a non-letter is typed while the completion window is open,
					// insert the currently selected element.
					_completionWindow.CompletionList.RequestInsertion(e);
				}
			}
			// Do not set e.Handled=true.
			// We still want to insert the character that was typed.
		}

		private void TextArea_TextEntered(object? sender, TextInputEventArgs e)
		{
			if(e.Text == ".") {
				int offset = _textEditor.TextArea.Caret.Offset;
				if(offset >= 4 && _model.Code.Substring(offset - 4, 4) == "emu.") {
					// Open code completion after the user has pressed dot:
					_completionWindow = new CompletionWindow(_textEditor.TextArea);
					IList<ICompletionData> data = _completionWindow.CompletionList.CompletionData;
					foreach(string name in CodeCompletionHelper.GetEntries()) {
						data.Add(new MyCompletionData(name));
					}
					_completionWindow.Show();

					_completionWindow.Closed += delegate {
						_completionWindow = null;
					};
				}
			}
		}

		public class MyCompletionData : ICompletionData
		{
			public MyCompletionData(string text)
			{
				this.Text = text;
			}

			public IBitmap Image
			{
				get { return ImageUtilities.BitmapFromAsset("Assets/Function.png")!; }
			}

			public string Text { get; private set; }

			// Use this property if you want to show a fancy UIElement in the list.
			public object Content
			{
				get { return new TextBlock() { Text = this.Text }; }
			}

			public object Description
			{
				get 
				{
					ScriptCodeCompletionView view = new();
					view.DataContext = CodeCompletionHelper.GetEntry(Text);
					return view;
				}
			}

			public double Priority => 1.0;

			public void Complete(TextArea textArea, ISegment completionSegment, EventArgs insertionRequestEventArgs)
			{
				textArea.Document.Replace(completionSegment, this.Text);
			}
		}
	}
}
