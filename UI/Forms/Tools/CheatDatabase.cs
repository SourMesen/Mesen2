using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace Mesen.GUI.Forms
{
	[XmlRoot("cheats")]
	public class CheatDatabase
	{
		[XmlElement("game")]
		public List<CheatDbGameEntry> Games;
	}

	[XmlType("game")]
	public class CheatDbGameEntry
	{
		[XmlAttribute("name")]
		public string Name;

		[XmlAttribute("sha1")]
		public string Sha1;

		[XmlElement("cheat")]
		public List<CheatDbCheatEntry> Cheats;

		public override string ToString()
		{
			return this.Name;
		}
	}

	public class CheatDbCheatEntry
	{
		[XmlAttribute("description")]
		public string Description;

		[XmlElement("code")]
		public List<string> Codes;
	}
}
