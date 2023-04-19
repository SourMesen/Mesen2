using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Mesen.Debugger.Labels;
using Mesen.Debugger.ViewModels;
using Mesen.Interop;
using Mesen.Utilities;
using System;

namespace Mesen.Debugger.Windows
{
	public class CommentEditWindow : MesenWindow
	{
		private CommentEditViewModel _model;
		
		[Obsolete("For designer only")]
		public CommentEditWindow() : this(new()) { }

		public CommentEditWindow(CommentEditViewModel model)
		{
			InitializeComponent();

			DataContext = model;
			_model = model;
			
			AddHandler(CommentEditWindow.KeyDownEvent, this.KeyDownHandler, RoutingStrategies.Tunnel);

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
			this.GetControl<TextBox>("txtComment").Focus();
			this.GetControl<TextBox>("txtComment").CaretIndex = Int32.MaxValue;
		}

		private void KeyDownHandler(object? sender, KeyEventArgs e)
		{
			if(e.Key == Key.Enter) {
				e.Handled = true;
				if(e.KeyModifiers == KeyModifiers.Shift) {
					TextBox txt = this.GetControl<TextBox>("txtComment");
					string comment = _model.Label.Comment;
					int caret = txt.CaretIndex;
					_model.Label.Comment = comment.Substring(0, caret) + Environment.NewLine + comment.Substring(caret);
					txt.CaretIndex++;
				} else {
					Close(true);
				}
			}
			base.OnKeyDown(e);
		}

		public static async void EditComment(Control parent, CodeLabel label)
		{
			CommentEditViewModel model;
			CodeLabel? copy = null;
			if(LabelManager.ContainsLabel(label)) {
				copy = label.Clone();
				model = new CommentEditViewModel(copy);
			} else {
				model = new CommentEditViewModel(label);
			}

			CommentEditWindow wnd = new CommentEditWindow(model);

			bool result = await wnd.ShowCenteredDialog<bool>(parent);
			if(result) {
				model.Label.Commit();
				if(string.IsNullOrWhiteSpace(model.Label.Comment) && string.IsNullOrWhiteSpace(model.Label.Label)) {
					//No comment and no label, delete the label completely
					LabelManager.DeleteLabel(label, true);
				} else {
					LabelManager.DeleteLabel(label, false);
					LabelManager.SetLabel(copy ?? label, true);
				}
			}
		}

		private void Ok_OnClick(object sender, RoutedEventArgs e)
		{
			Close(true);
		}

		private void Cancel_OnClick(object sender, RoutedEventArgs e)
		{
			Close(false);
		}
	}
}
