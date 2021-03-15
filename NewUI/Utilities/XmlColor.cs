using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace Mesen.GUI.Utilities
{
	public class XmlColor
	{
		private Color _color = Color.Black;

		public XmlColor() { }
		public XmlColor(Color c) { _color = c; }

		[XmlIgnore]
		public Color Color
		{
			get { return _color; }
			set { _color = value; }
		}

		public static implicit operator Color(XmlColor x)
		{
			return x.Color;
		}

		public static implicit operator XmlColor(Color c)
		{
			return new XmlColor(c);
		}
	}
}
