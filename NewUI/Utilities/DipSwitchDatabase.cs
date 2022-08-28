using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Xml;

namespace Mesen.Utilities
{
	public class DipSwitchDatabase
	{
		private static Dictionary<uint, GameDipSwitches> _gameDipswitches = new();

		public static GameDipSwitches GetGameDipswitches(InteropDipSwitchInfo dipSwitches)
		{
			if(_gameDipswitches.TryGetValue(dipSwitches.DatabaseId, out GameDipSwitches? switches)) {
				return JsonHelper.Clone<GameDipSwitches>(switches);
			} else {
				GameDipSwitches gameSwitches = new();
				for(int i = 0; i < dipSwitches.DipSwitchCount; i++) {
					DipSwitchDefinition def = new("Unknown Switch #" + (i+1));
					def.Options.Add("Off");
					def.Options.Add("On");
					gameSwitches.DipSwitches.Add(def);
				}
				return gameSwitches;
			}
		}

		static DipSwitchDatabase()
		{
			using StreamReader reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Utilities.DipSwitchDefinitions.xml")!);
			XmlDocument config = new XmlDocument();
			config.Load(reader);

			XmlNodeList gameNodes = config.SelectNodes("/DipSwitchDefinitions/Game") ?? throw new Exception("Invalid entry");
			foreach(XmlNode gameNode in gameNodes) {
				GameDipSwitches gameDipswitches = new();
				string crcValues = gameNode.Attributes?["PrgCrc32"]?.Value ?? "";
				foreach(string crc in crcValues.Split(',')) {
					if(gameNode.Attributes?["DefaultDip"]?.Value is string defaultDip) {
						gameDipswitches.DefaultDipSwitches = UInt32.Parse(defaultDip);
					}
					gameDipswitches.DipSwitches = new List<DipSwitchDefinition>();

					int unknownCount = 0;
					XmlNodeList dipSwitchNodes = gameNode.SelectNodes("DipSwitch") ?? throw new Exception("Invalid entry");
					foreach(XmlNode dipSwitch in dipSwitchNodes) {
						if(dipSwitch.Attributes?["Localization"] != null) {
							string name = dipSwitch.Attributes?["Localization"]?.Value ?? "n/a";
							DipSwitchDefinition def = new(name);

							XmlNodeList optionNodes = dipSwitch.SelectNodes("Option") ?? throw new Exception("Invalid entry");
							foreach(XmlNode option in optionNodes) {
								def.Options.Add(option.InnerText);
							}
							gameDipswitches.DipSwitches.Add(def);
						} else {
							DipSwitchDefinition def = new("Unknown Switch #" + (unknownCount+1));
							def.Options.Add("Off");
							def.Options.Add("On");
							unknownCount++;
							gameDipswitches.DipSwitches.Add(def);
						}
					}
					_gameDipswitches[uint.Parse(crc, System.Globalization.NumberStyles.HexNumber)] = gameDipswitches;
				}
			}
		}
	}

	public class DipSwitchDefinition
	{
		public string Name { get; set; }
		public List<string> Options { get; set; } = new();
		public int SelectedOption { get; set; } = 0;
		public int SwitchCount => (int)Math.Log2(Options.Count);

		public DipSwitchDefinition(string name)
		{
			Name = name;
		}
	}

	public class GameDipSwitches
	{
		public UInt32 DefaultDipSwitches { get; set; } = 0;
		public List<DipSwitchDefinition> DipSwitches { get; set; } = new();
	}
}
