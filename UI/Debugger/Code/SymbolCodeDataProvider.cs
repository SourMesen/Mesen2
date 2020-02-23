using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class SymbolCodeDataProvider : ICodeDataProvider
	{
		//private byte[] _prgRom;
		private int _lineCount;
		private SourceFileInfo _file;
		private CpuType _type;
		private bool _isC;
		private ISymbolProvider _symbolProvider;

		public SymbolCodeDataProvider(CpuType type, ISymbolProvider symbolProvider, SourceFileInfo file)
		{
			//_prgRom = DebugApi.GetMemoryState(SnesMemoryType.PrgRom);
			_type = type;
			_symbolProvider = symbolProvider;
			_file = file;
			_lineCount = file.Data.Length;

			string filename = file.Name.ToLower();
			_isC = filename.EndsWith(".h") || filename.EndsWith(".c");
		}

		public CodeLineData GetCodeLineData(int lineIndex)
		{
			AddressInfo? address = _symbolProvider.GetLineAddress(_file, lineIndex);

			CodeLineData data = new CodeLineData(_type) {
				Address = GetLineAddress(lineIndex),
				AbsoluteAddress = address.HasValue ? address.Value.Address : -1,
				EffectiveAddress = -1,
				Flags = LineFlags.VerifiedCode
			};

			//TODO
			/*if(prgAddress >= 0) {
				int opSize = DebugApi.GetDisassemblyOpSize(_prgRom[prgAddress]);
				string byteCode = "";
				for(int i = prgAddress, end = prgAddress + opSize; i < end && i < _prgRom.Length; i++) {
					byteCode += "$" + _prgRom[i].ToString("X2") + " ";
				}
				data.ByteCode = byteCode;
			}*/

			string text = _file.Data[lineIndex];
			string trimmed = text.TrimStart();
			data.CustomIndent = (text.Length - trimmed.Length) * 10;

			int commentIndex;
			if(_isC) {
				commentIndex = trimmed.IndexOf("//");
			} else {
				commentIndex = trimmed.IndexOfAny(new char[] { ';', '.' });
			}

			if(commentIndex >= 0) {
				data.Comment = trimmed.Substring(commentIndex);
				data.Text = trimmed.Substring(0, commentIndex).TrimEnd();
			} else {
				data.Comment = "";
				data.Text = trimmed;
			}

			return data;
		}

		public int GetLineAddress(int lineIndex)
		{
			AddressInfo? absAddress = _symbolProvider.GetLineAddress(_file, lineIndex);
			if(absAddress != null) {
				return DebugApi.GetRelativeAddress(absAddress.Value).Address;
			} else {
				return -1;
			}
		}

		public int GetLineCount()
		{
			return _lineCount;
		}

		public int GetLineIndex(uint cpuAddress)
		{
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = (int)cpuAddress, Type = SnesMemoryType.CpuMemory });
			for(int i = 0; i < _lineCount; i++) {
				AddressInfo? lineAddr = _symbolProvider.GetLineAddress(_file, i);
				if(lineAddr != null && lineAddr.Value.Address == absAddress.Address && lineAddr.Value.Type == absAddress.Type) {
					return i;
				}
			}
			return 0;
		}

		public bool UseOptimizedSearch { get { return false; } }

		public int GetNextResult(string searchString, int startPosition, int endPosition, bool searchBackwards)
		{
			throw new NotImplementedException();
		}
	}
}
