using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Emulation
{
	public class CursorManager
	{
		private static bool _cursorHidden = false;
		private static Point _lastPosition;
		private static Timer _tmrHideMouse = new Timer();
		private static Timer _tmrCheckMouseMove = new Timer();
		private static bool _mouseCaptured = false;

		static CursorManager()
		{
			_tmrHideMouse.Interval = 3000;
			_tmrHideMouse.Tick += tmrHideMouse_Tick;
			_tmrCheckMouseMove.Interval = 500;
			_tmrCheckMouseMove.Tick += tmrCheckMouseMove_Tick;
			_tmrCheckMouseMove.Start();
		}

		public static void StopTimers()
		{
			_tmrCheckMouseMove.Stop();
			_tmrHideMouse.Stop();
		}

		private static void tmrCheckMouseMove_Tick(object sender, EventArgs e)
		{
			//Rarely the cursor becomes hidden despite leaving the window or moving
			//Have not been able to find a reliable way to reproduce it yet
			//This is a patch to prevent that bug from having any negative impact
			if(!_mouseCaptured && _lastPosition != Cursor.Position) {
				_lastPosition = Cursor.Position;

				//TODO
				/*bool running = EmuRunner.IsRunning() && !EmuApi.IsPaused();
				if(running && ConfigManager.Config.Input.HideMousePointerForZapper && CursorManager.IsLightGun) {
					//Keep mouse hidden when using zapper if option to hide mouse is enabled
					return;
				}*/

				ShowMouse();
			}
		}

		private static void tmrHideMouse_Tick(object sender, EventArgs e)
		{
			bool running = EmuRunner.IsRunning() && !EmuApi.IsPaused();
			if(running) {
				HideMouse();
				_tmrHideMouse.Stop();
			} else {
				ShowMouse();
				_tmrHideMouse.Stop();
			}
		}

		private static void ShowMouse()
		{
			if(_cursorHidden) {
				Cursor.Show();
				_cursorHidden = false;
			}
		}

		private static void HideMouse()
		{
			if(!_cursorHidden) {
				Cursor.Hide();
				_cursorHidden = true;
			}
		}

		private static bool IsLightGun
		{
			get
			{
				for(int i = 0; i < 4; i++) {
					switch(ConfigManager.Config.Input.Controllers[i].Type) {
						case ControllerType.SuperScope:
							return true;
					}
				}
				return false;
			}
		}

		public static bool NeedMouseIcon
		{
			get { return IsLightGun; }
		}

		public static void OnMouseMove(Control ctrl)
		{
			if(_mouseCaptured && AllowMouseCapture) {
				HideMouse();
				_tmrHideMouse.Stop();
				Form frm = frmMain.Instance;
				Point centerPos = frm.PointToScreen(new Point(frm.Width / 2, frm.Height / 2));
				Point diff = new Point(Cursor.Position.X - centerPos.X, Cursor.Position.Y - centerPos.Y);
				if(diff.X != 0 || diff.Y != 0) {
					InputApi.SetMouseMovement((Int16)diff.X, (Int16)diff.Y);
					Cursor.Position = centerPos;
				}
			} else {
				_mouseCaptured = false;

				if(!EmuRunner.IsRunning() || EmuApi.IsPaused()) {
					ShowMouse();
				} /* TODO else if(ConfigManager.Config.Input.HideMousePointerForZapper && CursorManager.IsLightGun) {
					//Keep mouse hidden when using zapper if option to hide mouse is enabled
					HideMouse();
					return;
				}*/

				_tmrHideMouse.Stop();

				if(!CursorManager.NeedMouseIcon) {
					//Only hide mouse if no zapper (otherwise this could be pretty annoying)
					ctrl.Cursor = Cursors.Default;

					if(EmuRunner.IsRunning() && !EmuApi.IsPaused()) {
						_tmrHideMouse.Start();
					}
				}
			}
		}

		public static void OnMouseLeave()
		{
			_tmrHideMouse.Stop();
			ShowMouse();
		}

		public static bool AllowMouseCapture
		{
			get
			{
				if(!EmuRunner.IsRunning()) {
					return false;
				}

				if(EmuApi.IsPaused()) {
					return false;
				}

				//TODO
				/*
				if(EmuApi.CheckFlag(EmulationFlags.InBackground)) {
					return false;
				}*/

				for(int i = 0; i < 4; i++) {
					switch(ConfigManager.Config.Input.Controllers[i].Type) {
						case ControllerType.SnesMouse:
							return true;
					}
				}
				return false;
			}
		}

		public static void ReleaseMouse()
		{
			_mouseCaptured = false;
			ShowMouse();
		}

		public static void CaptureMouse()
		{
			if(AllowMouseCapture) {
				if(!_mouseCaptured) {
					EmuApi.DisplayMessage("Input", ResourceHelper.GetMessage("MouseModeEnabled"));
				}
				_mouseCaptured = true;
				HideMouse();
				Form frm = frmMain.Instance;
				Point centerPos = frm.PointToScreen(new Point(frm.Width / 2, frm.Height / 2));
				Cursor.Position = centerPos;
			}
		}
	}
}
