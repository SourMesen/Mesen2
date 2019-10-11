using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class ctrlPropertyList : UserControl
	{
		private List<RegEntry> _values = new List<RegEntry>();

		public ctrlPropertyList()
		{
			InitializeComponent();
		}

		protected override void OnResize(EventArgs e)
		{
			base.OnResize(e);
			colAddress.Width = this.ClientSize.Width / 5 - 5;
			colName.Width = (int)(this.ClientSize.Width / 2.6 - 5);
			colValue.Width = this.ClientSize.Width / 5 - 5;
			colValueHex.Width = this.ClientSize.Width / 5 - 5;
		}

		public void UpdateState(List<RegEntry> values)
		{
			lstProperties.BeginUpdate();
			_values = values;
			lstProperties.VirtualListSize = _values.Count;
			lstProperties.EndUpdate();
		}

		private void lstProperties_RetrieveVirtualItem(object sender, RetrieveVirtualItemEventArgs e)
		{
			string val = _values[e.ItemIndex].GetValue();

			e.Item = new ListViewItem(new string[] {
				(val == null ? "" : "  ") + _values[e.ItemIndex].Reg,
				_values[e.ItemIndex].Name,
				val,
				_values[e.ItemIndex].GetHexValue()
			});

			if(val == null) {
				e.Item.BackColor = SystemColors.MenuBar;
			}
		}
	}

	public class RegEntry
	{
		public string Reg;
		public string Name;

		private object _value;
		private Format _format;

		public RegEntry(string reg, string name, object value, Format format = Format.None)
		{
			this.Reg = reg;
			this.Name = name;

			this._value = value;
			this._format = format;
		}

		public string GetValue()
		{
			if(_value is string) {
				return _value as string;
			} else if(_value is bool) {
				return (bool)_value ? "☑ true" : "☐ false";
			} else if(_value is IFormattable) {
				return _value.ToString();
			} else if(_value == null) {
				return null;
			}
			throw new Exception("Unsupported type");
		}

		public string GetHexValue()
		{
			if(_value == null) {
				return "";
			}

			switch(_format) {
				default: return "";
				case Format.X8: return "$" + ((IFormattable)_value).ToString("X2", null);
				case Format.X16: return "$" + ((IFormattable)_value).ToString("X4", null);
				case Format.X24: return "$" + ((IFormattable)_value).ToString("X6", null);
			}
		}

		public enum Format
		{
			None,
			D,
			X8,
			X16,
			X24
		}
	}
}
