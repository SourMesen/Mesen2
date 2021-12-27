using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Xml;
using Mesen.Interop;

namespace Mesen.Localization
{
	public enum Language
	{
		SystemDefault = 0,
		English = 1,
		French = 2,
		Japanese = 3,
		Russian = 4,
		Spanish = 5,
		Ukrainian = 6,
		Portuguese = 7,
		Catalan = 8,
		Chinese = 9,
	}

	class ResourceHelper
	{
		private static Language _language;
		private static XmlDocument _resources = new XmlDocument();
		private static XmlDocument _enResources = new XmlDocument();

		private static Dictionary<Enum, string> _enumLabelCache = new();
		private static Dictionary<string, string> _viewLabelCache = new();
		private static Dictionary<string, string> _messageCache = new();

		public static Language GetCurrentLanguage()
		{
			return _language;
		}

		public static string GetLanguageCode()
		{
			switch(ResourceHelper.GetCurrentLanguage()) {
				case Language.English: return "en";
				case Language.French: return "fr";
				case Language.Japanese: return "ja";
				case Language.Russian: return "ru";
				case Language.Spanish: return "es";
				case Language.Ukrainian: return "uk";
				case Language.Portuguese: return "pt";
				case Language.Catalan: return "ca";
				case Language.Chinese: return "zh";
			}

			return "";
		}

		public static void UpdateEmuLanguage()
		{
			EmuApi.SetDisplayLanguage(_language);
		}

		public static void LoadResources(Language language)
		{
			if(language == Language.SystemDefault) {
				switch(System.Globalization.CultureInfo.CurrentUICulture.TwoLetterISOLanguageName) {
					default:
					case "en": language = Language.English; break;
					case "fr": language = Language.French; break;
					case "ja": language = Language.Japanese; break;
					case "ru": language = Language.Russian; break;
					case "es": language = Language.Spanish; break;
					case "uk": language = Language.Ukrainian; break;
					case "pt": language = Language.Portuguese; break;
					case "zh": language = Language.Chinese; break;
				}
			}

			string filename;
			string enFilename = "resources.en.xml";
			switch(language) {
				default:
				case Language.English: filename = enFilename; break;
				case Language.French: filename = "resources.fr.xml"; break;
				case Language.Japanese: filename = "resources.ja.xml"; break;
				case Language.Russian: filename = "resources.ru.xml"; break;
				case Language.Spanish: filename = "resources.es.xml"; break;
				case Language.Ukrainian: filename = "resources.uk.xml"; break;
				case Language.Portuguese: filename = "resources.pt.xml"; break;
				case Language.Catalan: filename = "resources.ca.xml"; break;
				case Language.Chinese: filename = "resources.zh.xml"; break;
			}

			_language = language;

			try {
				Assembly assembly = Assembly.GetExecutingAssembly();
				using(StreamReader reader = new StreamReader(assembly.GetManifestResourceStream("Mesen.Localization." + filename)!)) {
					_resources.LoadXml(reader.ReadToEnd());
				}

				using(StreamReader reader = new StreamReader(assembly.GetManifestResourceStream("Mesen.Localization.resources.en.xml")!)) {
					_enResources.LoadXml(reader.ReadToEnd());
				}

				foreach(XmlNode node in _resources.SelectNodes("/Resources/Messages/Message")!) {
					_messageCache[node.Attributes!["ID"]!.Value] = node.InnerText;
				}

				Dictionary<string, Type> enumTypes = Assembly.GetExecutingAssembly().GetTypes().Where(t => t.IsEnum).ToDictionary(t => t.Name);
				foreach(XmlNode node in _resources.SelectNodes("/Resources/Enums/Enum")!) {
					if(enumTypes.TryGetValue(node.Attributes!["ID"]!.Value!, out Type? enumType)) {
						foreach(XmlNode enumNode in node.ChildNodes) {
							if(Enum.TryParse(enumType, enumNode.Attributes!["ID"]!.Value, out object? value)) {
								_enumLabelCache[(Enum)value!] = enumNode.InnerText;
							}
						}
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
