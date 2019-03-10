using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	public partial class ctrlTrackbar : BaseControl
	{
		public event EventHandler ValueChanged
		{
			add { trackBar.ValueChanged += value; }
			remove { trackBar.ValueChanged -= value; }
		}

		public ctrlTrackbar()
		{
			InitializeComponent();

			if(!Program.IsMono) {
				this.trackBar.BackColor = System.Drawing.SystemColors.ControlLightLight;
			}
		}

		public int TickFrequency
		{
			get { return trackBar.TickFrequency; }
			set { trackBar.TickFrequency = value; }
		}

		public int SmallChange
		{
			get { return trackBar.SmallChange; }
			set { trackBar.SmallChange = value; }
		}

		public int LargeChange
		{
			get { return trackBar.LargeChange; }
			set { trackBar.LargeChange = value; }
		}

		public int Minimum
		{
			get { return trackBar.Minimum; }
			set { trackBar.Minimum = value; }
		}

		public int Maximum
		{
			get { return trackBar.Maximum; }
			set { trackBar.Maximum = value; }
		}

		public int Multiplier { get; set; } = 1;

		[Bindable(true)]
		[Browsable(true)]
		[DesignerSerializationVisibility(DesignerSerializationVisibility.Visible)]
		[EditorBrowsable(EditorBrowsableState.Always)]
		public override string Text
		{
			get { return lblText.Text; }
			set { lblText.Text = value; }
		}

		public int Value
		{
			get { return trackBar.Value * Multiplier; }
			set
			{
				trackBar.Value = value / Multiplier;
				UpdateText();
			}
		}

		private void UpdateText()
		{
			if(this.Minimum == 0) {
				lblValue.Text = trackBar.Value.ToString() + "%";
			} else {
				lblValue.Text = (trackBar.Value / (double)Multiplier).ToString() + "dB";
				lblValue.Font = new Font("Microsoft Sans Serif", 6.75F);
			}
		}

		private void trackBar_ValueChanged(object sender, EventArgs e)
		{
			UpdateText();
		}
	}
}
