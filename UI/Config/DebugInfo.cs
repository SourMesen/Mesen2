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
	public class TraceLoggerOptions
	{
		public bool ShowByteCode;
		public bool ShowRegisters;
		public bool ShowCpuCycles;
		public bool ShowPpuCycles;
		public bool ShowPpuScanline;
		public bool ShowPpuFrames;
		public bool ShowExtraInfo;
		public bool IndentCode;
		public bool ShowEffectiveAddresses;
		public bool ShowMemoryValues;
		public bool UseLabels;
		public bool ExtendZeroPage;
		public bool UseWindowsEol = !Program.IsMono;
		
		public StatusFlagFormat StatusFormat;

		public bool OverrideFormat;
		public string Format;
	}

	public class DebugInfo
	{
		public bool ShowSelectionLength = false;

		public XmlColor EventViewerBreakpointColor = ColorTranslator.FromHtml("#1898E4");

		public XmlColor AssemblerOpcodeColor = Color.FromArgb(22, 37, 37);
		public XmlColor AssemblerLabelDefinitionColor = Color.Blue;
		public XmlColor AssemblerImmediateColor = Color.Chocolate;
		public XmlColor AssemblerAddressColor = Color.DarkRed;
		public XmlColor AssemblerCommentColor = Color.Green;
		public XmlColor CodeEffectiveAddressColor = Color.SteelBlue;

		public TraceLoggerOptions TraceLoggerOptions;
		public bool TraceAutoRefresh = true;
		public int TraceLineCount = 1000;
		public Size TraceLoggerSize = new Size(0, 0);
		public Point TraceLoggerLocation;
		public string TraceFontFamily = BaseControl.MonospaceFontFamily;
		public FontStyle TraceFontStyle = FontStyle.Regular;
		public float TraceFontSize = BaseControl.DefaultFontSize;
		public int TraceTextZoom = 100;

		public DebugInfo()
		{
			TraceLoggerOptions = new TraceLoggerOptions() {
				ShowByteCode = true,
				ShowCpuCycles = true,
				ShowEffectiveAddresses = true,
				ShowExtraInfo = true,
				ShowPpuFrames = false,
				ShowPpuCycles = true,
				ShowPpuScanline = true,
				ShowRegisters = true,
				UseLabels = false,
				StatusFormat = StatusFlagFormat.Hexadecimal
			};
		}
	}

	public enum StatusFlagFormat
	{
		Hexadecimal = 0,
		Text = 1,
		CompactText = 2
	}
}
