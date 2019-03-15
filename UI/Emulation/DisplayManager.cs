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

		public DisplayManager(frmMain frm, ctrlRenderer renderer, Panel panel)
		{
			_frm = frm;
			_renderer = renderer;
			_panel = panel;
			_frm.Resize += frmMain_Resize;
		}

		private void frmMain_Resize(object sender, EventArgs e)
		{
			SetScaleBasedOnWindowSize();
		}

		public void UpdateViewerSize(bool resizeForm)
		{
			ScreenSize screenSize = EmuApi.GetScreenSize(false);

			if(resizeForm && _frm.WindowState != FormWindowState.Maximized) {
				_frm.Resize -= frmMain_Resize;
				Size newSize = new Size(screenSize.Width, screenSize.Height);
				_frm.ClientSize = new Size(newSize.Width, newSize.Height + _panel.Top);
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
			/*if(_fullscreenMode && ConfigManager.Config.Video.FullscreenForceIntegerScale) {
				scale = Math.Floor(scale);
			}*/

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
			ConfigManager.Config.Video.VideoScale = scale;
			ConfigManager.Config.Video.ApplyConfig();
			ConfigManager.ApplyChanges();

			UpdateViewerSize(resizeForm);
		}

		public void ToggleFullscreen()
		{

		}
	}
}
