using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Controls
{
	class MesenNumericUpDown : BaseControl
	{
		private TextBox txtValue;
		private Button btnUp;
		private Button btnDown;
		private Panel pnlButtons;
		private Timer tmrRepeat;

		private int _repeatCount = 0;
		private bool _repeatIncrease = false;
		private bool _preventClick = false;

		public event EventHandler ValueChanged;

		public MesenNumericUpDown()
		{
			InitializeComponent();

			if(!IsDesignMode) {
				this.BackColor = SystemColors.ControlLightLight;
				this.MaximumSize = new Size(10000, 21);
				this.MinimumSize = new Size(0, 21);
				this.Size = new Size(62, 21);

				this.btnDown.Image = Properties.Resources.NudDownArrow;
				this.btnUp.Image = Properties.Resources.NudUpArrow;

				if(Program.IsMono) {
					this.BorderStyle = BorderStyle.None;
					this.txtValue.Dock = DockStyle.Fill;
					this.txtValue.Multiline = true;
					this.btnUp.Location = new Point(-1, 0);
					this.btnDown.Location = new Point(-1, 10);
					this.MinimumSize = new Size(0, 22);
				} else {
					this.BorderStyle = BorderStyle.FixedSingle;
				}
			}
		}

		private void tmrRepeat_Tick(object sender, EventArgs e)
		{
			tmrRepeat.Interval = _repeatCount > 5 ? 75 : 200;

			if(_repeatIncrease) {
				this.Value += this.Increment;
			} else {
				this.Value -= this.Increment;
			}
			_repeatCount++;
			_preventClick = true;
		}
		
		private void btn_MouseDown(object sender, MouseEventArgs e)
		{
			_repeatCount = 0;
			_repeatIncrease = (sender == btnUp);
			tmrRepeat.Start();
		}

		private void btn_MouseUp(object sender, MouseEventArgs e)
		{
			tmrRepeat.Stop();
		}

		private void btnDown_Click(object sender, EventArgs e)
		{
			if(!_preventClick) {
				this.Value -= this.Increment;
			}
			_preventClick = false;
		}

		private void btnUp_Click(object sender, EventArgs e)
		{
			if(!_preventClick) {
				this.Value += this.Increment;
			}
			_preventClick = false;
		}

		private void txtValue_TextChanged(object sender, EventArgs e)
		{
			decimal val;
			int hexVal;
			if(string.IsNullOrWhiteSpace(txtValue.Text)) {
				SetValue(0, false);
			} else if(!IsHex && decimal.TryParse(txtValue.Text, out val)) {
				SetValue(val, false);
			} else if(IsHex && int.TryParse(txtValue.Text, System.Globalization.NumberStyles.HexNumber, null, out hexVal)) {
				SetValue(hexVal, false);
			}
		}

		private void txtValue_Validated(object sender, EventArgs e)
		{
			decimal val;
			int hexVal;
			if(string.IsNullOrWhiteSpace(txtValue.Text)) {
				SetValue(0, true);
			} else if(!IsHex && decimal.TryParse(txtValue.Text, out val)) {
				SetValue(val, true);
			} else if(IsHex && int.TryParse(txtValue.Text, System.Globalization.NumberStyles.HexNumber, null, out hexVal)) {
				SetValue(hexVal, true);
			} else {
				SetValue(this.Value, true);
			}
		}

		private void SetValue(decimal value, bool updateText)
		{
			value = decimal.Round(value, this.DecimalPlaces);

			if(value > Maximum) {
				value = Maximum;
			}
			if(value < Minimum) {
				value = Minimum;
			}

			if(value != _value) {
				_value = value;
				ValueChanged?.Invoke(this, EventArgs.Empty);
			}
			if(updateText) {
				txtValue.TextChanged -= txtValue_TextChanged;
				if(IsHex) {
					txtValue.Text = ((int)_value).ToString("X");
				} else {
					txtValue.Text = _value.ToString("0" + (this.DecimalPlaces > 0 ? "." : "") + new string('0', this.DecimalPlaces));
				}
				txtValue.TextChanged += txtValue_TextChanged;
			}
		}

		private decimal _value = 0;
		public decimal Value
		{
			get { return _value; }
			set
			{
				SetValue(value, true);
			}
		}

		public bool IsHex { get; set; } = false;

		public new string Text
		{
			get { return txtValue.Text; }
			set { txtValue.Text = value; }
		}

		private decimal _maximum = 100;
		public decimal Maximum
		{
			get { return _maximum; }
			set { _maximum = value; SetValue(this.Value, true); }
		}

		private decimal _minimum = 0;
		public decimal Minimum
		{
			get { return _minimum; }
			set { _minimum = value; SetValue(this.Value, true); }
		}

		public decimal Increment { get; set; } = 1;

		private int _decimalPlaces = 0;
		public int DecimalPlaces
		{
			get { return _decimalPlaces; }
			set { _decimalPlaces = value; }
		}

		protected override void OnSizeChanged(EventArgs e)
		{
			base.OnSizeChanged(e);
			if(this.Height < 21) {
				this.Height = 21;
			}
		}

		private void txtValue_KeyDown(object sender, KeyEventArgs e)
		{
			if(e.KeyCode == Keys.Up) {
				this.Value += this.Increment;
				e.SuppressKeyPress = true;
			} else if(e.KeyCode == Keys.Down) {
				this.Value -= this.Increment;
				e.SuppressKeyPress = true;
			} else if(
				!((e.KeyCode >= Keys.D0 && e.KeyCode <= Keys.D9) ||
				(e.KeyCode >= Keys.NumPad0 && e.KeyCode <= Keys.NumPad9) ||
				e.KeyCode == Keys.OemPeriod || e.KeyCode == Keys.Oemcomma || 
				e.KeyCode == Keys.Delete || e.KeyCode == Keys.Left ||
				e.KeyCode == Keys.Right || e.KeyCode == Keys.Home || 
				e.KeyCode == Keys.End || e.KeyCode == Keys.Back)
			) {
				if(IsHex && e.KeyCode >= Keys.A && e.KeyCode <= Keys.F) {
					//Allow A-F when in hex mode
					return;
				}

				e.SuppressKeyPress = true;
			}
		}

		private void InitializeComponent()
		{
			this.components = new System.ComponentModel.Container();
			this.txtValue = new System.Windows.Forms.TextBox();
			this.btnUp = new System.Windows.Forms.Button();
			this.btnDown = new System.Windows.Forms.Button();
			this.pnlButtons = new System.Windows.Forms.Panel();
			this.pnlButtons.SuspendLayout();
			this.SuspendLayout();
			// 
			// txtValue
			// 
			this.txtValue.Anchor = ((System.Windows.Forms.AnchorStyles)((System.Windows.Forms.AnchorStyles.Left | System.Windows.Forms.AnchorStyles.Right)));
			this.txtValue.BorderStyle = System.Windows.Forms.BorderStyle.None;
			this.txtValue.Location = new System.Drawing.Point(1, 3);
			this.txtValue.Margin = new System.Windows.Forms.Padding(0);
			this.txtValue.Name = "txtValue";
			this.txtValue.Size = new System.Drawing.Size(44, 13);
			this.txtValue.TabIndex = 0;
			this.txtValue.TextChanged += new System.EventHandler(this.txtValue_TextChanged);
			this.txtValue.KeyDown += new System.Windows.Forms.KeyEventHandler(this.txtValue_KeyDown);
			this.txtValue.Validated += new System.EventHandler(this.txtValue_Validated);
			// 
			// btnUp
			// 
			this.btnUp.Location = new System.Drawing.Point(1, -1);
			this.btnUp.Margin = new System.Windows.Forms.Padding(0);
			this.btnUp.Name = "btnUp";
			this.btnUp.Size = new System.Drawing.Size(15, 11);
			this.btnUp.TabIndex = 2;
			this.btnUp.TabStop = false;
			this.btnUp.UseVisualStyleBackColor = true;
			this.btnUp.Click += new System.EventHandler(this.btnUp_Click);
			this.btnUp.MouseDown += new System.Windows.Forms.MouseEventHandler(this.btn_MouseDown);
			this.btnUp.MouseUp += new System.Windows.Forms.MouseEventHandler(this.btn_MouseUp);
			// 
			// btnDown
			// 
			this.btnDown.Location = new System.Drawing.Point(1, 9);
			this.btnDown.Margin = new System.Windows.Forms.Padding(0);
			this.btnDown.Name = "btnDown";
			this.btnDown.Size = new System.Drawing.Size(15, 11);
			this.btnDown.TabIndex = 3;
			this.btnDown.TabStop = false;
			this.btnDown.UseVisualStyleBackColor = true;
			this.btnDown.Click += new System.EventHandler(this.btnDown_Click);
			this.btnDown.MouseDown += new System.Windows.Forms.MouseEventHandler(this.btn_MouseDown);
			this.btnDown.MouseUp += new System.Windows.Forms.MouseEventHandler(this.btn_MouseUp);
			// 
			// pnlButtons
			// 
			this.pnlButtons.Controls.Add(this.btnDown);
			this.pnlButtons.Controls.Add(this.btnUp);
			this.pnlButtons.Dock = System.Windows.Forms.DockStyle.Right;
			this.pnlButtons.Location = new System.Drawing.Point(47, 0);
			this.pnlButtons.Margin = new System.Windows.Forms.Padding(0);
			this.pnlButtons.Name = "pnlButtons";
			this.pnlButtons.Size = new System.Drawing.Size(15, 21);
			this.pnlButtons.TabIndex = 1;
			//
			// tmrRepeat
			//
			this.tmrRepeat = new Timer(components);
			this.tmrRepeat.Tick += tmrRepeat_Tick;
			// 
			// MesenNumericUpDown
			// 
			this.Controls.Add(this.txtValue);
			this.Controls.Add(this.pnlButtons);
			this.MaximumSize = new System.Drawing.Size(10000, 21);
			this.MinimumSize = new System.Drawing.Size(0, 21);
			this.Name = "MesenNumericUpDown";
			this.Size = new System.Drawing.Size(62, 21);
			this.pnlButtons.ResumeLayout(false);
			this.ResumeLayout(false);
			this.PerformLayout();
		}

		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.IContainer components = null;

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		/// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
		protected override void Dispose(bool disposing)
		{
			if(disposing && (components != null)) {
				components.Dispose();
			}
			base.Dispose(disposing);
		}
	}
}
