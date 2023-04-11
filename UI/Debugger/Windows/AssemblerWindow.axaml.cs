using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Threading;
using AvaloniaEdit;
using AvaloniaEdit.Highlighting;
using AvaloniaEdit.Highlighting.Xshd;
using DataBoxControl;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;
using System.ComponentModel;
using System.Reflection;
using System.Xml;

namespace Mesen.Debugger.Windows
{
	public class AssemblerWindow : MesenWindow, INotificationHandler
	{
		private static XshdSyntaxDefinition _syntaxDef;
		private IHighlightingDefinition _highlighting;
		private MesenTextEditor _textEditor;
		private MesenTextEditor _hexView;
		private AssemblerWindowViewModel _model;

		static AssemblerWindow()
		{
			using XmlReader reader = XmlReader.Create(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.HighlightAssembly.xshd")!);
			_syntaxDef = HighlightingLoader.LoadXshd(reader);
		}

		[Obsolete("For designer only")]
		public AssemblerWindow() : this(new()) { }

		public AssemblerWindow(AssemblerWindowViewModel model)
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
			_hexView = this.GetControl<MesenTextEditor>("HexView");

			if(Design.IsDesignMode) {
				return;
			}

			_textEditor.TemplateApplied += textEditor_TemplateApplied;
			_hexView.TemplateApplied += hexView_TemplateApplied;
			
			_textEditor.SyntaxHighlighting = _highlighting;

			_model.Config.LoadWindowSettings(this);
			_model.InitMenu(this);
		}
		
		private void UpdateSyntaxDef()
		{
			((XshdColor)_syntaxDef.Elements[0]).Foreground = new SimpleHighlightingBrush(ColorHelper.GetColor(ConfigManager.Config.Debug.Debugger.CodeCommentColor));
			((XshdColor)_syntaxDef.Elements[1]).Foreground = new SimpleHighlightingBrush(ColorHelper.GetColor(ConfigManager.Config.Debug.Debugger.CodeImmediateColor));
			((XshdColor)_syntaxDef.Elements[2]).Foreground = new SimpleHighlightingBrush(ColorHelper.GetColor(ConfigManager.Config.Debug.Debugger.CodeOpcodeColor));
			((XshdColor)_syntaxDef.Elements[3]).Foreground = new SimpleHighlightingBrush(ColorHelper.GetColor(ConfigManager.Config.Debug.Debugger.CodeAddressColor));
			((XshdColor)_syntaxDef.Elements[4]).Foreground = new SimpleHighlightingBrush(ColorHelper.GetColor(ConfigManager.Config.Debug.Debugger.CodeLabelDefinitionColor));
		}

		public static void EditCode(CpuType cpuType, int address, string code, int byteCount)
		{
			AssemblerWindowViewModel model = new AssemblerWindowViewModel(cpuType);
			model.InitEditCode(address, code, byteCount);

			DebugWindowManager.OpenDebugWindow(() => new AssemblerWindow(model));
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);
			_textEditor.Focus();
			_textEditor.TextArea.Focus();
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			_model.Config.SaveWindowSettings(this);
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

		private void OnCellClick(DataBoxCell cell)
		{
			int lineNumber = (((AssemblerError?)cell.DataContext)?.LineNumber ?? 0)- 1;
			if(lineNumber >= 0) {
				_textEditor.TextArea.Caret.Line = lineNumber;
				_textEditor.TextArea.Caret.Column = 0;
				_textEditor.ScrollLineToTop(lineNumber);
			}
		}

		private async void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			if(await _model.ApplyChanges(this)) {
				Close();
			}
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public void ProcessNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.GameLoaded:
					RomInfo romInfo = EmuApi.GetRomInfo();
					if(!romInfo.CpuTypes.Contains(_model.CpuType)) {
						Dispatcher.UIThread.Post(() => {
							Close();
						});
					}
					break;
			}
		}
	}
}
