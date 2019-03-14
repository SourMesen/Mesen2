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
using Mesen.GUI.Controls;

namespace Mesen.GUI.Forms.Config
{
	public partial class ctrlStandardController : BaseInputConfigControl
	{
		public ctrlStandardController()
		{
			InitializeComponent();
			if(IsDesignMode) {
				return;
			}
			picBackground.Resize += picBackground_Resize;
			UpdateBackground();
		}

		public int PortNumber { get; set; }

		private void picBackground_Resize(object sender, EventArgs e)
		{
			this.BeginInvoke((Action)(()=> UpdateBackground()));
		}
		
		public void UpdateBackground()
		{
			float xFactor = picBackground.Width / 585f;
			float yFactor = picBackground.Height / 253f;
			Bitmap bitmap = new Bitmap(picBackground.Width, picBackground.Height);
			using(Graphics g = Graphics.FromImage(bitmap)) {
				g.ScaleTransform(xFactor, yFactor);
				using(Pen pen = new Pen(Color.LightGray, 2f)) {
					g.DrawRectangle(pen, 1, 1, 585 - 4, 253 - 4);
					g.DrawEllipse(pen, 15, 55, 170, 170);
					g.FillEllipse(Brushes.WhiteSmoke, 370, 35, 210, 210);
				}
			}
			picBackground.Image = bitmap;
		}

		public override void Initialize(KeyMapping mappings)
		{
			InitButton(btnA, mappings.A);
			InitButton(btnB, mappings.B);
			InitButton(btnStart, mappings.Start);
			InitButton(btnSelect, mappings.Select);
			InitButton(btnUp, mappings.Up);
			InitButton(btnDown, mappings.Down);
			InitButton(btnLeft, mappings.Left);
			InitButton(btnRight, mappings.Right);
			InitButton(btnX, mappings.X);
			InitButton(btnY, mappings.Y);
			InitButton(btnL, mappings.L);
			InitButton(btnR, mappings.R);

			this.OnChange();
		}
	
		public override void UpdateKeyMappings(ref KeyMapping mappings)
		{
			mappings.A = (UInt32)btnA.Tag;
			mappings.B = (UInt32)btnB.Tag;
			mappings.Start = (UInt32)btnStart.Tag;
			mappings.Select = (UInt32)btnSelect.Tag;
			mappings.Up = (UInt32)btnUp.Tag;
			mappings.Down = (UInt32)btnDown.Tag;
			mappings.Left = (UInt32)btnLeft.Tag;
			mappings.Right = (UInt32)btnRight.Tag;
			mappings.X = (UInt32)btnX.Tag;
			mappings.Y = (UInt32)btnY.Tag;
			mappings.L = (UInt32)btnL.Tag;
			mappings.R = (UInt32)btnR.Tag;
		}
	}
}
