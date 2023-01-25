using Avalonia;
using Avalonia.Controls;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities.GlobalMouseLib;
using Mesen.ViewModels;
using Mesen.Views;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class MouseManager : IDisposable
	{
		public const int LeftMouseButtonKeyCode = 0x200;
		public const int RightMouseButtonKeyCode = 0x201;
		public const int MiddleMouseButtonKeyCode = 0x202;
		public const int MouseButton4KeyCode = 0x203;
		public const int MouseButton5KeyCode = 0x204;

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

		private bool IsPointerInMenu()
		{
			return _mainMenu.IsPointerOver || MenuHelper.IsPointerInMenu(_mainMenu.MainMenu);
		}

		private void tmrProcessMouse(object? sender, EventArgs e)
		{
			if(MainWindowViewModel.Instance.RecentGames.Visible) {
				return;
			}
			
			bool leftPressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Left);
			if(_wnd.IsActive && leftPressed && !IsPointerInMenu() && EmuApi.IsRunning()) {
				//Close menu when renderer is clicked
				_mainMenu.MainMenu.Close();
				if(MainWindowViewModel.Instance.AudioPlayer == null) {
					//Only give renderer focus when the audio player isn't active
					//Otherwise clicking on the audio player's buttons does nothing
					_renderer.Focus();
				}
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

			UpdateMainMenuVisibility(mousePos);

			if(_wnd.IsActive && (_mainMenu.IsPointerOver || _mainMenu.IsKeyboardFocusWithin || _mainMenu.MainMenu.IsOpen)) {
				//When mouse or keyboard focus is in menu, release mouse and keep arrow cursor
				SetMouseOffScreen();
				ReleaseMouse();
				if(rendererScreenRect.Contains(mousePos)) {
					GlobalMouse.SetCursorIcon(CursorIcon.Arrow);
				}
				return;
			}

			if(_wnd.IsActive && rendererScreenRect.Contains(mousePos)) {
				//Send mouse state to emulation core
				Point rendererPos = _renderer.PointToClient(mousePos);
				InputApi.SetMousePosition(rendererPos.X / rendererScreenRect.Width, rendererPos.Y / rendererScreenRect.Height);

				InputApi.SetKeyState(LeftMouseButtonKeyCode, leftPressed);
				InputApi.SetKeyState(RightMouseButtonKeyCode, GlobalMouse.IsMouseButtonPressed(MouseButtons.Right));
				InputApi.SetKeyState(MiddleMouseButtonKeyCode, GlobalMouse.IsMouseButtonPressed(MouseButtons.Middle));
				InputApi.SetKeyState(MouseButton4KeyCode, GlobalMouse.IsMouseButtonPressed(MouseButtons.Button4));
				InputApi.SetKeyState(MouseButton5KeyCode, GlobalMouse.IsMouseButtonPressed(MouseButtons.Button5));

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

		private void UpdateMainMenuVisibility(PixelPoint mousePos)
		{
			bool autoHideMenu = _wnd.WindowState == WindowState.FullScreen || ConfigManager.Config.Preferences.AutoHideMenu;
			if(autoHideMenu) {
				if(_mainMenu.MainMenu.IsOpen) {
					MainWindowViewModel.Instance.IsMenuVisible = true;
				} else {
					PixelPoint wndTopLeft = _wnd.PointToScreen(new Point(0, 0));
					bool showMenu = (
						mousePos.Y >= wndTopLeft.Y - 15 && mousePos.Y <= wndTopLeft.Y + Math.Max(_mainMenu.Bounds.Height + 10, 35) &&
						mousePos.X >= wndTopLeft.X && mousePos.X <= wndTopLeft.X + _wnd.Bounds.Width
					);
					MainWindowViewModel.Instance.IsMenuVisible = showMenu;
				}
			} else {
				MainWindowViewModel.Instance.IsMenuVisible = true;
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

				GlobalMouse.CaptureCursor(topLeft.X, topLeft.Y, (int)_renderer.Bounds.Width, (int)_renderer.Bounds.Height, _renderer.Handle);
			}
		}

		private void ReleaseMouse()
		{
			if(_mouseCaptured) {
				_mouseCaptured = false;
				GlobalMouse.ReleaseCursor();
			}
		}

		public void Dispose()
		{
			if(_timer is DispatcherTimer timer) {
				timer.Tick -= tmrProcessMouse;
				timer.Stop();
			}
		}
	}
}
