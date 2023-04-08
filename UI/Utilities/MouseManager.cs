using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
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
			UpdateMainMenuVisibility();

			if(MainWindowViewModel.Instance.RecentGames.Visible) {
				return;
			}
			
			bool leftPressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Left);
			if(_wnd.IsActive && leftPressed && !IsPointerInMenu() && (EmuApi.IsRunning() || !MainWindowViewModel.Instance.RecentGames.Visible)) {
				//Close menu when renderer is clicked
				_mainMenu.MainMenu.Close();
				if(MainWindowViewModel.Instance.AudioPlayer == null) {
					//Only give renderer focus when the audio player isn't active
					//Otherwise clicking on the audio player's buttons does nothing
					_renderer.Focus();
				}
			}

			PixelPoint rendererTopLeft = _renderer.PointToScreen(new Point());
			PixelRect rendererScreenRect = new PixelRect(rendererTopLeft, PixelSize.FromSize(_renderer.Bounds.Size, LayoutHelper.GetLayoutScale(_wnd)));

			MousePosition p = GlobalMouse.GetMousePosition(_renderer.Handle);
			if(_prevPosition.X != p.X || _prevPosition.Y != p.Y) {
				//Send mouse movement x/y values to core
				if(_mouseCaptured) {
					InputApi.SetMouseMovement((Int16)(p.X - _prevPosition.X), (Int16)(p.Y - _prevPosition.Y));
				}
				_prevPosition = p;
				_lastMouseMove = DateTime.Now;
			}
			PixelPoint mousePos = new PixelPoint(p.X, p.Y);

			if(_wnd.IsActive && (_mainMenu.IsPointerOver || _mainMenu.IsKeyboardFocusWithin || _mainMenu.MainMenu.IsOpen)) {
				//When mouse or keyboard focus is in menu, release mouse and keep arrow cursor
				SetMouseOffScreen();
				ReleaseMouse();
				if(rendererScreenRect.Contains(mousePos)) {
					GlobalMouse.SetCursorIcon(CursorIcon.Arrow);
				}
				return;
			}

			if(rendererScreenRect.Contains(mousePos)) {
				//Send mouse state to emulation core
				Point rendererPos = _renderer.PointToClient(mousePos) * LayoutHelper.GetLayoutScale(_wnd);
				InputApi.SetMousePosition(rendererPos.X / rendererScreenRect.Width, rendererPos.Y / rendererScreenRect.Height);

				bool rightPressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Right);
				bool middlePressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Middle);
				bool button4Pressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Button4);
				bool button5Pressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Button5);
				bool buttonPressed = (leftPressed || rightPressed || middlePressed || button4Pressed || button5Pressed);

				InputApi.SetKeyState(LeftMouseButtonKeyCode, leftPressed);
				InputApi.SetKeyState(RightMouseButtonKeyCode, rightPressed);
				InputApi.SetKeyState(MiddleMouseButtonKeyCode, middlePressed);
				InputApi.SetKeyState(MouseButton4KeyCode, button4Pressed);
				InputApi.SetKeyState(MouseButton5KeyCode, button5Pressed);

				if(!_mouseCaptured && AllowMouseCapture && buttonPressed) {
					//If the mouse button is clicked and mouse isn't captured but can be, turn on mouse capture
					CaptureMouse();
				}

				if(_mouseCaptured) {
					if(AllowMouseCapture) {
						GlobalMouse.SetCursorIcon(CursorIcon.Hidden);
						PixelPoint rendererCenter = _renderer.PointToScreen(new Point(rendererScreenRect.Width / 2, rendererScreenRect.Height / 2));
						GlobalMouse.SetMousePosition((uint)rendererCenter.X, (uint)rendererCenter.Y);
						_prevPosition = GlobalMouse.GetMousePosition(_renderer.Handle);
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

		private void UpdateMainMenuVisibility()
		{
			//Get global mouse position without restrictions - need to know if mouse is over menu or not
			MousePosition p = GlobalMouse.GetMousePosition(IntPtr.Zero);
			PixelPoint mousePos = new PixelPoint(p.X, p.Y);

			bool inExclusiveFullscreen = _wnd.WindowState == WindowState.FullScreen && ConfigManager.Config.Video.UseExclusiveFullscreen;
			bool autoHideMenu = _wnd.WindowState == WindowState.FullScreen || ConfigManager.Config.Preferences.AutoHideMenu;
			if(inExclusiveFullscreen) {
				MainWindowViewModel.Instance.IsMenuVisible = false;
			} else if(autoHideMenu) {
				if(_mainMenu.MainMenu.IsOpen) {
					MainWindowViewModel.Instance.IsMenuVisible = true;
				} else {
					PixelPoint wndTopLeft = _wnd.PointToScreen(new Point(0, 0));
					double scale = LayoutHelper.GetLayoutScale(_wnd);
					bool showMenu = (
						mousePos.Y >= wndTopLeft.Y - 15 && mousePos.Y <= wndTopLeft.Y + Math.Max(_mainMenu.Bounds.Height * scale + 10, 35 * scale) &&
						mousePos.X >= wndTopLeft.X && mousePos.X <= wndTopLeft.X + _wnd.Bounds.Width * scale
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
			InputApi.SetKeyState(MouseButton4KeyCode, false);
			InputApi.SetKeyState(MouseButton5KeyCode, false);
		}

		private bool AllowMouseCapture
		{
			get
			{
				if(!EmuApi.IsRunning() || EmuApi.IsPaused() || !_wnd.IsActive) {
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
				
				PixelPoint topLeft = _renderer.PointToScreen(new Point());
				PixelRect rendererScreenRect = new PixelRect(topLeft, PixelSize.FromSize(_renderer.Bounds.Size, LayoutHelper.GetLayoutScale(_wnd)));
				
				GlobalMouse.CaptureCursor(topLeft.X, topLeft.Y, rendererScreenRect.Width, rendererScreenRect.Height, _renderer.Handle);
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
