using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace Mesen.Utilities
{
	[XmlRoot("cheats")]
	public class CheatDatabase
	{
		[XmlElement("game")]
		public List<CheatDbGameEntry> Games { get; set; } = null!;
	}

	[XmlType("game")]
	public class CheatDbGameEntry
	{
		[XmlAttribute("name")]
		public string Name { get; set; } = null!;

		[XmlAttribute("sha1")]
		public string Sha1 { get; set; } = null!;

		[XmlElement("cheat")]
		public List<CheatDbCheatEntry> Cheats { get; set; } = null!;

		public override string ToString()
		{
			return this.Name;
		}
	}

	public class CheatDbCheatEntry
	{
		[XmlAttribute("desc")]
		public string Desc { get; set; } = null!;

		[XmlAttribute("code")]
		public string Code { get; set; } = null!;
	}
}
