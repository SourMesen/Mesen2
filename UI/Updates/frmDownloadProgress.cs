using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Net;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Updates
{
	public partial class frmDownloadProgress : BaseForm
	{
		private string _link;
		private string _filename;
		private bool _cancel = false;

		public frmDownloadProgress(string link, string filename)
		{
			InitializeComponent();

			_link = link;
			_filename = filename;

			try {
				File.Delete(_filename);
			} catch {}

			lblFilename.Text = link;

			tmrStart.Start();
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			_cancel = true;
			base.OnClosing(e);
		}

		private void tmrStart_Tick(object sender, EventArgs e)
		{
			tmrStart.Stop();
			
			DialogResult result = DialogResult.None;
	
			Task.Run(() => {
				using(var client = new WebClient()) {
					client.DownloadProgressChanged += (object s, DownloadProgressChangedEventArgs args) => {
						this.BeginInvoke((Action)(() => {
							lblFilename.Text = string.Format("{0} ({1:0.00}Mb)", _link, (double)args.TotalBytesToReceive/1024/1024);
							progressDownload.Value = args.ProgressPercentage;
						}));
					};
					client.DownloadFileCompleted += (object s, AsyncCompletedEventArgs args) => {
						if(!args.Cancelled && args.Error == null && File.Exists(_filename)) {
							result = DialogResult.OK;
						} else if(args.Error != null) {
							MesenMsgBox.Show("UnableToDownload", MessageBoxButtons.OK, MessageBoxIcon.Error, args.Error.ToString());
							result = DialogResult.Cancel;
						}
					};

					Task downloadTask = null;
					try {
						downloadTask = client.DownloadFileTaskAsync(_link, _filename);
					} catch(Exception ex) {
						MesenMsgBox.Show("UnableToDownload", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
						result = DialogResult.Cancel;
					}

					if(downloadTask == null) {
						result = DialogResult.Cancel;
					} else {
						while(!downloadTask.IsCompleted && !_cancel) {
							System.Threading.Thread.Sleep(200);
						}

						if(_cancel) {
							client.CancelAsync();
						} else if(result ==  DialogResult.None) {
							result = DialogResult.OK;
						}
					}
				}

				//Wait a bit for the progress bar to update to 100% (display updates are slower than the .Value updates)
				System.Threading.Thread.Sleep(500);
				this.BeginInvoke((Action)(() => {
					DialogResult = result;
					this.Close();
				}));
			});
		}
	}
}
