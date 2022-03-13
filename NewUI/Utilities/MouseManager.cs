using Avalonia;
using Avalonia.Controls;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities.GlobalMouseLib;
using Mesen.Views;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class MouseManager
	{
		private const int LeftMouseButtonKeyCode = 0x200;
		private const int RightMouseButtonKeyCode = 0x201;
		private const int MiddleMouseButtonKeyCode = 0x202;

		private DispatcherTimer _timer = new DispatcherTimer();

		private NativeRenderer _renderer;
		private MainMenuView _mainMenu;
		private MainWindow _wnd;

		private MousePosition _prevPosition;
		private bool _mouseCaptured = false;
		private DateTime _lastMouseMove = DateTime.Now;

		public MouseManager(MainWindow wnd, NativeRenderer renderer, MainMenuView mainMenu)
		{
			_wnd = wnd;
			_renderer = renderer;
			_mainMenu = mainMenu;

			_timer.Interval = TimeSpan.FromMilliseconds(15);
			_timer.Tick += tmrProcessMouse;
			_timer.Start();
		}

		private bool IsPointerInItem(MenuItem item)
		{
			if(item.IsSubMenuOpen) {
				foreach(var container in item.ItemContainerGenerator.Containers) {
					if(container.ContainerControl is MenuItem subItem && IsPointerInItem(subItem)) {
						return true;
					} else if(container.ContainerControl.IsPointerOver) {
						return true;
					}
				}
			}

			return item.IsPointerOver;
		}

		private bool IsPointerInMenu()
		{
			if(_mainMenu.IsPointerOver || _mainMenu.MainMenu.IsPointerOver) {
				return true;
			}

			foreach(MenuItem item in _mainMenu.MainMenu.Items) {
				if(IsPointerInItem(item)) {
					return true;
				}
			}

			return false;
		}

		private void tmrProcessMouse(object? sender, EventArgs e)
		{
			bool leftPressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Left);
			if(_wnd.IsActive && leftPressed && !IsPointerInMenu() && EmuApi.IsRunning()) {
				_mainMenu.MainMenu.Close();
				_renderer.Focus();
			}

			if(_mainMenu.IsPointerOver || _mainMenu.IsKeyboardFocusWithin || _mainMenu.MainMenu.IsOpen) {
				SetMouseOffScreen();
				ReleaseMouse();
				GlobalMouse.SetCursorIcon(CursorIcon.Arrow);
				return;
			}

			PixelPoint rendererTopLeft = _renderer.PointToScreen(new Point());
			PixelRect rendererScreenRect = new PixelRect(rendererTopLeft, PixelSize.FromSize(_renderer.Bounds.Size, 1.0));

			MousePosition p = GlobalMouse.GetMousePosition();
			if(_prevPosition.X != p.X || _prevPosition.Y != p.Y) {
				//Send mouse movement x/y values to core
				InputApi.SetMouseMovement((Int16)(p.X - _prevPosition.X), (Int16)(p.Y - _prevPosition.Y));
				_prevPosition = p;
				_lastMouseMove = DateTime.Now;
			}
			PixelPoint mousePos = new PixelPoint(p.X, p.Y);

			if(((Window)_renderer.Parent!.VisualRoot!).IsActive && rendererScreenRect.Contains(mousePos)) {
				//Send mouse state to emulation core
				Point rendererPos = _renderer.PointToClient(mousePos);
				InputApi.SetMousePosition(rendererPos.X / rendererScreenRect.Width, rendererPos.Y / rendererScreenRect.Height);

				InputApi.SetKeyState(LeftMouseButtonKeyCode, leftPressed);
				InputApi.SetKeyState(RightMouseButtonKeyCode, GlobalMouse.IsMouseButtonPressed(MouseButtons.Right));
				InputApi.SetKeyState(MiddleMouseButtonKeyCode, GlobalMouse.IsMouseButtonPressed(MouseButtons.Middle));

				if(!_mouseCaptured && AllowMouseCapture && leftPressed) {
					//If the mouse button is clicked and mouse isn't captured but can be, turn on mouse capture
					CaptureMouse();
				}

				if(_mouseCaptured) {
					if(AllowMouseCapture) {
						GlobalMouse.SetCursorIcon(CursorIcon.Hidden);
						PixelPoint rendererCenter = _renderer.PointToScreen(_renderer.Bounds.Center);
						GlobalMouse.SetMousePosition((uint)rendererCenter.X, (uint)rendererCenter.Y);
						_prevPosition = new MousePosition(rendererCenter.X, rendererCenter.Y);
					} else {
						ReleaseMouse();
					}
				} 

				if(!_mouseCaptured) {
					GlobalMouse.SetCursorIcon(MouseIcon);
				}
			} else {
				SetMouseOffScreen();
			}
		}

		private static void SetMouseOffScreen()
		{
			//Send mouse state to emulation core (mouse is set as off the screen)
			InputApi.SetMousePosition(-1, -1);

			InputApi.SetKeyState(LeftMouseButtonKeyCode, false);
			InputApi.SetKeyState(RightMouseButtonKeyCode, false);
			InputApi.SetKeyState(MiddleMouseButtonKeyCode, false);
		}

		private bool AllowMouseCapture
		{
			get
			{
				if(!EmuApi.IsRunning() || EmuApi.IsPaused()) {
					return false;
				}

				bool hasMouseDevice = (
					InputApi.HasControlDevice(ControllerType.SnesMouse) ||
					InputApi.HasControlDevice(ControllerType.SuborMouse) ||
					InputApi.HasControlDevice(ControllerType.FamicomArkanoidController) ||
					InputApi.HasControlDevice(ControllerType.NesArkanoidController) ||
					InputApi.HasControlDevice(ControllerType.HoriTrack)
				);

				if(hasMouseDevice) {
					return true;
				}

				return false;
			}
		}

		private CursorIcon MouseIcon
		{
			get
			{
				if(!EmuApi.IsRunning() || EmuApi.IsPaused()) {
					return CursorIcon.Arrow;
				}

				bool hasLightGun = (
					InputApi.HasControlDevice(ControllerType.FamicomZapper) ||
					InputApi.HasControlDevice(ControllerType.NesZapper) ||
					InputApi.HasControlDevice(ControllerType.SuperScope) ||
					InputApi.HasControlDevice(ControllerType.BandaiHyperShot)
				);

				if(hasLightGun) {
					if(ConfigManager.Config.Input.HidePointerForLightGuns) {
						return CursorIcon.Hidden;
					} else {
						return CursorIcon.Cross;
					}
				} else if(InputApi.HasControlDevice(ControllerType.OekaKidsTablet)) {
					return CursorIcon.Cross;
				}

				if((DateTime.Now - _lastMouseMove).TotalSeconds > 1) {
					return CursorIcon.Hidden;
				} else {
					return CursorIcon.Arrow;
				}
			}
		}

		private void CaptureMouse()
		{
			if(!_mouseCaptured && AllowMouseCapture) {
				EmuApi.DisplayMessage("Input", ResourceHelper.GetMessage("MouseModeEnabled"));
				_mouseCaptured = true;
				
				PixelPoint topLeft = _renderer.PointToScreen(new Point(0, 0));

				GlobalMouse.CaptureCursor(topLeft.X, topLeft.Y, (int)_renderer.Bounds.Width, (int)_renderer.Bounds.Height);
			}
		}

		private void ReleaseMouse()
		{
			if(_mouseCaptured) {
				_mouseCaptured = false;
				GlobalMouse.ReleaseCursor();
			}
		}

	}
}
