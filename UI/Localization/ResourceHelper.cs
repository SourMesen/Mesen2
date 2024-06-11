using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Xml;
using Mesen.Interop;

namespace Mesen.Localization
{
	class ResourceHelper
	{
		private static XmlDocument _resources = new XmlDocument();

		private static Dictionary<Enum, string> _enumLabelCache = new();
		private static Dictionary<string, string> _viewLabelCache = new();
		private static Dictionary<string, string> _messageCache = new();

		public static void LoadResources()
		{
			try {
				Assembly assembly = Assembly.GetExecutingAssembly();

				using(StreamReader reader = new StreamReader(assembly.GetManifestResourceStream("Mesen.Localization.resources.en.xml")!)) {
					_resources.LoadXml(reader.ReadToEnd());
				}

				foreach(XmlNode node in _resources.SelectNodes("/Resources/Messages/Message")!) {
					_messageCache[node.Attributes!["ID"]!.Value] = node.InnerText;
				}

#pragma warning disable IL2026 // Members annotated with 'RequiresUnreferencedCodeAttribute' require dynamic access otherwise can break functionality when trimming application code
				Dictionary<string, Type> enumTypes = Assembly.GetExecutingAssembly().GetTypes().Where(t => t.IsEnum).ToDictionary(t => t.Name);
#pragma warning restore IL2026 // Members annotated with 'RequiresUnreferencedCodeAttribute' require dynamic access otherwise can break functionality when trimming application code

				foreach(XmlNode node in _resources.SelectNodes("/Resources/Enums/Enum")!) {
					string enumName = node.Attributes!["ID"]!.Value;
					if(enumTypes.TryGetValue(enumName, out Type? enumType)) {
						foreach(XmlNode enumNode in node.ChildNodes) {
							if(Enum.TryParse(enumType, enumNode.Attributes!["ID"]!.Value, out object? value)) {
								_enumLabelCache[(Enum)value!] = enumNode.InnerText;
							}
						}
					} else {
						throw new Exception("Unknown enum type: " + enumName);
					}
				}

				foreach(XmlNode node in _resources.SelectNodes("/Resources/Forms/Form")!) {
					string viewName = node.Attributes!["ID"]!.Value;
					foreach(XmlNode formNode in node.ChildNodes) {
						if(formNode is XmlElement elem) {
							_viewLabelCache[viewName + "_" + elem.Attributes!["ID"]!.Value] = elem.InnerText;
						}
					}
				}
			} catch {
			}
		}

		public static string GetMessage(string id, params object[] args)
		{
			if(_messageCache.TryGetValue(id, out string? text)) {
				return string.Format(text, args);
			} else {
				return "[[" + id + "]]";
			}
		}

		public static string GetEnumText(Enum e)
		{
			if(_enumLabelCache.TryGetValue(e, out string? text)) {
				return text;
			} else {
				return "[[" + e.ToString() + "]]";
			}
		}

		public static Enum[] GetEnumValues(Type t)
		{
			List<Enum> values = new List<Enum>();
			XmlNode? node = _resources.SelectSingleNode("/Resources/Enums/Enum[@ID='" + t.Name + "']");
			if(node?.Attributes!["ID"]!.Value == t.Name) {
				foreach(XmlNode enumNode in node.ChildNodes) {
					if(Enum.TryParse(t, enumNode.Attributes!["ID"]!.Value, out object? value) && value != null) {
						values.Add((Enum)value);
					}
				}
			}
			return values.ToArray();
		}

		public static string GetViewLabel(string view, string control)
		{
			if(_viewLabelCache.TryGetValue(view + "_" + control, out string? text)) {
				return text;
			} else {
				return $"[{view}:{control}]";
			}
		}
	}
}
