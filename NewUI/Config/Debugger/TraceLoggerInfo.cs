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
using Mesen.GUI.Debugger;
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
		public string FontFamily = DebuggerConfig.MonospaceFontFamily;
		public FontStyle FontStyle = FontStyle.Normal;
		public float FontSize = DebuggerConfig.DefaultFontSize;
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
		public bool LogNecDsp;
		public bool LogSa1;
		public bool LogGsu;
		public bool LogCx4;
		public bool LogGameboy;

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
		public bool UseWindowsEol = true; //TODO

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
