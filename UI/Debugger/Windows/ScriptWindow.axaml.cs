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
using System.Linq;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class ScriptWindow : MesenWindow, INotificationHandler
	{
		private static XshdSyntaxDefinition _syntaxDef;
		private IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		private MesenTextEditor _txtScriptLog;
		private DispatcherTimer _timer;
		private ScriptWindowViewModel _model;
		
		public ScriptWindowViewModel Model => _model;

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
			_textEditor = this.GetControl<MesenTextEditor>("Editor");

			ColorHelper.InvalidateControlOnThemeChange(_textEditor, () => {
				UpdateSyntaxDef();
				_highlighting = HighlightingLoader.Load(_syntaxDef, HighlightingManager.Instance);
				_textEditor.SyntaxHighlighting = _highlighting;
			});

			_textEditor.TextArea.KeyDown += TextArea_KeyDown;
			_textEditor.TextArea.KeyUp += TextArea_KeyUp;
			_textEditor.TextArea.TextEntered += TextArea_TextEntered;
			_textEditor.TextArea.TextEntering += TextArea_TextEntering;
			_textEditor.TextArea.TextView.PointerMoved += TextView_PointerMoved;

			_txtScriptLog = this.GetControl<MesenTextEditor>("txtScriptLog");
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
			base.OnOpened(e);
			if(Design.IsDesignMode) {
				return;
			}

			_textEditor.Focus();
			_textEditor.TextArea.Focus();
			_timer.Start();
		}

		private bool _needCloseValidation = true;

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);

			if(Design.IsDesignMode) {
				return;
			}

			if(_needCloseValidation) {
				e.Cancel = true;
				ValidateExit();
			} else {
				_model.StopScript();
				_timer.Stop();
				_model.Config.SaveWindowSettings(this);
			}
		}

		private async void ValidateExit()
		{
			if(await _model.SavePrompt()) {
				_needCloseValidation = false;
				Close();
			}
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
					_txtScriptLog.ScrollToEnd();
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
					if(_model.ScriptId >= 0 && _model.Config.AutoRestartScriptAfterPowerCycle) {
						_model.RestartScript();
					} else {
						_model.StopScript();
					}
					break;
			}
		}

		private CompletionWindow? _completionWindow;
		private bool _ctrlPressed;
		private DocEntryViewModel? _prevTooltipEntry;

		private void TextView_PointerMoved(object? sender, PointerEventArgs e)
		{
			TextViewPosition? posResult = _textEditor.TextArea.TextView.GetPosition(e.GetCurrentPoint(_textEditor.TextArea.TextView).Position + _textEditor.TextArea.TextView.ScrollOffset);
			if(posResult is TextViewPosition pos) {
				int offset = _textEditor.TextArea.Document.GetOffset(pos.Location.Line, pos.Location.Column);
				DocEntryViewModel? entry = GetTooltipEntry(offset);
				if(_prevTooltipEntry != entry) {
					if(entry != null) {
						TooltipHelper.ShowTooltip(_textEditor.TextArea.TextView, new ScriptCodeCompletionView() { DataContext = entry }, 10);
					} else {
						TooltipHelper.HideTooltip(_textEditor.TextArea.TextView);
					}
					_prevTooltipEntry = entry;
				}
			} else {
				TooltipHelper.HideTooltip(_textEditor.TextArea.TextView);
				_prevTooltipEntry = null;
			}
		}

		private DocEntryViewModel? GetTooltipEntry(int offset)
		{
			if(offset >= _model.Code.Length || _model.Code[offset] == '\r' || _model.Code[offset] == '\n') {
				//End of line/document, close tooltip
				return null;
			}

			//Find the end of the expression
			for(; offset < _model.Code.Length; offset++) {
				if(!char.IsLetterOrDigit(_model.Code[offset]) && _model.Code[offset] != '.' && _model.Code[offset] != '_') {
					break;
				}
			}

			//Find the start of the expression
			int i = offset - 1;
			for(; i >= 0 && i < _model.Code.Length; i--) {
				if(!char.IsLetterOrDigit(_model.Code[i]) && _model.Code[i] != '.' && _model.Code[i] != '_') {
					break;
				}
			}

			string expr = _model.Code.Substring(i + 1, offset - i - 1);
			if(expr.StartsWith("emu.")) {
				expr = expr.Substring(4);
				bool hasTrailingDot = expr.EndsWith(".");
				if(hasTrailingDot) {
					expr = expr.Substring(0, expr.Length - 1);
				}

				DocEntryViewModel? entry = null;
				if(expr.Contains(".")) {
					string[] parts = expr.Split('.');
					entry = CodeCompletionHelper.GetEntry(parts[0]);
					if(parts.Length == 2 && entry != null && entry.EnumValues.Count > 0) {
						return entry;
					}
				}

				return CodeCompletionHelper.GetEntry(expr);
			}

			return null;
		}

		private void TextArea_KeyUp(object? sender, KeyEventArgs e)
		{
			if(e.Key == Key.LeftCtrl || e.Key == Key.RightCtrl) {
				_ctrlPressed = false;
			}
		}

		private void TextArea_KeyDown(object? sender, KeyEventArgs e)
		{
			if(e.Key == Key.LeftCtrl || e.Key == Key.RightCtrl || e.KeyModifiers.HasFlag(KeyModifiers.Control)) {
				_ctrlPressed = true;
			}
		}

		private void TextArea_TextEntering(object? sender, TextInputEventArgs e)
		{
			if(e.Text?.Length > 0 && _completionWindow != null) {
				if(!char.IsLetterOrDigit(e.Text[0])) {
					// Whenever a non-letter is typed while the completion window is open,
					// insert the currently selected element.
					_completionWindow.CompletionList.RequestInsertion(e);
				}
			}

			if(_ctrlPressed && e.Text == " ") {
				//Don't type the space if pressing ctrl+space
				e.Handled = true;

				OpenCompletionWindow();
			}

			// Do not set e.Handled=true.
			// We still want to insert the character that was typed.
		}

		private void TextArea_TextEntered(object? sender, TextInputEventArgs e)
		{
			if(e.Text == ".") {
				OpenCompletionWindow();
			}
		}

		private void OpenCompletionWindow()
		{
			int offset = _textEditor.TextArea.Caret.Offset;
			//Find the start of the expression
			int i = offset - 1;
			for(; i >= 0 && i < _model.Code.Length; i--) {
				if(!char.IsLetterOrDigit(_model.Code[i]) && _model.Code[i] != '.' && _model.Code[i] != '_') {
					break;
				}
			}

			string expr = _model.Code.Substring(i + 1, offset - i - 1);
			if(expr.StartsWith("emu.")) {
				expr = expr.Substring(4);
				bool hasTrailingDot = expr.EndsWith(".");
				if(hasTrailingDot) {
					expr = expr.Substring(0, expr.Length - 1);
				}

				DocEntryViewModel? entry = null;
				if(expr.Contains(".")) {
					string[] parts = expr.Split('.');
					entry = CodeCompletionHelper.GetEntry(parts[0]);
					if(parts.Length == 2 && entry != null && entry.EnumValues.Count > 0) {
						OpenCompletionWindow(entry.EnumValues.Select(x => x.Name), expr, parts[1], parts[1].Length);
						return;
					}
				}

				entry = CodeCompletionHelper.GetEntry(expr);
				if(entry != null && entry.EnumValues.Count > 0 && hasTrailingDot) {
					OpenCompletionWindow(entry.EnumValues.Select(x => x.Name), expr, "", 0);
				} else {
					if(!hasTrailingDot) {
						OpenCompletionWindow(CodeCompletionHelper.GetEntries(), null, expr, expr.Length);
					}
				}
			}
		}

		private void OpenCompletionWindow(IEnumerable<string> entries, string? enumName, string defaultFilter, int insertOffset)
		{
			_completionWindow = new CompletionWindow(_textEditor.TextArea);
			IList<ICompletionData> data = _completionWindow.CompletionList.CompletionData;
			foreach(string name in entries) {
				data.Add(new MyCompletionData(name, enumName, -insertOffset));
			}
			_completionWindow.Closed += delegate {
				_completionWindow = null;
			};
			_completionWindow.Show();
			if(defaultFilter.Length > 0) {
				_completionWindow.CompletionList.SelectItem(defaultFilter);
			}
		}

		public class MyCompletionData : ICompletionData
		{
			private string? _enumName;
			private int _insertOffset;

			public MyCompletionData(string text, string? enumName = null, int insertOffset = 0)
			{
				Text = text;
				_enumName = enumName;
				_insertOffset = insertOffset;
			}

			public IImage Image
			{
				get
				{
					if(_enumName != null) {
						return ImageUtilities.BitmapFromAsset("Assets/Enum.png")!;
					} else {
						return ImageUtilities.BitmapFromAsset(CodeCompletionHelper.GetEntry(Text)?.EnumValues.Count > 0 ? "Assets/Enum.png" : "Assets/Function.png")!;
					}
				}
			}

			public string Text { get; private set; }

			public object Content
			{
				get { return new TextBlock() { Text = this.Text }; }
			}

			public object Description
			{
				get 
				{
					if(_enumName != null) {
						DocEntryViewModel? enumEntry = CodeCompletionHelper.GetEntry(_enumName);
						if(enumEntry != null) {
							foreach(DocEnumValue val in enumEntry.EnumValues) {
								if(val.Name == Text) {
									return val.Description;
								}
							}
						}
					} else {
						DocEntryViewModel? entry = CodeCompletionHelper.GetEntry(Text);
						if(entry != null) {
							return new ScriptCodeCompletionView() { DataContext = entry };
						}
					}
					return null!;
				}
			}

			public double Priority => 1.0;

			public void Complete(TextArea textArea, ISegment completionSegment, EventArgs insertionRequestEventArgs)
			{
				textArea.Document.Replace(completionSegment.Offset + _insertOffset, completionSegment.Length - _insertOffset, this.Text);
			}
		}
	}
}
