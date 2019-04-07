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
	public class TraceLoggerInfo
	{
		public TraceLoggerOptions LogOptions;
		public bool AutoRefresh = true;
		public int LineCount = 1000;

		public Size WindowSize = new Size(0, 0);
		public Point WindowLocation;
		public string FontFamily = BaseControl.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Regular;
		public float FontSize = BaseControl.DefaultFontSize;
		public int TextZoom = 100;

		public TraceLoggerInfo()
		{
			LogOptions = new TraceLoggerOptions() {
				LogCpu = true,
				ShowByteCode = true,
				ShowEffectiveAddresses = true,
				ShowPpuFrames = false,
				ShowPpuCycles = true,
				ShowPpuScanline = true,
				ShowRegisters = true,
				UseLabels = false,
				StatusFormat = StatusFlagFormat.Text
			};
		}
	}

	public class TraceLoggerOptions
	{
		public bool LogCpu;
		public bool LogSpc;

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

	public enum StatusFlagFormat
	{
		Hexadecimal = 0,
		Text = 1,
		CompactText = 2
	}
}
