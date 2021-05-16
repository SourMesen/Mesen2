using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;
using Avalonia;
using Avalonia.Media;
using Mesen.Interop;

namespace Mesen.Config
{
	public class HexEditorConfig
	{
		public bool HighDensityTextMode = false;
		public bool EnablePerByteNavigation = true;
		public bool ByteEditingMode = true;
		public bool AutoRefresh = true;
		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Normal;
		public bool IgnoreRedundantWrites = false;
		public bool HighlightCurrentRowColumn = true;
		public int ColumnCount = 2;

		public string FontFamily = DebuggerConfig.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Normal;
		public float FontSize = DebuggerConfig.DefaultFontSize;
		public int TextZoom = 100;

		public bool ShowCharacters = true;
		public bool ShowLabelInfo = true;
		public bool HighlightExecution = true;
		public bool HighlightWrites = true;
		public bool HighlightReads = true;
		public int FadeSpeed = 300;
		public bool HideUnusedBytes = false;
		public bool HideReadBytes = false;
		public bool HideWrittenBytes = false;
		public bool HideExecutedBytes = false;
		public bool HighlightBreakpoints = false;
		public bool HighlightLabelledBytes = false;
		public bool HighlightCodeBytes = false;
		public bool HighlightDataBytes = false;

		public Color ReadColor = Colors.Blue;
		public Color WriteColor = Colors.Red;
		public Color ExecColor = Colors.Green;
		public Color LabelledByteColor = Colors.LightPink;
		public Color CodeByteColor = Colors.DarkSeaGreen;
		public Color DataByteColor = Colors.LightSteelBlue;

		public SnesMemoryType MemoryType = SnesMemoryType.CpuMemory;

		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;
		
		public HexEditorConfig()
		{
		}
	}
}
