using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Config;
using Mesen.Controls;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.Windows
{
	public class MemoryViewerFindWindow : Window
	{
		private MemoryViewerFindViewModel _model;
		private MemoryToolsViewModel _viewerModel;

		[Obsolete("For designer only")]
		public MemoryViewerFindWindow() : this(new(), null!) { }

		public MemoryViewerFindWindow(MemoryViewerFindViewModel model, MemoryToolsViewModel viewerModel)
		{
			DataContext = model;
			_model = model;
			_viewerModel = viewerModel;

			Activated += MemorySearchWindow_Activated;

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
			this.GetControl<TextBox>("txtValue").Focus();
			this.GetControl<TextBox>("txtValue").SelectAll();

			DebugShortcutManager.RegisterActions(this, new List<ContextMenuAction>() {
				new ContextMenuAction() {
					ActionType = ActionType.FindPrev,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindPrev),
					OnClick = () => _viewerModel.Find(SearchDirection.Backward)
				},
				new ContextMenuAction() {
					ActionType = ActionType.FindNext,
					Shortcut = () => ConfigManager.Config.Debug.Shortcuts.Get(DebuggerShortcut.FindNext),
					OnClick = () => _viewerModel.Find(SearchDirection.Forward)
				},
			});
		}

		private void MemorySearchWindow_Activated(object? sender, EventArgs e)
		{
			this.GetControl<TextBox>("txtValue").Focus();
			this.GetControl<TextBox>("txtValue").SelectAll();
		}

		private void FindPrev_OnClick(object sender, RoutedEventArgs e)
		{
			_viewerModel.Find(SearchDirection.Backward);
		}

		private void FindNext_OnClick(object sender, RoutedEventArgs e)
		{
			_viewerModel.Find(SearchDirection.Forward);
		}

		private void Close_OnClick(object sender, RoutedEventArgs e)
		{
			Close();
		}
	}
}
