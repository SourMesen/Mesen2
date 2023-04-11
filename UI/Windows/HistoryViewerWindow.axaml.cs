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
using System.Runtime.InteropServices;
using Mesen.Controls;

namespace Mesen.Windows
{
	public class HistoryViewerWindow : MesenWindow
	{
		private HistoryViewerViewModel _model;
		private DispatcherTimer _timer;
		private DispatcherTimer _mouseTimer;

		private NativeRenderer _renderer;
		private SoftwareRendererView _softwareRenderer;
		private Panel _rendererPanel;
		private Size _rendererSize;

		private Border _controlBar;
		private Menu _mainMenu;
		private bool _prevLeftPressed;
		private NotificationListener? _listener;

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
			_softwareRenderer = this.GetControl<SoftwareRendererView>("SoftwareRenderer");
			_rendererPanel = this.GetControl<Panel>("RendererPanel");
			_rendererPanel.LayoutUpdated += RendererPanel_LayoutUpdated;

			_controlBar = this.GetControl<Border>("ControlBar");
			_mainMenu = this.GetControl<Menu>("ActionMenu");
			_timer = new DispatcherTimer(TimeSpan.FromMilliseconds(50), DispatcherPriority.Normal, (s, e) => {
				_model.Update();
			});

			_mouseTimer = new DispatcherTimer(TimeSpan.FromMilliseconds(15), DispatcherPriority.Normal, (s, e) => {
				UpdateMouse();
			});

			ConfigManager.Config.HistoryViewer.LoadWindowSettings(this);
		}

		private void UpdateMouse()
		{
			if(MenuHelper.IsPointerInMenu(_mainMenu)) {
				return;
			}

			bool leftPressed = GlobalMouse.IsMouseButtonPressed(MouseButtons.Left);
			MousePosition p = GlobalMouse.GetMousePosition(_renderer.Handle);
			PixelPoint mousePos = new PixelPoint(p.X, p.Y);
			PixelPoint rendererTopLeft = _renderer.PointToScreen(new Point());
			PixelRect rendererScreenRect = new PixelRect(rendererTopLeft, PixelSize.FromSize(_renderer.Bounds.Size, LayoutHelper.GetLayoutScale(this)));

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
			_model.SetCoreOptions();
			_model.Update();
			_model.InitActions(this);
			_timer.Start();
			_mouseTimer.Start();
			
			_listener = new NotificationListener(forHistoryViewer: true);
			_listener.OnNotification += OnNotification;
		}

		protected override void OnClosing(WindowClosingEventArgs e)
		{
			base.OnClosing(e);
			if(Design.IsDesignMode) {
				return;
			}
			_timer.Stop();
			_mouseTimer.Stop();
			_listener?.Dispose();
			HistoryApi.HistoryViewerRelease();
			ConfigManager.Config.HistoryViewer.SaveWindowSettings(this);
		}

		public void SetScale(double scale)
		{
			//TODOv2 - Calling this twice seems to fix what might be an issue in Avalonia?
			//On the first call, when DPI > 100%, sometimes _rendererPanel's bounds are incorrect
			InternalSetScale(scale);
			InternalSetScale(scale);
		}

		private void InternalSetScale(double scale)
		{
			double dpiScale = LayoutHelper.GetLayoutScale(this);
			scale /= dpiScale;

			FrameInfo screenSize = EmuApi.GetBaseScreenSize();
			if(WindowState == WindowState.Normal) {
				_rendererSize = new Size();

				double aspectRatio = EmuApi.GetAspectRatio();
				double menuHeight = _mainMenu.Bounds.Height;

				double width = Math.Max(MinWidth, Math.Round(screenSize.Height * aspectRatio) * scale);
				double height = Math.Max(MinHeight, screenSize.Height * scale);
				ClientSize = new Size(width, height + menuHeight + _controlBar.Bounds.Height);
				ResizeRenderer();
			} else if(WindowState == WindowState.Maximized || WindowState == WindowState.FullScreen) {
				_rendererSize = new Size(Math.Floor(screenSize.Width * scale), Math.Floor(screenSize.Height * scale));
				ResizeRenderer();
			}
		}

		private void ResizeRenderer()
		{
			_rendererPanel.InvalidateMeasure();
			_rendererPanel.InvalidateArrange();
		}

		private void RendererPanel_LayoutUpdated(object? sender, EventArgs e)
		{
			double aspectRatio = EmuApi.GetAspectRatio();

			Size finalSize = _rendererSize == default ? _rendererPanel.Bounds.Size : _rendererSize;
			double height = finalSize.Height;
			double width = finalSize.Height * aspectRatio;
			if(width > finalSize.Width) {
				width = finalSize.Width;
				height = width / aspectRatio;
			}

			if(ConfigManager.Config.Video.FullscreenForceIntegerScale && VisualRoot is Window wnd && (wnd.WindowState == WindowState.FullScreen || wnd.WindowState == WindowState.Maximized)) {
				FrameInfo baseSize = EmuApi.GetBaseScreenSize();
				double scale = height * LayoutHelper.GetLayoutScale(this) / baseSize.Height;
				if(scale != Math.Floor(scale)) {
					height = baseSize.Height * Math.Max(1, Math.Floor(scale / LayoutHelper.GetLayoutScale(this)));
					width = height * aspectRatio;
				}
			}

			_model.RendererSize = new Size(
				Math.Round(width * LayoutHelper.GetLayoutScale(this)),
				Math.Round(height * LayoutHelper.GetLayoutScale(this))
			);

			_renderer.Width = width;
			_renderer.Height = height;
			_model.SoftwareRenderer.Width = width;
			_model.SoftwareRenderer.Height = height;
		}

		protected override void ArrangeCore(Rect finalRect)
		{
			//TODOv2 why is this needed to make resizing the window by setting ClientSize work?
			base.ArrangeCore(new Rect(ClientSize));
		}

		private void OnWindowStateChanged()
		{
			_rendererSize = new Size();
			ResizeRenderer();
		}

		private void OnNotification(NotificationEventArgs e)
		{
			switch(e.NotificationType) {
				case ConsoleNotificationType.RefreshSoftwareRenderer:
					SoftwareRendererFrame frame = Marshal.PtrToStructure<SoftwareRendererFrame>(e.Parameter);
					_softwareRenderer.UpdateSoftwareRenderer(frame);
					break;
			}
		}
	}
}