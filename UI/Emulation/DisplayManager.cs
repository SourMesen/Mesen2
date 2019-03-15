using Mesen.GUI.Config;
using Mesen.GUI.Controls;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Emulation
{
	public class DisplayManager
	{
		private frmMain _frm;
		private ctrlRenderer _renderer;
		private Panel _panel;
		private MenuStrip _menu;
		private ctrlRecentGames _recentGames;
		private bool _resizeForm;
		private bool _fullscreenMode;
		private FormWindowState _originalWindowState;
		private Size _originalWindowMinimumSize;
		private frmFullscreenRenderer _frmFullscreenRenderer = null;

		public bool Fullscreen { get { return _fullscreenMode; } }
		public bool ExclusiveFullscreen { get { return _frmFullscreenRenderer != null; } }

		public DisplayManager(frmMain frm, ctrlRenderer renderer, Panel panel, MenuStrip menu, ctrlRecentGames recentGames)
		{
			_frm = frm;
			_renderer = renderer;
			_panel = panel;
			_menu = menu;
			_recentGames = recentGames;

			_renderer.MouseClick += ctrlRenderer_MouseClick;
			_renderer.Enter += ctrlRenderer_Enter;
			_renderer.MouseMove += ctrlRenderer_MouseMove;
			_panel.MouseMove += ctrlRenderer_MouseMove;
			_recentGames.MouseMove += ctrlRenderer_MouseMove;
			_panel.Click += panelRenderer_Click;

			_frm.Resize += frmMain_Resize;
			_menu.VisibleChanged += menu_VisibleChanged;
		}

		private void frmMain_Resize(object sender, EventArgs e)
		{
			SetScaleBasedOnWindowSize();
		}
		
		private void menu_VisibleChanged(object sender, EventArgs e)
		{
			if(!_menu.Visible) {
				_renderer.Top += _menu.Height;
			} else {
				_renderer.Top -= _menu.Height;
			}
		}

		public bool HideMenuStrip
		{
			get {	return (_fullscreenMode && !ConfigManager.Config.Video.UseExclusiveFullscreen) || ConfigManager.Config.Preferences.AutoHideMenu; }
		}

		private void ctrlRenderer_MouseMove(object sender, MouseEventArgs e)
		{
			if(sender != _recentGames) {
				//CursorManager.OnMouseMove((Control)sender);
			}

			if(this.HideMenuStrip && !_menu.ContainsFocus) {
				if(sender == _renderer) {
					_menu.Visible = _renderer.Top + e.Y < 30;
				} else {
					_menu.Visible = e.Y < 30;
				}
			}
		}

		private void ctrlRenderer_MouseClick(object sender, MouseEventArgs e)
		{
			if(this.HideMenuStrip) {
				_menu.Visible = false;
			}
			//CursorManager.CaptureMouse();
		}

		private void panelRenderer_Click(object sender, EventArgs e)
		{
			if(this.HideMenuStrip) {
				_menu.Visible = false;
			}
			//CursorManager.CaptureMouse();

			_renderer.Focus();
		}

		private void ctrlRenderer_Enter(object sender, EventArgs e)
		{
			if(this.HideMenuStrip) {
				_menu.Visible = false;
			}
		}

		public void UpdateViewerSize()
		{
			if(HideMenuStrip) {
				_menu.Visible = false;
			}

			ScreenSize screenSize = EmuApi.GetScreenSize(false);

			if(_resizeForm && _frm.WindowState != FormWindowState.Maximized) {
				_frm.Resize -= frmMain_Resize;
				Size newSize = new Size(screenSize.Width, screenSize.Height);
				_frm.ClientSize = new Size(newSize.Width, newSize.Height + (HideMenuStrip ? 0 : _panel.Top));
				_frm.Resize += frmMain_Resize;
			}

			_renderer.Size = new Size(screenSize.Width, screenSize.Height);
			_renderer.Top = (_panel.Height - _renderer.Height) / 2;
			_renderer.Left = (_panel.Width - _renderer.Width) / 2;
		}

		public void SetScaleBasedOnDimensions(Size dimensions)
		{
			ScreenSize size = EmuApi.GetScreenSize(true);

			double verticalScale = (double)dimensions.Height / size.Height;
			double horizontalScale = (double)dimensions.Width / size.Width;
			double scale = Math.Min(verticalScale, horizontalScale);
			if(_fullscreenMode && ConfigManager.Config.Video.FullscreenForceIntegerScale) {
				scale = Math.Floor(scale);
			}

			SetScale(scale, false);
		}

		private void SetScaleBasedOnWindowSize()
		{
			SetScaleBasedOnDimensions(_panel.ClientSize);
		}

		private void SetScaleBasedOnScreenSize()
		{
			SetScaleBasedOnDimensions(Screen.FromControl(_frm).Bounds.Size);
		}

		public void SetScale(double scale, bool resizeForm)
		{
			_resizeForm = resizeForm;
			ConfigManager.Config.Video.VideoScale = scale;
			ConfigManager.Config.Video.ApplyConfig();
			ConfigManager.ApplyChanges();

			UpdateViewerSize();
		}

		public void ToggleFullscreen()
		{
			SetFullscreenState(!_fullscreenMode);
		}

		public void SetFullscreenState(bool enabled)
		{
			if(_fullscreenMode == enabled) {
				//Fullscreen mode already matches, no need to do anything
				return;
			}

			bool saveState = !_fullscreenMode;
			_fullscreenMode = enabled;

			if(ConfigManager.Config.Video.UseExclusiveFullscreen) {
				if(enabled && EmuRunner.IsRunning()) {
					StartExclusiveFullscreenMode();
				} else {
					StopExclusiveFullscreenMode();
				}
			} else {
				_frm.Resize -= frmMain_Resize;
				if(enabled) {
					StartFullscreenWindowMode(saveState);
				} else {
					StopFullscreenWindowMode();
				}
				_frm.Resize += frmMain_Resize;
				UpdateViewerSize();
			}
		}

		private void StartFullscreenWindowMode(bool saveState)
		{
			_menu.Visible = false;
			if(saveState) {
				_originalWindowState = _frm.WindowState;
				_originalWindowMinimumSize = _frm.MinimumSize;
			}
			_frm.MinimumSize = new Size(0, 0);
			_frm.WindowState = FormWindowState.Normal;
			_frm.FormBorderStyle = FormBorderStyle.None;
			_frm.WindowState = FormWindowState.Maximized;
			SetScaleBasedOnWindowSize();
		}

		private void StopFullscreenWindowMode()
		{
			_menu.Visible = true;
			_frm.WindowState = _originalWindowState;
			_frm.MinimumSize = _originalWindowMinimumSize;
			_frm.FormBorderStyle = FormBorderStyle.Sizable;
			frmMain_Resize(null, EventArgs.Empty);
		}

		private void StopExclusiveFullscreenMode()
		{
			if(_frmFullscreenRenderer != null) {
				_frmFullscreenRenderer.Close();
			}
			_fullscreenMode = false;
		}

		private void StartExclusiveFullscreenMode()
		{
			Size screenSize = Screen.FromControl(_frm).Bounds.Size;
			_frmFullscreenRenderer = new frmFullscreenRenderer();
			_frmFullscreenRenderer.Shown += (object sender, EventArgs e) => {
				_renderer.Visible = false;
				SetScaleBasedOnScreenSize();
				EmuApi.SetFullscreenMode(true, _frmFullscreenRenderer.Handle, (UInt32)screenSize.Width, (UInt32)screenSize.Height);
			};
			_frmFullscreenRenderer.FormClosing += (object sender, FormClosingEventArgs e) => {
				EmuApi.SetFullscreenMode(false, _renderer.Handle, (UInt32)screenSize.Width, (UInt32)screenSize.Height);
				_frmFullscreenRenderer = null;
				_renderer.Visible = true;
				_fullscreenMode = false;
				frmMain_Resize(null, EventArgs.Empty);
			};

			Screen currentScreen = Screen.FromHandle(_frm.Handle);
			_frmFullscreenRenderer.StartPosition = FormStartPosition.Manual;
			_frmFullscreenRenderer.Top = currentScreen.Bounds.Top;
			_frmFullscreenRenderer.Left = currentScreen.Bounds.Left;
			_frmFullscreenRenderer.Show();
		}
	}
}
