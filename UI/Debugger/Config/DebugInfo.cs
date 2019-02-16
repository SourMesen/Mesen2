using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Mesen.GUI.Debugger;
using Mesen.GUI.Controls;
using Mesen.GUI.Utilities;

namespace Mesen.GUI.Config
{
	public class DebugInfo
	{
		public DebuggerShortcutsConfig Shortcuts = new DebuggerShortcutsConfig();
		public TraceLoggerInfo TraceLogger = new TraceLoggerInfo();
		public HexEditorInfo HexEditor = new HexEditorInfo();

		public bool ShowSelectionLength = false;

		public XmlColor EventViewerBreakpointColor = ColorTranslator.FromHtml("#1898E4");
		public XmlColor CodeOpcodeColor = Color.FromArgb(22, 37, 37);
		public XmlColor CodeLabelDefinitionColor = Color.Blue;
		public XmlColor CodeImmediateColor = Color.Chocolate;
		public XmlColor CodeAddressColor = Color.DarkRed;
		public XmlColor CodeCommentColor = Color.Green;
		public XmlColor CodeEffectiveAddressColor = Color.SteelBlue;

		
		public DebugInfo()
		{		
		}
	}

	public enum RefreshSpeed
	{
		Low = 0,
		Normal = 1,
		High = 2
	}
}
