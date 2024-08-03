using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Layout;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
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

		private DispatcherTimer _timer = new DispatcherTimer(DispatcherPriority.Normal);

		private Control _renderer;
		private bool _usesSoftwareRenderer;
		private MainMenuView _mainMenu;
		private MainWindow _wnd;

		private int _prevPositionX;
		private int _prevPositionY;
		private bool _mouseCaptured = false;
		private DateTime _lastMouseMove = DateTime.Now;

		public MouseManager(MainWindow wnd, Control renderer, MainMenuView mainMenu, bool usesSoftwareRenderer)
		{
			_wnd = wnd;
			_renderer = renderer;
			_mainMenu = mainMenu;
			_usesSoftwareRenderer = usesSoftwareRenderer;

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

			SystemMouseState mouseState = InputApi.GetSystemMouseState(GetRendererHandle());
			
			if(_wnd.IsActive && mouseState.LeftButton && !IsPointerInMenu() && (EmuApi.IsRunning() || !MainWindowViewModel.Instance.RecentGames.Visible)) {
				//Close menu when renderer is clicked
				_mainMenu.MainMenu.Close();
				if(MainWindowViewModel.Instance.AudioPlayer == null) {
					//Only give renderer focus when the audio player isn't active
					//Otherwise clicking on the audio player's buttons does nothing
					_renderer.Focus();
				}
			}

			PixelPoint rendererTopLeft = _renderer.PointToScreen(new Point());
			PixelRect rendererScreenRect = new PixelRect(rendererTopLeft, PixelSize.FromSize(_renderer.Bounds.Size, LayoutHelper.GetLayoutScale(_wnd) / InputApi.GetPixelScale()));

			if(_prevPositionX != mouseState.XPosition || _prevPositionY != mouseState.YPosition) {
				//Send mouse movement x/y values to core
				if(_mouseCaptured) {
					InputApi.SetMouseMovement((Int16)(mouseState.XPosition - _prevPositionX), (Int16)(mouseState.YPosition - _prevPositionY));
				}
				_prevPositionX = mouseState.XPosition;
				_prevPositionY = mouseState.YPosition;
				_lastMouseMove = DateTime.Now;
			}
			PixelPoint mousePos = new PixelPoint(mouseState.XPosition, mouseState.YPosition);

			if(_wnd.IsActive && (_mainMenu.IsPointerOver || _mainMenu.IsKeyboardFocusWithin || _mainMenu.MainMenu.IsOpen)) {
				//When mouse or keyboard focus is in menu, release mouse and keep arrow cursor
				SetMouseOffScreen();
				ReleaseMouse();
				if(rendererScreenRect.Contains(mousePos)) {
					SetMouseCursor(CursorImage.Arrow);
				}
				return;
			}

			if(rendererScreenRect.Contains(mousePos)) {
				//Send mouse state to emulation core
				Point rendererPos = _renderer.PointToClient(mousePos) * LayoutHelper.GetLayoutScale(_wnd) / InputApi.GetPixelScale();
				InputApi.SetMousePosition(rendererPos.X / rendererScreenRect.Width, rendererPos.Y / rendererScreenRect.Height);

				bool buttonPressed = (mouseState.LeftButton || mouseState.RightButton || mouseState.MiddleButton || mouseState.Button4 || mouseState.Button5);

				InputApi.SetKeyState(LeftMouseButtonKeyCode, mouseState.LeftButton);
				InputApi.SetKeyState(RightMouseButtonKeyCode, mouseState.RightButton);
				InputApi.SetKeyState(MiddleMouseButtonKeyCode, mouseState.MiddleButton);
				InputApi.SetKeyState(MouseButton4KeyCode, mouseState.Button4);
				InputApi.SetKeyState(MouseButton5KeyCode, mouseState.Button5);

				if(!_mouseCaptured && AllowMouseCapture && buttonPressed) {
					//If the mouse button is clicked and mouse isn't captured but can be, turn on mouse capture
					CaptureMouse();
				}

				if(_mouseCaptured) {
					if(AllowMouseCapture) {
						SetMouseCursor(CursorImage.Hidden);
						InputApi.SetSystemMousePosition(rendererTopLeft.X + rendererScreenRect.Width / 2, rendererTopLeft.Y + rendererScreenRect.Height / 2);
						SystemMouseState newState = InputApi.GetSystemMouseState(GetRendererHandle());
						_prevPositionX = newState.XPosition;
						_prevPositionY = newState.YPosition;
					} else {
						ReleaseMouse();
					}
				}

				if(!_mouseCaptured) {
					SetMouseCursor(MouseIcon);
				}
			} else {
				SetMouseOffScreen();
			}
		}

		private void SetMouseCursor(CursorImage icon)
		{
			InputApi.SetCursorImage(icon);
			if(_usesSoftwareRenderer && !OperatingSystem.IsMacOS()) {
				//On MacOS, also setting the cursor on the renderer causes the cursor visibility to act oddly
				_renderer.Cursor = new Cursor(icon.ToStandardCursorType());
			}
		}

		private void UpdateMainMenuVisibility()
		{
			//Get global mouse position without restrictions - need to know if mouse is over menu or not
			SystemMouseState mouseState = InputApi.GetSystemMouseState(IntPtr.Zero);
			PixelPoint mousePos = new PixelPoint(mouseState.XPosition, mouseState.YPosition);

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

		private CursorImage MouseIcon
		{
			get
			{
				if(!EmuApi.IsRunning() || EmuApi.IsPaused()) {
					return CursorImage.Arrow;
				}

				bool hasLightGun = (
					InputApi.HasControlDevice(ControllerType.FamicomZapper) ||
					InputApi.HasControlDevice(ControllerType.NesZapper) ||
					InputApi.HasControlDevice(ControllerType.SmsLightPhaser) ||
					InputApi.HasControlDevice(ControllerType.SuperScope) ||
					InputApi.HasControlDevice(ControllerType.BandaiHyperShot)
				);

				if(hasLightGun) {
					if(ConfigManager.Config.Input.HidePointerForLightGuns) {
						return CursorImage.Hidden;
					} else {
						return CursorImage.Cross;
					}
				} else if(InputApi.HasControlDevice(ControllerType.OekaKidsTablet)) {
					return CursorImage.Cross;
				}

				if((DateTime.Now - _lastMouseMove).TotalSeconds > 1) {
					return CursorImage.Hidden;
				} else {
					return CursorImage.Arrow;
				}
			}
		}

		private void CaptureMouse()
		{
			if(!_mouseCaptured && AllowMouseCapture) {
				PixelPoint topLeft = _renderer.PointToScreen(new Point());
				PixelRect rendererScreenRect = new PixelRect(topLeft, PixelSize.FromSize(_renderer.Bounds.Size, LayoutHelper.GetLayoutScale(_wnd)));
				
				if(InputApi.CaptureMouse(topLeft.X, topLeft.Y, rendererScreenRect.Width, rendererScreenRect.Height, GetRendererHandle())) {
					DisplayMessageHelper.DisplayMessage("Input", ResourceHelper.GetMessage("MouseModeEnabled"));
					_mouseCaptured = true;
				}
			}
		}

		private void ReleaseMouse()
		{
			if(_mouseCaptured) {
				_mouseCaptured = false;
				InputApi.ReleaseMouse();
			}
		}

		private IntPtr GetRendererHandle()
		{
			return _usesSoftwareRenderer ? IntPtr.Zero : (_renderer as NativeRenderer)!.Handle;
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
