using Mesen.GUI.Config;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Forms.Config
{
	public partial class frmGameboyConfig : BaseConfigForm
	{
		public frmGameboyConfig()
		{
			InitializeComponent();
			if(DesignMode) {
				return;
			}

			Entity = ConfigManager.Config.Gameboy.Clone();

			AddBinding(nameof(GameboyConfig.Model), cboGameboyModel);
			AddBinding(nameof(GameboyConfig.UseSgb2), chkUseSgb2);
			AddBinding(nameof(GameboyConfig.BlendFrames), chkGbBlendFrames);
			AddBinding(nameof(GameboyConfig.GbcAdjustColors), chkGbcAdjustColors);
			
			AddBinding(nameof(GameboyConfig.BgColor0), picGbBgPal0);
			AddBinding(nameof(GameboyConfig.BgColor1), picGbBgPal1);
			AddBinding(nameof(GameboyConfig.BgColor2), picGbBgPal2);
			AddBinding(nameof(GameboyConfig.BgColor3), picGbBgPal3);

			AddBinding(nameof(GameboyConfig.Obj0Color0), picGbObj0Pal0);
			AddBinding(nameof(GameboyConfig.Obj0Color1), picGbObj0Pal1);
			AddBinding(nameof(GameboyConfig.Obj0Color2), picGbObj0Pal2);
			AddBinding(nameof(GameboyConfig.Obj0Color3), picGbObj0Pal3);

			AddBinding(nameof(GameboyConfig.Obj1Color0), picGbObj1Pal0);
			AddBinding(nameof(GameboyConfig.Obj1Color1), picGbObj1Pal1);
			AddBinding(nameof(GameboyConfig.Obj1Color2), picGbObj1Pal2);
			AddBinding(nameof(GameboyConfig.Obj1Color3), picGbObj1Pal3);
		}

		protected override void OnApply()
		{
			ConfigManager.Config.Gameboy = (GameboyConfig)this.Entity;
			ConfigManager.ApplyChanges();
		}

		private void btnSelectPreset_Click(object sender, EventArgs e)
		{
			ctxGbColorPresets.Show(btnSelectPreset.PointToScreen(new Point(0, btnSelectPreset.Height - 1)));
		}

		private void mnuGbColorPresetGrayscale_Click(object sender, EventArgs e)
		{
			SetPalette(Color.FromArgb(232, 232, 232), Color.FromArgb(160, 160, 160), Color.FromArgb(88, 88, 88), Color.FromArgb(16, 16, 16));
		}

		private void mnuGbColorPresetGrayscaleConstrast_Click(object sender, EventArgs e)
		{
			SetPalette(Color.FromArgb(255, 255, 255), Color.FromArgb(176, 176, 176), Color.FromArgb(104, 104, 104), Color.FromArgb(0, 0, 0));
		}

		private void mnuGbColorPresetGreen_Click(object sender, EventArgs e)
		{
			SetPalette(Color.FromArgb(224, 248, 208), Color.FromArgb(136, 192, 112), Color.FromArgb(52, 104, 86), Color.FromArgb(8, 24, 32));
		}

		private void mnuGbColorPresetBrown_Click(object sender, EventArgs e)
		{
			SetPalette(Color.FromArgb(248, 224, 136), Color.FromArgb(216, 176, 88), Color.FromArgb(152, 120, 56), Color.FromArgb(72, 56, 24));
		}

		private void SetPalette(Color color0, Color color1, Color color2, Color color3)
		{
			picGbBgPal0.BackColor = color0;
			picGbBgPal1.BackColor = color1;
			picGbBgPal2.BackColor = color2;
			picGbBgPal3.BackColor = color3;

			picGbObj0Pal0.BackColor = color0;
			picGbObj0Pal1.BackColor = color1;
			picGbObj0Pal2.BackColor = color2;
			picGbObj0Pal3.BackColor = color3;

			picGbObj1Pal0.BackColor = color0;
			picGbObj1Pal1.BackColor = color1;
			picGbObj1Pal2.BackColor = color2;
			picGbObj1Pal3.BackColor = color3;
		}
	}
}
