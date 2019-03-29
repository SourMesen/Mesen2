using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public class DebugWindowManager
	{
		private static HashSet<Form> _openedWindows = new HashSet<Form>();

		public static List<Form> GetWindows()
		{
			return new List<Form>(_openedWindows);
		}

		public static Form OpenDebugWindow(DebugWindow window)
		{
			Form existingWindow = GetExistingSingleInstanceWindow(window);
			if(existingWindow != null) {
				existingWindow.BringToFront();
				if(existingWindow.WindowState == FormWindowState.Minimized) {
					//Unminimize window if it was minimized
					existingWindow.WindowState = FormWindowState.Normal;
				}
				existingWindow.Focus();
				return existingWindow;
			} else {
				BaseForm frm = null;
				switch(window) {
					case DebugWindow.Debugger: frm = new frmDebugger(); frm.Icon = Properties.Resources.Debugger; break;
					case DebugWindow.TraceLogger: frm = new frmTraceLogger(); frm.Icon = Properties.Resources.LogWindow; break;
					case DebugWindow.MemoryTools: frm = new frmMemoryTools(); frm.Icon = Properties.Resources.CheatCode; break;
					case DebugWindow.TileViewer: frm = new frmTileViewer(); frm.Icon = Properties.Resources.VerticalLayout; break;
					case DebugWindow.TilemapViewer: frm = new frmTilemapViewer(); frm.Icon = Properties.Resources.VideoOptions; break;
					case DebugWindow.PaletteViewer: frm = new frmPaletteViewer(); frm.Icon = Properties.Resources.VideoFilter; break;
					case DebugWindow.EventViewer: frm = new frmEventViewer(); frm.Icon = Properties.Resources.NesEventViewer; break;
				}
				_openedWindows.Add(frm);
				frm.FormClosed += Debugger_FormClosed;
				frm.Show();
				return frm;
			}
		}
		
		private static frmMemoryTools OpenMemoryViewer()
		{
			frmMemoryTools frm = GetMemoryViewer();
			if(frm == null) {
				frm = new frmMemoryTools();
				frm.Icon = Properties.Resources.CheatCode;
				frm.FormClosed += Debugger_FormClosed;
				_openedWindows.Add(frm);
			} else {
				if(frm.WindowState == FormWindowState.Minimized) {
					//Unminimize window if it was minimized
					frm.WindowState = FormWindowState.Normal;
				}
				frm.BringToFront();
			}
			frm.Show();
			return frm;
		}
		
		public static frmMemoryTools GetMemoryViewer()
		{
			return _openedWindows.ToList().Find((form) => form.GetType() == typeof(frmMemoryTools)) as frmMemoryTools;
		}

		public static bool HasOpenedWindow
		{
			get
			{
				return _openedWindows.Count > 0;
			}
		}

		public static void CloseAll()
		{
			List<Form> openedWindows = new List<Form>(_openedWindows);
			foreach(Form frm in openedWindows) {
				frm.Close();
			}
		}

		private static Form GetExistingSingleInstanceWindow(DebugWindow window)
		{
			//Only one of each of these windows can be opened at once, check if one is already opened
			switch(window) {
				case DebugWindow.Debugger: return _openedWindows.ToList().Find((form) => form.GetType() == typeof(frmDebugger));
				case DebugWindow.TraceLogger: return _openedWindows.ToList().Find((form) => form.GetType() == typeof(frmTraceLogger));
			}

			return null;
		}

		public static void CleanupDebugger()
		{
			if(_openedWindows.Count == 0) {
				//All windows have been closed, disable debugger
				DebugWorkspaceManager.SaveWorkspace();
				DebugWorkspaceManager.Clear();
				DebugApi.ReleaseDebugger();
			}
		}

		private static void Debugger_FormClosed(object sender, FormClosedEventArgs e)
		{
			_openedWindows.Remove((Form)sender);
			CleanupDebugger();
		}
	}

	public enum DebugWindow
	{
		Debugger,
		MemoryTools,
		TraceLogger,
		TileViewer,
		TilemapViewer,
		PaletteViewer,
		EventViewer
	}
}
