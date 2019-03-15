using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmCopyFiles : Form
	{
		private volatile int _fileCount = -1;
		private volatile int _filesCopied = 0;
		private List<string> _sourceFiles = new List<string>();
		private List<string> _targetFiles = new List<string>();
		public Exception Exception { get; set; }

		public frmCopyFiles(string source, string target)
		{
			InitializeComponent();

			tmrProgress.Start();
			Task.Run(() => {
				GetFilesToCopy(source, target, _sourceFiles, _targetFiles);
				_fileCount = _sourceFiles.Count;
				
				for(int i = 0; i < _sourceFiles.Count; i++) {
					try {
						File.Copy(_sourceFiles[i], _targetFiles[i], true);
					} catch(Exception ex) {
						Exception = ex;
						break;
					}
					_filesCopied++;
				}
			});
		}
		
		private void GetFilesToCopy(string source, string target, List<string> sourceFiles, List<string> targetFiles)
		{
			if(!Directory.Exists(target)) {
				Directory.CreateDirectory(target);
			}

			foreach(string file in Directory.GetFiles(source)) {
				if(Path.GetExtension(file).ToLower() != ".exe" && Path.GetExtension(file).ToLower() != ".dll") {
					sourceFiles.Add(file);
					targetFiles.Add(Path.Combine(target, Path.GetFileName(file)));
				}
			}

			foreach(string subdir in Directory.GetDirectories(source)) {
				GetFilesToCopy(subdir, Path.Combine(target, Path.GetDirectoryName(subdir)), sourceFiles, targetFiles);
			}
		}

		protected override void OnFormClosing(FormClosingEventArgs e)
		{
			if(_filesCopied < _fileCount && Exception == null) {
				e.Cancel = true;
			}
			base.OnFormClosing(e);
		}

		private void tmrProgress_Tick(object sender, EventArgs e)
		{
			if(_fileCount >= 0) {
				pbProgress.Maximum = _fileCount + 1;
				pbProgress.Value = _filesCopied + 1;
				pbProgress.Value = _filesCopied;
				if(_sourceFiles.Count > _filesCopied) {
					lblTarget.Text = Path.GetFileName(_sourceFiles[_filesCopied]);
				}

				if(_filesCopied >= _fileCount || Exception != null) {
					tmrProgress.Stop();
					this.Close();
				}
			}
		}
	}
}
