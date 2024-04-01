using Mesen.Interop;
using ReactiveUI.Fody.Helpers;
using System.Runtime.InteropServices;
using System;

namespace Mesen.Config
{
	public class HdPackBuilderConfig : BaseConfig<HdPackBuilderConfig>
	{
		public ScaleFilterType FilterType { get; set; } = ScaleFilterType.Prescale;
		public UInt32 Scale { get; set; } = 1;
		public UInt32 ChrRamBankSize { get; set; } = 0x1000;

		public bool UseLargeSprites { get; set; } = false;
		public bool SortByUsageFrequency { get; set; } = true;
		public bool GroupBlankTiles { get; set; } = true;
		public bool IgnoreOverscan { get; set; } = false;

		public HdPackBuilderOptions ToInterop(string saveFolder)
		{
			return new HdPackBuilderOptions() {
				SaveFolder = saveFolder,
				FilterType = FilterType,
				Scale = Scale,
				ChrRamBankSize = ChrRamBankSize,
				UseLargeSprites = UseLargeSprites,
				SortByUsageFrequency = SortByUsageFrequency,
				GroupBlankTiles = GroupBlankTiles,
				IgnoreOverscan = IgnoreOverscan,
			};
		}
	}

	public enum ScaleFilterType
	{
		xBRZ = 0,
		HQX = 1,
		Scale2x = 2,
		_2xSai = 3,
		Super2xSai = 4,
		SuperEagle = 5,
		Prescale = 6,
		LcdGrid = 7,
	}

	public struct HdPackBuilderOptions
	{
		[MarshalAs(UnmanagedType.LPUTF8Str)] public string SaveFolder;
		public ScaleFilterType FilterType;
		public UInt32 Scale;
		public UInt32 ChrRamBankSize;

		[MarshalAs(UnmanagedType.I1)] public bool UseLargeSprites;
		[MarshalAs(UnmanagedType.I1)] public bool SortByUsageFrequency;
		[MarshalAs(UnmanagedType.I1)] public bool GroupBlankTiles;
		[MarshalAs(UnmanagedType.I1)] public bool IgnoreOverscan;
	}
}
