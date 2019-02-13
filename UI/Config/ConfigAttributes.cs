using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Config
{
	public class MinMaxAttribute : Attribute
	{
		public object Min { get; set; }
		public object Max { get; set; }

		public MinMaxAttribute(object min, object max)
		{
			this.Min = min;
			this.Max = max;
		}
	}

	public class ValidValuesAttribute : Attribute
	{
		public uint[] ValidValues { get; set; }

		public ValidValuesAttribute(params uint[] validValues)
		{
			this.ValidValues = validValues;
		}
	}
}
