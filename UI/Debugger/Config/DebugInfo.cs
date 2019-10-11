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
		public EventViewerConfig EventViewer = new EventViewerConfig();
		public DebuggerInfo Debugger = new DebuggerInfo();
		public TilemapViewerConfig TilemapViewer = new TilemapViewerConfig();
		public TileViewerConfig TileViewer = new TileViewerConfig();
		public RegisterViewerConfig RegisterViewer = new RegisterViewerConfig();
		public SpriteViewerConfig SpriteViewer = new SpriteViewerConfig();
		public DbgIntegrationConfig DbgIntegration = new DbgIntegrationConfig();
		public ScriptWindowConfig ScriptWindow = new ScriptWindowConfig();

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
