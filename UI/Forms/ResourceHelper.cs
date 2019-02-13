using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.Windows.Forms.Layout;
using System.Xml;
using Mesen.GUI.Config;
using Mesen.GUI.Controls;

namespace Mesen.GUI.Forms
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

			using(Stream stream = ResourceExtractor.GetZippedResource(filename)) {
				_resources.Load(stream);
			}

			using(Stream stream = ResourceExtractor.GetZippedResource(enFilename)) {
				_enResources.Load(stream);
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
				return e.ToString();
			}
		}
		
		public static void ApplyResources(Form form)
		{
			ApplyResources(form, form.Name);
		}

		public static void ApplyResources(Form form, string formName)
		{
			XmlNode baseNode = _resources.SelectSingleNode("/Resources/Forms/Form[@ID='" + formName + "']");

			if(baseNode != null) {
				if(baseNode.Attributes["Title"] != null) {
					form.Text = baseNode.Attributes["Title"].Value;
				}
				ApplyResources(baseNode, form.Controls);
			}
		}

		public static void ApplyResources(Form form, ContextMenuStrip contextMenu)
		{
			XmlNode baseNode = _resources.SelectSingleNode("/Resources/Forms/Form[@ID='" + form.Name + "']");

			if(baseNode != null) {
				ApplyResources(baseNode, contextMenu.Items);
			}
		}
		
		private static void ApplyResources(XmlNode baseNode, UserControl control)
		{
			XmlNode controlNode = _resources.SelectSingleNode("/Resources/UserControls/UserControl[@ID='" + control.GetType().Name + "']");

			if(controlNode != null) {
				ApplyResources(controlNode, control.Controls);
			} else {
				ApplyResources(baseNode, control.Controls);
			}
		}

		private static void ApplyResources(XmlNode baseNode, IEnumerable container)
		{
			foreach(object ctrl in container) {
				string name = null;
				if(ctrl is Control) {
					name = ((Control)ctrl).Name;
				} else if(ctrl is ToolStripItem) {
					name = ((ToolStripItem)ctrl).Name;
				} else if(ctrl is ColumnHeader) {
					name = ((ColumnHeader)ctrl).Name;
				} else if(ctrl is DataGridViewColumn) {
					name = ((DataGridViewColumn)ctrl).Name;
				}

				var controlNode = baseNode.SelectSingleNode("Control[@ID='" + name + "']");
				if(controlNode != null) {
					if(ctrl is Control) {
						((Control)ctrl).Text = controlNode.InnerText;
					} else if(ctrl is ToolStripItem) {
						((ToolStripItem)ctrl).Text = controlNode.InnerText;
						if(((ToolStripItem)ctrl).DisplayStyle != ToolStripItemDisplayStyle.Image) {
							((ToolStripItem)ctrl).ToolTipText = "";
						}
					} else if(ctrl is ColumnHeader) {
						((ColumnHeader)ctrl).Text = controlNode.InnerText;
					} else if(ctrl is DataGridViewColumn) {
						((DataGridViewColumn)ctrl).HeaderText = controlNode.InnerText;
					}
				}

				if(ctrl is DataGridView) {
					ApplyResources(baseNode, ((DataGridView)ctrl).Columns);
				} else if(ctrl is MenuStrip) {
					ApplyResources(baseNode, ((MenuStrip)ctrl).Items);
				} else if(ctrl is ContextMenuStrip) {
					ApplyResources(baseNode, ((ContextMenuStrip)ctrl).Items);
				} else if(ctrl is ListView) {
					ApplyResources(baseNode, ((ListView)ctrl).Columns);
				} else if(ctrl is ToolStrip) {
					ApplyResources(baseNode, ((ToolStrip)ctrl).Items);
				} else if(ctrl is ToolStripSplitButton) {
					ApplyResources(baseNode, ((ToolStripSplitButton)ctrl).DropDownItems);
				} else if(ctrl is UserControl) {
					ApplyResources(baseNode, ctrl as UserControl);
				} else if(ctrl is Control) {
					ApplyResources(baseNode, ((Control)ctrl).Controls);
				} else if(ctrl is ToolStripMenuItem) {
					ApplyResources(baseNode, ((ToolStripMenuItem)ctrl).DropDownItems);
				}

				if(ctrl is Control) {
					if(((Control)ctrl).ContextMenuStrip != null) {
						ApplyResources(baseNode, ((Control)ctrl).ContextMenuStrip.Items);
					}
				}
			}
		}

		private static XmlDocument BuildResourceFile(Form form)
		{
			XmlDocument document = new XmlDocument();
			XmlNode resources = document.CreateElement("Resources");
			document.AppendChild(resources);
			resources.AppendChild(document.CreateElement("Forms"));

			BuildResourceFile(document, form, form.Controls);

			return document;
		}

		private static void BuildResourceFile(XmlDocument xmlDoc, Form form, IEnumerable container)
		{
			var baseNode = xmlDoc.SelectSingleNode("/Resources/Forms/Form[@ID='" + form.Name + "']");
			if(baseNode == null) {
				baseNode = xmlDoc.CreateElement("Form");
				baseNode.Attributes.Append(xmlDoc.CreateAttribute("ID"));
				baseNode.Attributes.Append(xmlDoc.CreateAttribute("Title"));
				baseNode.Attributes["ID"].Value = form.Name;
				baseNode.Attributes["Title"].Value = form.Text;
				xmlDoc.SelectSingleNode("/Resources/Forms").AppendChild(baseNode);
			}

			foreach(Component ctrl in container) {
				string text = null;
				string name = null;
				if(ctrl is Control) {
					text = ((Control)ctrl).Text;
					name = ((Control)ctrl).Name;
				} else if(ctrl is ToolStripItem) {
					text = ((ToolStripItem)ctrl).Text;
					name = ((ToolStripItem)ctrl).Name;
				}

				if(!string.IsNullOrWhiteSpace(text)) {
					var controlNode = baseNode.SelectSingleNode("Control[@ID='" + name + "']");
					if(controlNode == null) {
						controlNode = xmlDoc.CreateElement("Control");
						controlNode.Attributes.Append(xmlDoc.CreateAttribute("ID"));
						controlNode.Attributes["ID"].Value = name;
						baseNode.AppendChild(controlNode);
					}
					controlNode.InnerText = text;
				}
				if(ctrl is MenuStrip) {
					BuildResourceFile(xmlDoc, form, ((MenuStrip)ctrl).Items);
				} else if(ctrl is Control) {
					BuildResourceFile(xmlDoc, form, ((Control)ctrl).Controls);
				} else if(ctrl is ToolStripMenuItem) {
					BuildResourceFile(xmlDoc, form, ((ToolStripMenuItem)ctrl).DropDownItems);
				}
			}
		}
	}
}
