using System;
using System.Drawing;
using System.Linq;
using Be.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Labels;

namespace Mesen.GUI.Debugger
{
	public class ByteColorProvider : IByteColorProvider
	{
		SnesMemoryType _memoryType;
		UInt64[] _readStamps;
		UInt64[] _writeStamps;
		UInt64[] _execStamps;
		UInt32[] _readCounts;
		UInt32[] _writeCounts;
		UInt32[] _execCounts;
		byte[] _cdlData;
		bool[] _hasLabel;
		DebugState _state = new DebugState();
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
		ByteColors _colors = new ByteColors();
		BreakpointTypeFlags[] _breakpointTypes;

		public ByteColorProvider(SnesMemoryType memoryType, bool showExec, bool showWrite, bool showRead, int framesToFade, bool hideUnusedBytes, bool hideReadBytes, bool hideWrittenBytes, bool hideExecutedBytes, bool highlightDataBytes, bool highlightCodeBytes, bool highlightLabelledBytes, bool highlightBreakpoints)
		{
			_memoryType = memoryType;
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

		public void Prepare(long firstByteIndex, long lastByteIndex)
		{
			int visibleByteCount = (int)(lastByteIndex - firstByteIndex + 1);

			if(_highlightBreakpoints) {
				Breakpoint[] breakpoints = BreakpointManager.Breakpoints.ToArray();
				_breakpointTypes = new BreakpointTypeFlags[visibleByteCount];

				for(int i = 0; i < visibleByteCount; i++) {
					int byteIndex = i + (int)firstByteIndex;
					foreach(Breakpoint bp in breakpoints) {
						if(bp.Enabled && bp.Matches((uint)byteIndex, _memoryType)) {
							_breakpointTypes[i] = bp.BreakOnExec ? BreakpointTypeFlags.Execute : (bp.BreakOnWrite ? BreakpointTypeFlags.Write : BreakpointTypeFlags.Read);
							break;
						}
					}
				}
			} else {
				_breakpointTypes = null;
			}

			_readStamps = DebugApi.GetMemoryAccessStamps((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType, MemoryOperationType.Read);
			_writeStamps = DebugApi.GetMemoryAccessStamps((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType, MemoryOperationType.Write);
			_execStamps = DebugApi.GetMemoryAccessStamps((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType, MemoryOperationType.ExecOpCode);

			_readCounts = DebugApi.GetMemoryAccessCounts((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType, MemoryOperationType.Read);
			_writeCounts = DebugApi.GetMemoryAccessCounts((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType, MemoryOperationType.Write);
			_execCounts = DebugApi.GetMemoryAccessCounts((UInt32)firstByteIndex, (UInt32)visibleByteCount, _memoryType, MemoryOperationType.ExecOpCode);

			_cdlData = null;
			if(_highlightDataBytes || _highlightCodeBytes) {
				switch(_memoryType) {
					case SnesMemoryType.CpuMemory:
					case SnesMemoryType.PrgRom:
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
						_hasLabel[i] = !string.IsNullOrWhiteSpace(LabelManager.GetLabel(addr)?.Label);
					}
				} else if(_memoryType == SnesMemoryType.PrgRom || _memoryType == SnesMemoryType.WorkRam || _memoryType == SnesMemoryType.SaveRam) {
					for(long i = 0; i < _hasLabel.Length; i++) {
						_hasLabel[i] = !string.IsNullOrWhiteSpace(LabelManager.GetLabel((uint)(firstByteIndex + i), _memoryType)?.Label);
					}
				}
			}

			_state = DebugApi.GetState();
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
			return Color.FromArgb((int)(input.R * brightnessPercentage), (int)(input.G * brightnessPercentage), (int)(input.B * brightnessPercentage));
		}

		public ByteColors GetByteColor(long firstByteIndex, long byteIndex)
		{
			HexEditorInfo cfg = ConfigManager.Config.Debug.HexEditor;

			const int CyclesPerFrame = 357368;
			long index = byteIndex - firstByteIndex;
			double framesSinceExec = (double)(_state.MasterClock - _execStamps[index]) / CyclesPerFrame;
			double framesSinceWrite = (double)(_state.MasterClock - _writeStamps[index]) / CyclesPerFrame;
			double framesSinceRead = (double)(_state.MasterClock - _readStamps[index]) / CyclesPerFrame;

			bool isRead = _readCounts[index] > 0;
			bool isWritten = _writeCounts[index] > 0;
			bool isExecuted = _execCounts[index] > 0;
			bool isUnused = !isRead && !isWritten && !isExecuted;

			int alpha = 0;
			if(isRead && _hideReadBytes || isWritten && _hideWrittenBytes || isExecuted && _hideExecutedBytes || isUnused && _hideUnusedBytes) {
				alpha = 128;
			}
			if(isRead && !_hideReadBytes || isWritten && !_hideWrittenBytes || isExecuted && !_hideExecutedBytes || isUnused && !_hideUnusedBytes) {
				alpha = 255;
			}

			_colors.BackColor = Color.Transparent;
			if(_cdlData != null) {
				if((_cdlData[index] & (byte)CdlFlags.Code) != 0 && _highlightCodeBytes) {
					//Code
					_colors.BackColor = cfg.CodeByteColor;
				} else if((_cdlData[index] & (byte)CdlFlags.Data) != 0 && _highlightDataBytes) {
					//Data
					_colors.BackColor = cfg.DataByteColor;
				}
			}

			if(_hasLabel[index]) {
				//Labels/comments
				_colors.BackColor = cfg.LabelledByteColor;
			}

			_colors.BorderColor = Color.Empty;
			if(_breakpointTypes != null) {
				switch(_breakpointTypes[index]) {
					case BreakpointTypeFlags.Execute:
						_colors.BorderColor = ConfigManager.Config.Debug.Debugger.CodeExecBreakpointColor;
						break;
					case BreakpointTypeFlags.Write:
						_colors.BorderColor = ConfigManager.Config.Debug.Debugger.CodeWriteBreakpointColor;
						break;
					case BreakpointTypeFlags.Read:
						_colors.BorderColor = ConfigManager.Config.Debug.Debugger.CodeReadBreakpointColor;
						break;
				}
			}

			if(_showExec && _execStamps[index] != 0 && framesSinceExec >= 0 && (framesSinceExec < _framesToFade || _framesToFade == 0)) {
				_colors.ForeColor = Color.FromArgb(alpha, DarkerColor(cfg.ExecColor, (_framesToFade - framesSinceExec) / _framesToFade));
			} else if(_showWrite && _writeStamps[index] != 0 && framesSinceWrite >= 0 && (framesSinceWrite < _framesToFade || _framesToFade == 0)) {
				_colors.ForeColor = Color.FromArgb(alpha, DarkerColor(cfg.WriteColor, (_framesToFade - framesSinceWrite) / _framesToFade));
			} else if(_showRead && _readStamps[index] != 0 && framesSinceRead >= 0 && (framesSinceRead < _framesToFade || _framesToFade == 0)) {
				_colors.ForeColor = Color.FromArgb(alpha, DarkerColor(cfg.ReadColor, (_framesToFade - framesSinceRead) / _framesToFade));
			} else {
				_colors.ForeColor = Color.FromArgb(alpha, Color.Black);
			}

			return _colors;
		}
	}
}
