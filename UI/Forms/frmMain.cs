using Mesen.GUI.Config;
using Mesen.GUI.Debugger;
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

namespace Mesen.GUI.Forms
{
	public partial class frmMain : BaseForm
	{
		public frmMain(string[] args)
		{
			InitializeComponent();
			ResourceHelper.LoadResources(Language.English);
		}

		protected override void OnShown(EventArgs e)
		{
			base.OnShown(e);

			EmuApi.InitDll();
			EmuApi.InitializeEmu(ConfigManager.HomeFolder, Handle, ctrlRenderer.Handle, false, false, false);
		}

		private void mnuTraceLogger_Click(object sender, EventArgs e)
		{
			frmTraceLogger frm = new frmTraceLogger();
			frm.Show();
		}

		private void mnuMemoryTools_Click(object sender, EventArgs e)
		{
			frmMemoryTools frm = new frmMemoryTools();
			frm.Show();
		}

		private void mnuStep_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1);
		}

		private void mnuOpen_Click(object sender, EventArgs e)
		{
			using(OpenFileDialog ofd = new OpenFileDialog()) {
				ofd.Filter = ResourceHelper.GetMessage("FilterRom");
				if(ofd.ShowDialog() == DialogResult.OK) {
					LoadFile(ofd.FileName);
				}
			}
		}

		private void mnuRun_Click(object sender, EventArgs e)
		{
			DebugApi.ResumeExecution();
		}

		private void mnuRun100Instructions_Click(object sender, EventArgs e)
		{
			DebugApi.Step(1000);
		}

		private void LoadFile(string filepath)
		{
			EmuApi.LoadRom(filepath);
			Task.Run(() => {
				EmuApi.Run();
			});
		}

		protected override void OnDragDrop(DragEventArgs e)
		{
			base.OnDragDrop(e);

			try {
				string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
				if(File.Exists(files[0])) {
					LoadFile(files[0]);
					this.Activate();
				} else {
					//InteropEmu.DisplayMessage("Error", "File not found: " + files[0]);
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}

		protected override void OnDragEnter(DragEventArgs e)
		{
			base.OnDragEnter(e);

			try {
				if(e.Data != null && e.Data.GetDataPresent(DataFormats.FileDrop)) {
					e.Effect = DragDropEffects.Copy;
				} else {
					//InteropEmu.DisplayMessage("Error", "Unsupported operation.");
				}
			} catch(Exception ex) {
				MesenMsgBox.Show("UnexpectedError", MessageBoxButtons.OK, MessageBoxIcon.Error, ex.ToString());
			}
		}
	}
}
