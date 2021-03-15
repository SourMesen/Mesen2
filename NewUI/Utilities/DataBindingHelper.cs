using Avalonia.Controls;
using Mesen.Localization;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class DataBindingHelper
	{
		public static void InitializeComboBox(ComboBox combo, Type enumType, params Enum[] hiddenValues)
		{
			int index = combo.SelectedIndex;

			List<ComboBoxEnumValue> values = new List<ComboBoxEnumValue>();
			foreach(Enum value in Enum.GetValues(enumType)) {
				if(hiddenValues.Length == 0 || Array.IndexOf(hiddenValues, value) < 0) {
					values.Add(new ComboBoxEnumValue() { Display = ResourceHelper.GetEnumText(value), Value = value });
				}
			}

			combo.Items = values;
			combo.SelectedIndex = index >= 0 ? index : 0;
		}

		public class ComboBoxEnumValue
		{
			public Enum Value { get; set; }
			public string Display { get; set; }
			
			public override string ToString()
			{
				return this.Display;
			}

			public static implicit operator Enum(ComboBoxEnumValue val) => val.Value;
			public static implicit operator ComboBoxEnumValue(Enum val) => new ComboBoxEnumValue() { Display = ResourceHelper.GetEnumText(val), Value = val };
		}
	}
}
