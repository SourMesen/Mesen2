using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;

namespace Mesen.Debugger.Windows
{
	public class LabelEditWindow : MesenWindow
	{
		private LabelEditViewModel _model;
		
		[Obsolete("For designer only")]
		public LabelEditWindow() : this(new()) { }

		public LabelEditWindow(LabelEditViewModel model)
		{
			InitializeComponent();

			DataContext = model;
			_model = model;

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
			this.GetControl<TextBox>("txtLabel").FocusAndSelectAll();
		}

		public static async void EditLabel(CpuType cpuType, Control parent, CodeLabel label)
		{
			LabelEditViewModel model;
			CodeLabel? copy = null;
			if(LabelManager.ContainsLabel(label)) {
				copy = label.Clone();
				model = new LabelEditViewModel(cpuType, copy, label);
			} else {
				model = new LabelEditViewModel(cpuType, label, null);
			}

			LabelEditWindow wnd = new LabelEditWindow(model);

			bool result = await wnd.ShowCenteredDialog<bool>(parent);
			if(result) {
				model.Commit();
				LabelManager.DeleteLabel(label, false);
				LabelManager.SetLabel(copy ?? label, true);
				DebugWorkspaceManager.AutoSave();
			}

			model.Dispose();
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}

		private void Delete_OnClick(object sender, RoutedEventArgs e)
		{
			_model.DeleteLabel();
			Close(false);
		}
	}
}
