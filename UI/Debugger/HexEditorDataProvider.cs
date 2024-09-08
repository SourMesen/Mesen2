using System;
using System.Linq;
using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.Config;
using Mesen.Debugger;
using Mesen.Interop;
using Mesen.Debugger.Labels;
using Mesen.Debugger.Utilities;

namespace Mesen.Debugger
{
	public class HexEditorDataProvider : IHexEditorDataProvider
	{
		public int Length { get; private set; }

		private MemoryType _memoryType;
		private CpuType _cpuType;
		private HexEditorConfig _cfg;
		private AddressCounters[] _counters = Array.Empty<AddressCounters>();
		private CdlFlags[]? _cdlData;
		private bool[] _hasLabel = Array.Empty<bool>();
		private TimingInfo _timing;
		private ByteInfo _byteInfo = new ByteInfo();
		private BreakpointTypeFlags[]? _breakpointTypes;
		private byte[] _data = Array.Empty<byte>();
		private long _firstByteIndex = 0;
		private TblByteCharConverter? _tblConverter = null;
		private byte[]? _frozenAddresses = null;

		public HexEditorDataProvider(MemoryType memoryType, HexEditorConfig cfg, TblByteCharConverter? tblConverter)
		{
			_memoryType = memoryType;
			_cpuType = memoryType.ToCpuType();
			_cfg = cfg;
			_tblConverter = tblConverter;
			Length = DebugApi.GetMemorySize(memoryType);
		}

		public byte[] GetRawBytes(int start, int length)
		{
			return DebugApi.GetMemoryValues(_memoryType, (uint)start, (uint)(start + length - 1));
		}

		public void Prepare(int firstByteIndex, int lastByteIndex)
		{
			if(firstByteIndex >= Length) {
				return;
			}
			if(lastByteIndex >= Length) {
				lastByteIndex = Length - 1;
			}

			_data = DebugApi.GetMemoryValues(_memoryType, (uint)firstByteIndex, (uint)lastByteIndex);

			_firstByteIndex = firstByteIndex;
			int visibleByteCount = (int)(lastByteIndex - firstByteIndex + 1);

			if(_cfg.HighlightBreakpoints) {
				Breakpoint[] breakpoints = BreakpointManager.Breakpoints.ToArray();
				_breakpointTypes = new BreakpointTypeFlags[visibleByteCount];

				for(int i = 0; i < visibleByteCount; i++) {
					int byteIndex = i + (int)firstByteIndex;
					foreach(Breakpoint bp in breakpoints) {
						if(bp.Enabled && bp.Matches((uint)byteIndex, _memoryType, null)) {
							_breakpointTypes[i] = bp.BreakOnExec ? BreakpointTypeFlags.Execute : (bp.BreakOnWrite ? BreakpointTypeFlags.Write : BreakpointTypeFlags.Read);
							break;
						}
					}
				}
			} else {
				_breakpointTypes = null;
			}

			_counters = DebugApi.GetMemoryAccessCounts((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType);

			if(_cfg.FrozenHighlight.Highlight && _memoryType.IsRelativeMemory() && !_memoryType.IsPpuMemory()) {
				_frozenAddresses = DebugApi.GetFrozenState(_cpuType, (UInt32)firstByteIndex, (UInt32)(firstByteIndex + visibleByteCount));
			} else {
				_frozenAddresses = null;
			}

			_cdlData = null;
			if(_memoryType.SupportsCdl()) {
				if(_cfg.DataHighlight.Highlight || _cfg.CodeHighlight.Highlight || (_cpuType == CpuType.Nes && (_cfg.NesDrawnChrRomHighlight.Highlight || _cfg.NesPcmDataHighlight.Highlight))) {
					_cdlData = DebugApi.GetCdlData((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType);
				}
			}

			_hasLabel = new bool[visibleByteCount];
			if(_cfg.LabelHighlight.Highlight) {
				if(_memoryType.IsRelativeMemory()) {
					AddressInfo addr = new AddressInfo();
					addr.Type = _memoryType;
					for(long i = 0; i < _hasLabel.Length; i++) {
						addr.Address = (int)(firstByteIndex + i);
						_hasLabel[i] = !string.IsNullOrWhiteSpace(LabelManager.GetLabel(addr)?.Label);
					}
				} else if(_memoryType.SupportsLabels()) {
					for(long i = 0; i < _hasLabel.Length; i++) {
						_hasLabel[i] = !string.IsNullOrWhiteSpace(LabelManager.GetLabel((uint)(firstByteIndex + i), _memoryType)?.Label);
					}
				}
			}

			_timing = EmuApi.GetTimingInfo(_cpuType);
		}

		public static Color DarkerColor(byte alpha, Color input, double brightnessPercentage)
		{
			if(double.IsInfinity(brightnessPercentage)) {
				brightnessPercentage = 1.0;
			}
			if(brightnessPercentage < 0.20) {
				brightnessPercentage *= 5;
			} else {
				brightnessPercentage = 1.0;
			}
			return Color.FromArgb(alpha, (byte)(input.R * brightnessPercentage), (byte)(input.G * brightnessPercentage), (byte)(input.B * brightnessPercentage));
		}

		public ByteInfo GetByte(int byteIndex)
		{
			int cyclesPerFrame = (int)(_timing.MasterClockRate / _timing.Fps);
			long index = byteIndex - _firstByteIndex;
			double framesSinceExec = (double)(_timing.MasterClock - _counters[index].ExecStamp) / cyclesPerFrame;
			double framesSinceWrite = (double)(_timing.MasterClock - _counters[index].WriteStamp) / cyclesPerFrame;
			double framesSinceRead = (double)(_timing.MasterClock - _counters[index].ReadStamp) / cyclesPerFrame;

			bool isRead = _counters[index].ReadStamp > 0;
			bool isWritten = _counters[index].WriteStamp > 0;
			bool isExecuted = _counters[index].ExecStamp > 0;
			bool isUnused = !isRead && !isWritten && !isExecuted;

			byte alpha = 0;
			if(isRead && _cfg.HideReadBytes || isWritten && _cfg.HideWrittenBytes || isExecuted && _cfg.HideExecutedBytes || isUnused && _cfg.HideUnusedBytes) {
				alpha = 128;
			}
			if(isRead && !_cfg.HideReadBytes || isWritten && !_cfg.HideWrittenBytes || isExecuted && !_cfg.HideExecutedBytes || isUnused && !_cfg.HideUnusedBytes) {
				alpha = 255;
			}

			_byteInfo.BackColor = Colors.Transparent;
			if(_cdlData != null) {
				if(_memoryType.IsPpuMemory()) {
					if(_cpuType == CpuType.Nes && _cdlData[index].HasFlag(CdlFlags.NesChrDrawn) && _cfg.NesDrawnChrRomHighlight.Highlight) {
						_byteInfo.BackColor = _cfg.NesDrawnChrRomHighlight.Color;
					}
				} else {
					if(_cpuType == CpuType.Nes && _cdlData[index].HasFlag(CdlFlags.NesPcmData) && _cfg.NesPcmDataHighlight.Highlight) {
						//NES PCM data
						_byteInfo.BackColor = _cfg.NesPcmDataHighlight.Color;
					} else if(_cdlData[index].HasFlag(CdlFlags.Code) && _cfg.CodeHighlight.Highlight) {
						//Code
						_byteInfo.BackColor = _cfg.CodeHighlight.Color;
					} else if(_cdlData[index].HasFlag(CdlFlags.Data) && _cfg.DataHighlight.Highlight) {
						//Data
						_byteInfo.BackColor = _cfg.DataHighlight.Color;
					} 
				}
			}

			if(_hasLabel[index]) {
				//Labels/comments
				_byteInfo.BackColor = _cfg.LabelHighlight.Color;
			}

			_byteInfo.BorderColor = Colors.Transparent;
			if(_breakpointTypes != null) {
				switch(_breakpointTypes[index]) {
					case BreakpointTypeFlags.Execute:
						_byteInfo.BorderColor = Color.FromUInt32(ConfigManager.Config.Debug.Debugger.CodeExecBreakpointColor);
						break;
					case BreakpointTypeFlags.Write:
						_byteInfo.BorderColor = Color.FromUInt32(ConfigManager.Config.Debug.Debugger.CodeWriteBreakpointColor);
						break;
					case BreakpointTypeFlags.Read:
						_byteInfo.BorderColor = Color.FromUInt32(ConfigManager.Config.Debug.Debugger.CodeReadBreakpointColor);
						break;
				}
			}

			int framesToFade = _cfg.FadeSpeed.ToFrameCount();
			if(_cfg.ExecHighlight.Highlight && _counters[index].ExecStamp != 0 && framesSinceExec >= 0 && (framesSinceExec < framesToFade || framesToFade == 0)) {
				_byteInfo.ForeColor = DarkerColor(alpha, _cfg.ExecHighlight.Color, (framesToFade - framesSinceExec) / framesToFade);
			} else if(_cfg.WriteHighlight.Highlight && _counters[index].WriteStamp != 0 && framesSinceWrite >= 0 && (framesSinceWrite < framesToFade || framesToFade == 0)) {
				_byteInfo.ForeColor = DarkerColor(alpha, _cfg.WriteHighlight.Color, (framesToFade - framesSinceWrite) / framesToFade);
			} else if(_cfg.ReadHighlight.Highlight && _counters[index].ReadStamp != 0 && framesSinceRead >= 0 && (framesSinceRead < framesToFade || framesToFade == 0)) {
				_byteInfo.ForeColor = DarkerColor(alpha, _cfg.ReadHighlight.Color, (framesToFade - framesSinceRead) / framesToFade);
			} else {
				_byteInfo.ForeColor = Color.FromArgb(alpha, 0, 0, 0);
			}

			if(_frozenAddresses != null && _frozenAddresses[index] != 0) {
				_byteInfo.ForeColor = _cfg.FrozenHighlight.Color;
			}

			_byteInfo.Value = _data[index];

			return _byteInfo;
		}

		public byte GetRawByte(int byteIndex)
		{
			long index = byteIndex - _firstByteIndex;
			byte[] data = _data;
			if(index < 0 || index >= _data.Length) {
				//Byte is not available in the currently visible data (e.g user scrolled down/up after selecting the byte)
				//Get the byte from the core instead
				return DebugApi.GetMemoryValue(_memoryType, (uint)byteIndex);
			}

			return data[index];
		}

		public string ConvertValueToString(UInt64 val, out int keyLength)
		{
			if(_tblConverter != null) {
				return _tblConverter.ToChar(val, out keyLength);
			}

			keyLength = 1;
			val &= 0xFF;

			if(val < 32 || val >= 127) {
				return ".";
			}
			return ((char)val).ToString();
		}

		public byte ConvertCharToByte(char c)
		{
			if(_tblConverter != null) {
				return _tblConverter.GetBytes(c.ToString())[0];
			}

			if(c > 32 && c < 127) {
				return (byte)c;
			}
			return (byte)'.';
		}
	}
}
