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

namespace Mesen.GUI.Debugger.Workspace
{
	public class DebugWorkspace
	{
		public List<Breakpoint> Breakpoints = new List<Breakpoint>();
		public List<string> WatchValues = new List<string>();
		public List<string> SpcWatchValues = new List<string>();
		public List<string> Sa1WatchValues = new List<string>();
		public List<string> GsuWatchValues = new List<string>();
		public List<CodeLabel> CpuLabels = new List<CodeLabel>();
		public List<CodeLabel> SpcLabels = new List<CodeLabel>();
		public List<string> TblMappings = null;
		private string _filePath;

		public static DebugWorkspace GetWorkspace()
		{
			RomInfo info = EmuApi.GetRomInfo();
			return Deserialize(Path.Combine(ConfigManager.DebuggerFolder, info.GetRomName() + ".Workspace.xml"));
		}

		private static DebugWorkspace Deserialize(string path)
		{
			DebugWorkspace config = new DebugWorkspace();

			if(File.Exists(path)) {
				try {
					XmlSerializer xmlSerializer = new XmlSerializer(typeof(DebugWorkspace));
					using(TextReader textReader = new StreamReader(path)) {
						config = (DebugWorkspace)xmlSerializer.Deserialize(textReader);
					}
				} catch { }
			}

			config._filePath = path;

			return config;
		}

		public void Save()
		{
			try {
				XmlWriterSettings ws = new XmlWriterSettings();
				ws.NewLineHandling = NewLineHandling.Entitize;
				ws.Indent = true;

				XmlSerializer xmlSerializer = new XmlSerializer(typeof(DebugWorkspace));
				using(XmlWriter xmlWriter = XmlWriter.Create(_filePath, ws)) {
					xmlSerializer.Serialize(xmlWriter, this);
				}
			} catch {
			}
		}
	}
}
