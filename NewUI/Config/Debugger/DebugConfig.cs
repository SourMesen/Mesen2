using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Mesen.Debugger;

namespace Mesen.Config
{
	public class DebugConfig
	{
		public DebuggerShortcutsConfig Shortcuts { get; set; } = new DebuggerShortcutsConfig();
		public TraceLoggerConfig TraceLogger { get; set; } = new TraceLoggerConfig();
		public HexEditorConfig HexEditor { get; set; } = new HexEditorConfig();
		public EventViewerConfig EventViewer { get; set; } = new EventViewerConfig();
		public DebuggerConfig Debugger { get; set; } = new DebuggerConfig();
		public TilemapViewerConfig TilemapViewer { get; set; } = new TilemapViewerConfig();
		public TileViewerConfig TileViewer { get; set; } = new TileViewerConfig();
		public PaletteViewerConfig PaletteViewer { get; set; } = new PaletteViewerConfig();
		public RegisterViewerConfig RegisterViewer { get; set; } = new RegisterViewerConfig();
		public SpriteViewerConfig SpriteViewer { get; set; } = new SpriteViewerConfig();
		public DbgIntegrationConfig DbgIntegration { get; set; } = new DbgIntegrationConfig();
		public ScriptWindowConfig ScriptWindow { get; set; } = new ScriptWindowConfig();
		public ProfilerConfig Profiler { get; set; } = new ProfilerConfig();
		public AssemblerConfig Assembler { get; set; } = new AssemblerConfig();
		public DebugLogConfig DebugLog { get; set; } = new DebugLogConfig();
		public FontConfig Font { get; set; } = new FontConfig();

		public DebugConfig()
		{		
		}
	}

	public enum RefreshSpeed
	{
		Off = 0,
		Low = 1,
		Normal = 2,
		High = 3
	}

}
