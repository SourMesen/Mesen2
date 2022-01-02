using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Localization;
using Mesen.Utilities;
using System.Threading.Tasks;

namespace Mesen.Windows
{
	public partial class MessageBox : Window
	{
		public MessageBox()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		public static Task<DialogResult> Show(Window? parent, string text, string title, MessageBoxButtons buttons, MessageBoxIcon icon)
		{
			DialogResult result = DialogResult.OK;
			MessageBox msgbox = new MessageBox() { Title = title };
			msgbox.FindControl<TextBlock>("Text").Text = text;
			
			switch(icon) {
				case MessageBoxIcon.Error: msgbox.FindControl<Image>("imgError").IsVisible = true; break;
				case MessageBoxIcon.Warning: msgbox.FindControl<Image>("imgWarning").IsVisible = true; break;
				case MessageBoxIcon.Question: msgbox.FindControl<Image>("imgQuestion").IsVisible = true; break;
			}


			StackPanel buttonPanel = msgbox.FindControl<StackPanel>("pnlButtons");
			void AddButton(string caption, DialogResult r)
			{
				Button btn = new Button { Content = caption };
				btn.Click += (_, _) => {
					result = r;
					msgbox.Close();
				};
				buttonPanel.Children.Add(btn);
			}

			if(buttons == MessageBoxButtons.OK || buttons == MessageBoxButtons.OKCancel) {
				AddButton(ResourceHelper.GetMessage("btnOK"), DialogResult.OK);
				result = DialogResult.OK;
			}

			if(buttons == MessageBoxButtons.YesNo || buttons == MessageBoxButtons.YesNoCancel) {
				AddButton(ResourceHelper.GetMessage("btnYes"), DialogResult.Yes);
				AddButton(ResourceHelper.GetMessage("btnNo"), DialogResult.No);
				result = DialogResult.No;
			}

			if(buttons == MessageBoxButtons.OKCancel || buttons == MessageBoxButtons.YesNoCancel) {
				AddButton(ResourceHelper.GetMessage("btnCancel"), DialogResult.Cancel);
				result = DialogResult.Cancel;
			}

			TaskCompletionSource<DialogResult> tcs = new TaskCompletionSource<DialogResult>();
			msgbox.Closed += (_, _) => { tcs.TrySetResult(result); };
			
			if(parent != null) {
				msgbox.WindowStartupLocation = WindowStartupLocation.CenterOwner;
				msgbox.ShowDialog(parent);
			} else {
				msgbox.Show();
			}
			return tcs.Task;
		}
	}

	public enum MessageBoxButtons
	{
		OK,
		OKCancel,
		YesNo,
		YesNoCancel
	}

	public enum MessageBoxIcon
	{
		Error,
		Warning,
		Question
	}

	public enum DialogResult
	{
		OK,
		Cancel,
		Yes,
		No
	}
}
