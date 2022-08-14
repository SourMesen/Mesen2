using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using System.ComponentModel;
using Avalonia.Data;
using Mesen.Interop;
using Mesen.ViewModels;
using Avalonia.Layout;
using Mesen.Utilities.GlobalMouseLib;
using Mesen.Utilities;
using Mesen.Config;

namespace Mesen.Windows
{
	public class HistoryViewerWindow : Window
	{
		private HistoryViewerViewModel _model;
		private DispatcherTimer _timer;
		private NativeRenderer _renderer;
		private Border _controlBar;
		private Menu _mainMenu;
		private bool _prevLeftPressed;

		static HistoryViewerWindow()
		{
			WindowStateProperty.Changed.AddClassHandler<HistoryViewerWindow>((x, e) => x.OnWindowStateChanged());
		}

		public HistoryViewerWindow()
		{
			_model = new HistoryViewerViewModel();
			DataContext = _model;

			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_renderer = this.GetControl<NativeRenderer>("Renderer");
			_controlBar = this.GetControl<Border>("ControlBar");
			_mainMenu = this.GetControl<Menu>("MainMenu");
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(50), DispatcherPriority.Normal, (s, e) => {
				_model.Update();
				UpdateMouse();
			});

			ConfigManager.Config.HistoryViewer.LoadWindowSettings(this);
		}

		private void UpdateMouse()
		{
			if(!IsActive || MenuHelper.IsPointerInMenu(_mainMenu)) {
				return;
			}

			bool leftPressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Left);
			MousePosition p = GlobalMouse.GetMousePosition();
			PixelPoint mousePos = new PixelPoint(p.X, p.Y);
			PixelPoint rendererTopLeft = _renderer.PointToScreen(new Point());
			PixelRect rendererScreenRect = new PixelRect(rendererTopLeft, PixelSize.FromSize(_renderer.Bounds.Size, 1.0));

			if(rendererScreenRect.Contains(mousePos)) {
				GlobalMouse.SetCursorIcon(CursorIcon.Arrow);
				if(leftPressed && !_prevLeftPressed) {
					if(_mainMenu.IsOpen) {
						_mainMenu.Close();
					} else {
						_model.TogglePause();
					}
				}
			}

			_prevLeftPressed = leftPressed;
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			if(Design.IsDesignMode) {
				return;
			}

			HistoryApi.HistoryViewerInitialize(PlatformImpl?.Handle.Handle ?? IntPtr.Zero, _renderer.Handle);
			_model.Update();
			_model.SetCoreOptions();
			_model.InitActions(this);
			_timer.Start();

			SetScale(ConfigManager.Config.HistoryViewer.Scale);
		}

		protected override void OnClosing(CancelEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}
			_timer.Stop();
			HistoryApi.HistoryViewerRelease();
			ConfigManager.Config.HistoryViewer.SaveWindowSettings(this);
		}

		public void SetScale(double scale)
		{
			ConfigManager.Config.HistoryViewer.Scale = (int)scale;

			double dpiScale = LayoutHelper.GetLayoutScale(this);
			scale /= dpiScale;

			FrameInfo screenSize = EmuApi.GetBaseScreenSize();
			if(WindowState == WindowState.Normal) {
				_renderer.Width = double.NaN;
				_renderer.Height = double.NaN;

				double aspectRatio = EmuApi.GetAspectRatio();

				//When menu is set to auto-hide, don't count its height when calculating the window's final size
				double menuHeight = _mainMenu.Bounds.Height;

				ClientSize = new Size(screenSize.Width * scale, screenSize.Width * scale / aspectRatio + menuHeight + _controlBar.Bounds.Height);
				ResizeRenderer();
			} else if(WindowState == WindowState.Maximized || WindowState == WindowState.FullScreen) {
				_renderer.Width = screenSize.Width * scale;
				_renderer.Height = screenSize.Height * scale;
			}
		}

		private void ResizeRenderer()
		{
			_renderer.InvalidateMeasure();
			_renderer.InvalidateArrange();
		}

		protected override void ArrangeCore(Rect finalRect)
		{
			//TODO why is this needed to make resizing the window by setting ClientSize work?
			base.ArrangeCore(new Rect(ClientSize));
		}

		private void OnWindowStateChanged()
		{
			if(WindowState == WindowState.Normal) {
				_renderer.Width = double.NaN;
				_renderer.Height = double.NaN;
				ResizeRenderer();
			}
		}
	}
}