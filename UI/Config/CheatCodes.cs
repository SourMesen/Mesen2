using Mesen.GUI.Config;
using Mesen.GUI.Debugger;
using Mesen.GUI.Debugger.Labels;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace Mesen.GUI.Config
{
	public class CheatCodes
	{
		private static string FilePath { get { return Path.Combine(ConfigManager.CheatFolder, EmuApi.GetRomInfo().GetRomName() + ".xml"); } }

		public List<CheatCode> Cheats = new List<CheatCode>();

		public static CheatCodes LoadCheatCodes()
		{
			return Deserialize(CheatCodes.FilePath);
		}

		private static CheatCodes Deserialize(string path)
		{
			CheatCodes cheats = new CheatCodes();

			if(File.Exists(path)) {
				try {
					XmlSerializer xmlSerializer = new XmlSerializer(typeof(CheatCodes));
					using(TextReader textReader = new StreamReader(path)) {
						cheats = (CheatCodes)xmlSerializer.Deserialize(textReader);
					}
				} catch { }
			}
			
			return cheats;
		}

		public void Save()
		{
			try {
				XmlWriterSettings ws = new XmlWriterSettings();
				ws.NewLineHandling = NewLineHandling.Entitize;
				ws.Indent = true;

				XmlSerializer xmlSerializer = new XmlSerializer(typeof(CheatCodes));
				using(XmlWriter xmlWriter = XmlWriter.Create(CheatCodes.FilePath, ws)) {
					xmlSerializer.Serialize(xmlWriter, this);
				}
			} catch {
			}
		}

		public static void ApplyCheats()
		{
			if(ConfigManager.Config.Cheats.DisableAllCheats) {
				EmuApi.ClearCheats();
			} else {
				CheatCodes.ApplyCheats(LoadCheatCodes().Cheats);
			}
		}

		public static void ApplyCheats(List<CheatCode> cheats)
		{
			List<UInt32> encodedCheats = new List<uint>();
			foreach(CheatCode cheat in cheats) {
				if(cheat.Enabled) {
					encodedCheats.AddRange(cheat.GetEncodedCheats());
				}
			}

			EmuApi.SetCheats(encodedCheats.ToArray(), (UInt32)encodedCheats.Count);
		}
	}

	public class CheatCode
	{
		private static string _convertTable = "DF4709156BC8A23E";

		public string Description;
		public CheatFormat Format;
		public bool Enabled;
		public string Codes;

		public List<UInt32> GetEncodedCheats()
		{
			List<UInt32> encodedCheats = new List<UInt32>();
			foreach(string code in this.Codes.Split('\n')) {
				if(code.Trim().Length > 0) {
					encodedCheats.Add(GetEncodedCheat(code, this.Format));
				}
			}
			return encodedCheats;
		}

		public override string ToString()
		{
			StringBuilder sb = new StringBuilder();
			foreach(string code in this.Codes.Split('\n')) {
				if(code.Trim().Length > 0) {
					if(sb.Length != 0) {
						sb.Append(", ");
					}
					sb.Append(code);
				}
			}
			return sb.ToString();
		}

		public static UInt32 GetEncodedCheat(string code, CheatFormat format)
		{
			code = code.Trim();
			UInt32 encodedCheat = 0;
			if(format == CheatFormat.ProActionReplay && UInt32.TryParse(code, System.Globalization.NumberStyles.HexNumber, null, out encodedCheat)) {
				return encodedCheat;
			} else if(format == CheatFormat.GameGenie) {
				return ConvertGameGenie(code);
			}

			throw new Exception("Invalid cheat code");
		}

		public static uint ConvertGameGenie(string code)
		{
			code = code.Trim().ToUpper();

			uint rawValue = 0;
			for(int i = 0; i < code.Length; i++) {
				if(code[i] != '-') {
					rawValue <<= 4;
					rawValue |= (uint)_convertTable.IndexOf(code[i]);
				}
			}

			uint address = (
				((rawValue & 0x3C00) << 10) |
				((rawValue & 0x3C) << 14) |
				((rawValue & 0xF00000) >> 8) |
				((rawValue & 0x03) << 10) |
				((rawValue & 0xC000) >> 6) |
				((rawValue & 0xF0000) >> 12) |
				((rawValue & 0x3C0) >> 6)
			);

			uint value = rawValue >> 24;

			return ((address << 8) | value);
		}
	}

	public enum CheatFormat
	{
		GameGenie = 0,
		ProActionReplay = 1,
	}
}
