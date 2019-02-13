using System.Windows.Forms;
using System.Drawing;
using Mesen.GUI;
using System;
using System.ComponentModel;

namespace Mesen.GUI.Controls
{
	public class BaseControl : UserControl
	{
		public static float DefaultFontSize = Program.IsMono ? 10 : 12;

		public static string MonospaceFontFamily
		{
			get
			{
				if(Program.IsMono) {
					return "DroidSansMono";
				} else {
					return "Consolas";
				}
			}
		}

		private static bool? _isDesignMode = null;
		public bool IsDesignMode
		{
			get
			{
				try {
					if(!_isDesignMode.HasValue) {
						_isDesignMode = System.Diagnostics.Process.GetCurrentProcess().ProcessName == "devenv";
					}
					return _isDesignMode.Value || LicenseManager.UsageMode == LicenseUsageMode.Designtime;
				} catch {
					return false;
				}
			}
		}

		public static Bitmap DownArrow
		{
			get
			{
				if(!Program.IsMono && Environment.OSVersion.Version >= new Version(6, 2)) {
					return Properties.Resources.DownArrowWin10;
				} else {
					return Properties.Resources.DownArrow;
				}
			}
		}

		public new AutoScaleMode AutoScaleMode
		{
			set {
				if(Program.IsMono) { 
					base.AutoScaleMode = AutoScaleMode.None;
				} else {
					base.AutoScaleMode = value;
				}
			}
		}

		public new SizeF AutoScaleDimensions
		{
			set 
			{ 
				if(!Program.IsMono) { 
					base.AutoScaleDimensions = value; 
				}
			}
		} 		
	}
}