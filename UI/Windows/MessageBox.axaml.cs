using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Mesen.Localization;
using Mesen.Utilities;
using System;
using System.Threading.Tasks;

namespace Mesen.Windows
{
	public partial class MessageBox : MesenWindow
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
			msgbox.GetControl<TextBlock>("Text").Text = text;
			
			switch(icon) {
				case MessageBoxIcon.Error: msgbox.GetControl<Image>("imgError").IsVisible = true; break;
				case MessageBoxIcon.Warning: msgbox.GetControl<Image>("imgWarning").IsVisible = true; break;
				case MessageBoxIcon.Question: msgbox.GetControl<Image>("imgQuestion").IsVisible = true; break;
				case MessageBoxIcon.Info: msgbox.GetControl<Image>("imgInfo").IsVisible = true; break;
			}

			StackPanel buttonPanel = msgbox.GetControl<StackPanel>("pnlButtons");
			void AddButton(string caption, DialogResult r)
			{
				Button btn = new Button { Content = caption };
				if(r == DialogResult.Cancel || (r == DialogResult.No && buttons != MessageBoxButtons.YesNoCancel) || (r == DialogResult.OK && buttons == MessageBoxButtons.OK)) {
					btn.IsCancel = true;
				}
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

			parent ??= ApplicationHelper.GetActiveOrMainWindow();

			if(parent != null) {
				if(!OperatingSystem.IsWindows()) {
					//TODOv2 - This fixes Avalonia apparently not working properly with CenterOwner on X11 with SizeToContent="WidthAndHeight"
					msgbox.Opened += (_, _) => { WindowExtensions.CenterWindow(msgbox, parent); };
				}

				msgbox.WindowStartupLocation = WindowStartupLocation.CenterOwner;
				msgbox.ShowDialog(parent);
			} else {
				Console.WriteLine(title + " - " + text);
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
		Question,
		Info
	}

	public enum DialogResult
	{
		OK,
		Cancel,
		Yes,
		No
	}
}
