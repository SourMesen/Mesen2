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
	public class HexEditorInfo
	{
		public bool HighDensityTextMode = false;
		public bool EnablePerByteNavigation = true;
		public bool ByteEditingMode = true;
		public bool AutoRefresh = true;
		public RefreshSpeed AutoRefreshSpeed = RefreshSpeed.Normal;
		public bool IgnoreRedundantWrites = false;
		public bool HighlightCurrentRowColumn = true;
		public int ColumnCount = 2;

		public string FontFamily = BaseControl.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Regular;
		public float FontSize = BaseControl.DefaultFontSize;
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

		public XmlColor ReadColor = Color.Blue;
		public XmlColor WriteColor = Color.Red;
		public XmlColor ExecColor = Color.Green;
		public XmlColor LabelledByteColor = Color.LightPink;
		public XmlColor CodeByteColor = Color.DarkSeaGreen;
		public XmlColor DataByteColor = Color.LightSteelBlue;

		public SnesMemoryType MemoryType = SnesMemoryType.CpuMemory;

		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;
		
		public HexEditorInfo()
		{
		}
	}
}
