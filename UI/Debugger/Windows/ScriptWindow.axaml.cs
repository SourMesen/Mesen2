using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Threading;
using AvaloniaEdit;
using AvaloniaEdit.CodeCompletion;
using AvaloniaEdit.Document;
using AvaloniaEdit.Editing;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using DynamicData;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.Views;
using Mesen.Interop;
using Mesen.Utilities;
using OmniSharp.Extensions.LanguageServer.Client;
using OmniSharp.Extensions.LanguageServer.Protocol.Client.Capabilities;
using OmniSharp.Extensions.LanguageServer.Protocol.Document;
using OmniSharp.Extensions.LanguageServer.Protocol.Models;
using ReactiveUI;
using System;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Reactive.Linq;
using System.Reflection;
using System.Threading.Tasks;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class ScriptWindow : MesenWindow, INotificationHandler
	{
		private static XshdSyntaxDefinition _syntaxDef;
		private static readonly string _luaMetaDefinition;
		private IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		private TextBox _txtScriptLog;
		private DispatcherTimer _timer;
		private ScriptWindowViewModel _model;
		private Process _lspServer = null!;
		private LanguageClient? _lspClient = null!;

		static ScriptWindow()
		{
			using XmlReader reader = XmlReader.Create(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.HighlightLua.xshd")!);
			_syntaxDef = HighlightingLoader.LoadXshd(reader);

			{
				using var stream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.Documentation.meta.lua")!;
				using var metaReader = new StreamReader(stream);
				_luaMetaDefinition = metaReader.ReadToEnd();
			}
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
			_textEditor.TextArea.KeyDown += TextArea_KeyDown;
			_textEditor.TextArea.KeyUp += TextArea_KeyUp;
			_textEditor.TextArea.TextEntering += TextArea_TextEntering;
			//_textEditor.TextArea.TextView.PointerMoved += TextView_PointerMoved;

			_txtScriptLog = this.GetControl<TextBox>("txtScriptLog");
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(200), DispatcherPriority.Normal, (s, e) => UpdateLog());

			if(Design.IsDesignMode) {
				return;
			}

			_model.InitActions(this);
			_model.Config.LoadWindowSettings(this);

			_textEditor.SyntaxHighlighting = _highlighting;

			#region Start Lsp

			var _ = InitializeLspClientAsync();

			#endregion
		}

		private async Task InitializeLspClientAsync()
		{
			_lspServer = new Process();
			_lspServer.StartInfo.FileName = LspServerHelper.ExecutableFullName;
			_lspServer.StartInfo.RedirectStandardInput = true;
			_lspServer.StartInfo.RedirectStandardOutput = true;
			_lspServer.StartInfo.CreateNoWindow = true;
			try {
				_lspServer.Start();
			} catch(Exception) {
				return;
			}

			_lspClient = LanguageClient.Create(options =>
				options
					.WithOutput(_lspServer.StandardInput.BaseStream)
					.WithInput(_lspServer.StandardOutput.BaseStream)
					.WithClientCapabilities(new() {
						TextDocument = new() {
							Completion = new CompletionCapability {
								CompletionItem = new() {
									InsertReplaceSupport = true,
								}
							}
						}
					})
			);

			// TODO handle for initializing lsp client
			await _lspClient.Initialize(default);
			_lspClient.DidOpenTextDocument(new() {
				TextDocument = new() {
					LanguageId = "lua",
					Text = _luaMetaDefinition,
					Uri = _model.GetMetaLuaUri(),
				}
			});

			// mouse hover tips
			Observable
				.FromEventPattern<PointerEventArgs>(
					_textEditor.TextArea.TextView,
					nameof(_textEditor.TextArea.TextView.PointerMoved))
				.ObserveOn(RxApp.MainThreadScheduler)
				.Select(p => _textEditor.TextArea.TextView.GetPosition(p.EventArgs.GetCurrentPoint(_textEditor.TextArea.TextView).Position + _textEditor.TextArea.TextView.ScrollOffset))
				.Where(p => p != null)
				.Select(p => (TextViewPosition)p!)
				.Distinct(p => p.Location) //! i don't know why this event is keeping emitted even when I don't move my mouse
				.Throttle(TimeSpan.FromSeconds(1))
				.SubscribeOn(RxApp.TaskpoolScheduler)
				.Subscribe(pos => HandleEditorHover(pos.Location.Line - 1, pos.Location.Column - 1));

			// tips on fly
			Observable
				.FromEventPattern<TextInputEventArgs>(
					_textEditor.TextArea,
					nameof(_textEditor.TextArea.TextEntered))
				.ObserveOn(RxApp.MainThreadScheduler)
				.SubscribeOn(RxApp.TaskpoolScheduler)
				.Where(x =>
					!string.IsNullOrWhiteSpace(x.EventArgs.Text)
					&& !x.EventArgs.Text.EndsWith(")"))
				.Throttle(TimeSpan.FromSeconds(0.5))
				.Subscribe(_ => ShowCompletions());
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
				_lspServer.Kill();
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
		private Hover? _previousHover;

		private Uri OpenCodeForLsp()
		{
			// TODO handle open/change/save/... for editing code
			var uri = _model.GetCodeUri();
			_lspClient?.DidOpenTextDocument(new() {
				TextDocument = new() { Uri = uri, Text = _model.Code },
			});
			return uri;
		}

		private void HandleEditorHover(int line, int column)
		{
			_ = Task.Run(async () => {
				if(_lspClient == null) return;

				var hoverResult = await _lspClient.RequestHover(new() {
					TextDocument = new() { Uri = OpenCodeForLsp() },
					Position = new(line, column)
				});

				if(_previousHover == null
				// TODO identify whether updated
				|| !string.Equals(
					_previousHover.Contents.MarkupContent?.ToString(),
					hoverResult?.Contents.MarkupContent?.ToString())
				) {
					Dispatcher.UIThread.Post(() => {
						if(hoverResult != null) {
							TooltipHelper.ShowTooltip(
								_textEditor.TextArea.TextView,
								new ScriptCodeHoverView() { DataContext = hoverResult.Contents.MarkupContent },
								10);
						} else {
							TooltipHelper.HideTooltip(_textEditor.TextArea.TextView);
						}
						_previousHover = hoverResult;
					});
				}
			});
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

				ShowCompletions();
			}

			// Do not set e.Handled=true.
			// We still want to insert the character that was typed.
		}

		private void ShowCompletions()
		{
			_ = Task.Run(async () => {
				if(_lspClient == null) return;

				var completions = await _lspClient.RequestCompletion(new() {
					TextDocument = new() { Uri = OpenCodeForLsp() },
					Position = new(_textEditor.TextArea.Caret.Line - 1, _textEditor.TextArea.Caret.Column - 1)
				});
				if(completions is null) return;

				Dispatcher.UIThread.Post(() => {
					_completionWindow = new CompletionWindow(_textEditor.TextArea);
					_completionWindow
						.CompletionList
						.CompletionData
						.AddRange(completions.Select(x => new LspCompletionData(x, async () => await _lspClient.ResolveCompletion(x))));
					_completionWindow.Closed += (sender, e) => _completionWindow = null;
					_completionWindow.Show();
				});
			});
		}

		public class LspCompletionData : ICompletionData
		{
			private readonly CompletionItem _completionItem;
			private CompletionItem? _completionDetailed = null;
			private Func<CompletionItem> _detailsResolver;
			private CompletionItem CompletionDetailed
			{
				get
				{
					if(_completionDetailed == null) {
						_completionDetailed = _detailsResolver() ?? _completionItem;
					}
					return _completionDetailed;
				}
			}

			public LspCompletionData(CompletionItem completionItem, Func<Task<CompletionItem>> detailsResolver)
			{
				_completionItem = completionItem;
				_detailsResolver = () => {
					// TODO clean up code
					var task = Task.Run(async () => await detailsResolver());
					task.Wait();
					return task.Result;
				};
			}

			// TODO handle image for kinds except enum and function
			public IBitmap Image => ImageUtilities.BitmapFromAsset(_completionItem.Kind == CompletionItemKind.Function ? "Assets/Function.png" : "Assets/Enum.png")!;
			public string Text => _completionItem.Label;
			public object Content => new TextBlock { Text = _completionItem.Label };
			public object Description => new ScriptCodeHoverView {
				DataContext = CompletionDetailed.Documentation?.MarkupContent,
				MaxHeight = 800,
				MaxWidth = 400
			};
			public double Priority
			{
				get
				{
					if(double.TryParse(_completionItem.SortText, out var priority))
						return priority;

					return 0;
				}
			}

			public void Complete(TextArea textArea, ISegment completionSegment, EventArgs insertionRequestEventArgs)
			{
				// TODO CompletionDetailed.AdditionalTextEdits; CompletionDetailed.TextEdit; may be better.
				// However LSP server doesn't response with these fields and I don't know reason yet.
				// See `textEdit?: TextEdit | InsertReplaceEdit;` from https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#completionItem
				// May be helpful.

				var toInsert = CompletionDetailed.InsertText ?? _completionItem.Label;
				var caretPosition = textArea.Caret.Position;
				int offset = completionSegment.Offset;
				int length = completionSegment.Length;
				if(completionSegment.Length == 0) {
					length = 1;

					// find the real offset and length to replace existing partial code.
					int newLength = 0;
					for(; completionSegment.Offset - length > textArea.Document.GetOffset(caretPosition.Line, 1); length++) {
						if(toInsert.StartsWith(textArea.Document.GetText(completionSegment.Offset - length, length))) {
							// match as long as much
							newLength = Math.Max(newLength, length);
						}
					}

					length = newLength;
					offset = completionSegment.Offset - length;
				}

				textArea.Document.Replace(offset, length, toInsert);
			}
		}

	}
}
