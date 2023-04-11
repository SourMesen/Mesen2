using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.ViewModels;
using System;
using System.ComponentModel;
using Mesen.Config;
using Mesen.Debugger.ViewModels;
using Mesen.Utilities;
using Avalonia.Rendering;
using Mesen.Windows;
using Avalonia.Input;

namespace Mesen.Debugger.Windows
{
	public class DebuggerConfigWindow : MesenWindow
	{
		private DebuggerConfigWindowViewModel _model;
		private bool _promptToSave = true;

		[Obsolete("For designer only")]
		public DebuggerConfigWindow() : this(new())
		{
		}

		public DebuggerConfigWindow(DebuggerConfigWindowViewModel model)
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_model = model;
			DataContext = model;
		}

		public static void Open(DebugConfigWindowTab tab, IRenderRoot? parent)
		{
			new DebuggerConfigWindow(new DebuggerConfigWindowViewModel(tab)).ShowCenteredDialog(parent as Visual);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnClosed(EventArgs e)
		{
			base.OnClosed(e);
			_model.Dispose();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			_promptToSave = false;
			Close();
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			_promptToSave = false;
			_model.RevertChanges();
			Close();
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			base.OnKeyDown(e);
			if(e.Key == Key.Escape) {
				Close();
			}
		}

		private async void DisplaySaveChangesPrompt()
		{
			DialogResult result = await MesenMsgBox.Show(this, "PromptSaveChanges", MessageBoxButtons.YesNoCancel, MessageBoxIcon.Question);
			switch(result) {
				case DialogResult.Yes: _promptToSave = false; Close(); break;
				case DialogResult.No: _promptToSave = false; _model.RevertChanges(); Close(); break;
				default: break;
			}
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}

			if(_promptToSave && _model.IsDirty()) {
				e.Cancel = true;
				DisplaySaveChangesPrompt();
				return;
			}
		}
	}
}