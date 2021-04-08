using System;
using System.Linq;
using Avalonia.Media;
using Mesen.Debugger.Controls;
using Mesen.GUI;
using Mesen.GUI.Config;
using Mesen.GUI.Debugger;

namespace Mesen.Debugger
{
	public class HexEditorDataProvider : IHexEditorDataProvider
	{
		SnesMemoryType _memoryType;
		AddressCounters[] _counters;
		byte[] _cdlData;
		bool[] _hasLabel;
		UInt64 _masterClock;
		bool _showExec;
		bool _showWrite;
		bool _showRead;
		int _framesToFade;
		bool _hideUnusedBytes;
		bool _hideReadBytes;
		bool _hideWrittenBytes;
		bool _hideExecutedBytes;
		bool _highlightDataBytes;
		bool _highlightCodeBytes;
		bool _highlightLabelledBytes;
		bool _highlightBreakpoints;
		ByteInfo _byteInfo = new ByteInfo();
		BreakpointTypeFlags[] _breakpointTypes;
		byte[] _data = new byte[0];

		private long _firstByteIndex = 0;

		public HexEditorDataProvider(SnesMemoryType memoryType, bool showExec, bool showWrite, bool showRead, int framesToFade, bool hideUnusedBytes, bool hideReadBytes, bool hideWrittenBytes, bool hideExecutedBytes, bool highlightDataBytes, bool highlightCodeBytes, bool highlightLabelledBytes, bool highlightBreakpoints)
		{
			_memoryType = memoryType;
			Length = DebugApi.GetMemorySize(memoryType);
			_showExec = showExec;
			_showWrite = showWrite;
			_showRead = showRead;
			_framesToFade = framesToFade;
			_hideUnusedBytes = hideUnusedBytes;
			_hideReadBytes = hideReadBytes;
			_hideWrittenBytes = hideWrittenBytes;
			_hideExecutedBytes = hideExecutedBytes;
			_highlightDataBytes = highlightDataBytes;
			_highlightCodeBytes = highlightCodeBytes;
			_highlightLabelledBytes = highlightLabelledBytes;
			_highlightBreakpoints = highlightBreakpoints;
		}

		public int Length { get; private set; }

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

			if(_highlightBreakpoints) {
				//TODO
				/*Breakpoint[] breakpoints = BreakpointManager.Breakpoints.ToArray();
				_breakpointTypes = new BreakpointTypeFlags[visibleByteCount];

				for(int i = 0; i < visibleByteCount; i++) {
					int byteIndex = i + (int)firstByteIndex;
					foreach(Breakpoint bp in breakpoints) {
						if(bp.Enabled && bp.Matches((uint)byteIndex, _memoryType, null)) {
							_breakpointTypes[i] = bp.BreakOnExec ? BreakpointTypeFlags.Execute : (bp.BreakOnWrite ? BreakpointTypeFlags.Write : BreakpointTypeFlags.Read);
							break;
						}
					}
				}*/
			} else {
				_breakpointTypes = null;
			}

			_counters = DebugApi.GetMemoryAccessCounts((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType);

			_cdlData = null;
			if(_highlightDataBytes || _highlightCodeBytes) {
				switch(_memoryType) {
					case SnesMemoryType.CpuMemory:
					case SnesMemoryType.Sa1Memory:
					case SnesMemoryType.Cx4Memory:
					case SnesMemoryType.GsuMemory:
					case SnesMemoryType.GameboyMemory:
					case SnesMemoryType.PrgRom:
					case SnesMemoryType.GbPrgRom:
						_cdlData = DebugApi.GetCdlData((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType);
						break;
				}
			}

			_hasLabel = new bool[visibleByteCount];
			if(_highlightLabelledBytes) {
				if(_memoryType <= SnesMemoryType.SpcMemory) {
					AddressInfo addr = new AddressInfo();
					addr.Type = _memoryType;
					for(long i = 0; i < _hasLabel.Length; i++) {
						addr.Address = (int)(firstByteIndex + i);
						//TODO
						//_hasLabel[i] = !string.IsNullOrWhiteSpace(LabelManager.GetLabel(addr)?.Label);
					}
				} else if(_memoryType == SnesMemoryType.PrgRom || _memoryType == SnesMemoryType.WorkRam || _memoryType == SnesMemoryType.SaveRam) {
					for(long i = 0; i < _hasLabel.Length; i++) {
						//TODO
						//_hasLabel[i] = !string.IsNullOrWhiteSpace(LabelManager.GetLabel((uint)(firstByteIndex + i), _memoryType)?.Label);
					}
				}
			}

			//TODO
			_masterClock = 0;
		}

		public static Color DarkerColor(Color input, double brightnessPercentage)
		{
			if(double.IsInfinity(brightnessPercentage)) {
				brightnessPercentage = 1.0;
			}
			if(brightnessPercentage < 0.20) {
				brightnessPercentage *= 5;
			} else {
				brightnessPercentage = 1.0;
			}
			return Color.FromRgb((byte)(input.R * brightnessPercentage), (byte)(input.G * brightnessPercentage), (byte)(input.B * brightnessPercentage));
		}

		public ByteInfo GetByte(int byteIndex)
		{
			HexEditorConfig cfg = ConfigManager.Config.Debug.HexEditor;

			const int CyclesPerFrame = 357368;
			long index = byteIndex - _firstByteIndex;
			double framesSinceExec = (double)(_masterClock - _counters[index].ExecStamp) / CyclesPerFrame;
			double framesSinceWrite = (double)(_masterClock - _counters[index].WriteStamp) / CyclesPerFrame;
			double framesSinceRead = (double)(_masterClock - _counters[index].ReadStamp) / CyclesPerFrame;

			bool isRead = _counters[index].ReadCount > 0;
			bool isWritten = _counters[index].WriteCount > 0;
			bool isExecuted = _counters[index].ExecCount > 0;
			bool isUnused = !isRead && !isWritten && !isExecuted;

			byte alpha = 0;
			if(isRead && _hideReadBytes || isWritten && _hideWrittenBytes || isExecuted && _hideExecutedBytes || isUnused && _hideUnusedBytes) {
				alpha = 128;
			}
			if(isRead && !_hideReadBytes || isWritten && !_hideWrittenBytes || isExecuted && !_hideExecutedBytes || isUnused && !_hideUnusedBytes) {
				alpha = 255;
			}

			_byteInfo.BackColor = Colors.Transparent;
			if(_cdlData != null) {
				if((_cdlData[index] & (byte)CdlFlags.Code) != 0 && _highlightCodeBytes) {
					//Code
					_byteInfo.BackColor = cfg.CodeByteColor;
				} else if((_cdlData[index] & (byte)CdlFlags.Data) != 0 && _highlightDataBytes) {
					//Data
					_byteInfo.BackColor = cfg.DataByteColor;
				}
			}

			if(_hasLabel[index]) {
				//Labels/comments
				_byteInfo.BackColor = cfg.LabelledByteColor;
			}

			_byteInfo.BorderColor = Colors.Transparent;
			if(_breakpointTypes != null) {
				switch(_breakpointTypes[index]) {
					case BreakpointTypeFlags.Execute:
						_byteInfo.BorderColor = ConfigManager.Config.Debug.Debugger.CodeExecBreakpointColor;
						break;
					case BreakpointTypeFlags.Write:
						_byteInfo.BorderColor = ConfigManager.Config.Debug.Debugger.CodeWriteBreakpointColor;
						break;
					case BreakpointTypeFlags.Read:
						_byteInfo.BorderColor = ConfigManager.Config.Debug.Debugger.CodeReadBreakpointColor;
						break;
				}
			}

			if(_showExec && _counters[index].ExecStamp != 0 && framesSinceExec >= 0 && (framesSinceExec < _framesToFade || _framesToFade == 0)) {
				_byteInfo.ForeColor = cfg.ExecColor; //TODO Color.FromArgb(alpha, DarkerColor(cfg.ExecColor, (_framesToFade - framesSinceExec) / _framesToFade));
			} else if(_showWrite && _counters[index].WriteStamp != 0 && framesSinceWrite >= 0 && (framesSinceWrite < _framesToFade || _framesToFade == 0)) {
				_byteInfo.ForeColor = cfg.WriteColor; //TODO Color.FromArgb(alpha, DarkerColor(cfg.WriteColor, (_framesToFade - framesSinceWrite) / _framesToFade));
			} else if(_showRead && _counters[index].ReadStamp != 0 && framesSinceRead >= 0 && (framesSinceRead < _framesToFade || _framesToFade == 0)) {
				_byteInfo.ForeColor = cfg.ReadColor; //TODO Color.FromArgb(alpha, DarkerColor(cfg.ReadColor, (_framesToFade - framesSinceRead) / _framesToFade));
			} else {
				_byteInfo.ForeColor = Color.FromArgb(alpha, 0, 0, 0);
			}

			_byteInfo.Value = _data[index];

			return _byteInfo;
		}
	}
}
