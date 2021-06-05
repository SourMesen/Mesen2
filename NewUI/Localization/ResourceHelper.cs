using System;
using System.IO;
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
			} catch {
			}
		}

		public static string GetMessage(string id, params object[] args)
		{
			var baseNode = _resources.SelectSingleNode("/Resources/Messages/Message[@ID='" + id + "']");
			if(baseNode == null) {
				baseNode = _enResources.SelectSingleNode("/Resources/Messages/Message[@ID='" + id + "']");
			}

			if(baseNode != null) {
				return string.Format(baseNode.InnerText, args);
			} else {
				return "[[" + id + "]]";
			}
		}

		public static string GetEnumText(Enum e)
		{
			var baseNode = _resources.SelectSingleNode("/Resources/Enums/Enum[@ID='" + e.GetType().Name + "']/Value[@ID='" + e.ToString() + "']");
			if(baseNode == null) {
				baseNode = _enResources.SelectSingleNode("/Resources/Enums/Enum[@ID='" + e.GetType().Name + "']/Value[@ID='" + e.ToString() + "']");
			}

			if(baseNode != null) {
				return baseNode.InnerText;
			} else {
				return "[[" + e.ToString() + "]]";
			}
		}

		public static string GetViewLabel(string view, string control)
		{
			var baseNode = _resources.SelectSingleNode("/Resources/Forms/Form[@ID='" + view + "']/Control[@ID='" + control + "']");
			if(baseNode == null) {
				baseNode = _enResources.SelectSingleNode("/Resources/Forms/Form[@ID='" + view + "']/Control[@ID='" + control + "']");
			}

			if(baseNode != null) {
				return baseNode.InnerText;
			} else {
				return $"[{view}:{control}]";
			}
		}
	}
}
