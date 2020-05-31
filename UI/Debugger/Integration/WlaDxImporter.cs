using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Debugger.Workspace;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Integration
{
	public class WlaDxImporter : ISymbolProvider
	{
		private Dictionary<int, SourceFileInfo> _sourceFiles = new Dictionary<int, SourceFileInfo>();
		private Dictionary<string, AddressInfo> _addressByLine = new Dictionary<string, AddressInfo>();
		private Dictionary<string, SourceCodeLocation> _linesByAddress = new Dictionary<string, SourceCodeLocation>();

		public DateTime SymbolFileStamp { get; private set; }
		public string SymbolPath { get; private set; }

		public List<SourceFileInfo> SourceFiles { get { return _sourceFiles.Values.ToList(); } }

		public AddressInfo? GetLineAddress(SourceFileInfo file, int lineIndex)
		{
			AddressInfo address;
			if(_addressByLine.TryGetValue(file.Name.ToString() + "_" + lineIndex.ToString(), out address)) {
				return address;
			}
			return null;
		}

		public string GetSourceCodeLine(int prgRomAddress)
		{
			throw new NotImplementedException();
		}

		public SourceCodeLocation GetSourceCodeLineInfo(AddressInfo address)
		{
			string key = address.Type.ToString() + address.Address.ToString();
			SourceCodeLocation location;
			if(_linesByAddress.TryGetValue(key, out location)) {
				return location;
			}
			return null;
		}

		public SourceSymbol GetSymbol(string word, int prgStartAddress, int prgEndAddress)
		{
			return null;
		}

		public AddressInfo? GetSymbolAddressInfo(SourceSymbol symbol)
		{
			return null;
		}

		public SourceCodeLocation GetSymbolDefinition(SourceSymbol symbol)
		{
			return null;
		}

		public List<SourceSymbol> GetSymbols()
		{
			return new List<SourceSymbol>();
		}

		public int GetSymbolSize(SourceSymbol srcSymbol)
		{
			return 1;
		}

		public void Import(string path, bool silent)
		{
			string basePath = Path.GetDirectoryName(path);
			string[] lines = File.ReadAllLines(path);

			Regex labelRegex = new Regex(@"^([0-9a-fA-F]{2,4}):([0-9a-fA-F]{4}) ([^\s]*)", RegexOptions.Compiled);
			Regex fileRegex = new Regex(@"^([0-9a-fA-F]{4}) ([0-9a-fA-F]{8}) (.*)", RegexOptions.Compiled);
			Regex addrRegex = new Regex(@"^([0-9a-fA-F]{2,4}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", RegexOptions.Compiled);

			Dictionary<string, CodeLabel> labels = new Dictionary<string, CodeLabel>();

			bool isGameboy = EmuApi.GetRomInfo().CoprocessorType == CoprocessorType.Gameboy;

			for(int i = 0; i < lines.Length; i++) {
				string str = lines[i].Trim();
				if(str == "[labels]") {
					for(; i < lines.Length; i++) {
						if(lines[i].Length > 0) {
							Match m = labelRegex.Match(lines[i]);
							if(m.Success) {
								int bank = Int32.Parse(m.Groups[1].Value, System.Globalization.NumberStyles.HexNumber);
								string label = m.Groups[3].Value;
								label = label.Replace('.', '_').Replace(':', '_').Replace('$', '_');

								if(!LabelManager.LabelRegex.IsMatch(label)) {
									//ignore labels that don't respect the label naming restrictions
									continue;
								}

								AddressInfo absAddr;
								if(isGameboy) {
									int addr = Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
									if(addr >= 0x8000) {
										AddressInfo relAddr = new AddressInfo() { Address = addr, Type = SnesMemoryType.GameboyMemory };
										absAddr = DebugApi.GetAbsoluteAddress(relAddr);
									} else {
										absAddr = new AddressInfo() { Address = bank * 0x4000 + (addr & 0x3FFF), Type = SnesMemoryType.GbPrgRom };
									}
								} else {
									int addr = (bank << 16) | Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
									AddressInfo relAddr = new AddressInfo() { Address = addr, Type = SnesMemoryType.CpuMemory };
									absAddr = DebugApi.GetAbsoluteAddress(relAddr);
								}

								if(absAddr.Address < 0) {
									continue;
								}

								string orgLabel = label;
								int j = 1;
								while(labels.ContainsKey(label)) {
									label = orgLabel + j.ToString();
									j++;
								}

								labels[label] = new CodeLabel() {
									Label = label,
									Address = (UInt32)absAddr.Address,
									MemoryType = absAddr.Type,
									Comment = "",
									Flags = CodeLabelFlags.None,
									Length = 1
								};
							}
						} else {
							break;
						}
					}
				} else if(str == "[source files]") {
					for(; i < lines.Length; i++) {
						if(lines[i].Length > 0) {
							Match m = fileRegex.Match(lines[i]);
							if(m.Success) {
								int fileId = Int32.Parse(m.Groups[1].Value, System.Globalization.NumberStyles.HexNumber);
								//int fileCrc = Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
								string filePath = m.Groups[3].Value;

								string fullPath = Path.Combine(basePath, filePath);
								_sourceFiles[fileId] = new SourceFileInfo() {
									Name = filePath,
									Data = File.Exists(fullPath) ? File.ReadAllLines(fullPath) : new string[0]
								};
							}
						} else {
							break;
						}
					}
				} else if(str == "[addr-to-line mapping]") {
					for(; i < lines.Length; i++) {
						if(lines[i].Length > 0) {
							Match m = addrRegex.Match(lines[i]);
							if(m.Success) {
								int bank = Int32.Parse(m.Groups[1].Value, System.Globalization.NumberStyles.HexNumber);
								int addr = (bank << 16) | Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
								
								int fileId = Int32.Parse(m.Groups[3].Value, System.Globalization.NumberStyles.HexNumber);
								int lineNumber = Int32.Parse(m.Groups[4].Value, System.Globalization.NumberStyles.HexNumber);

								if(lineNumber <= 1) {
									//Ignore line number 0 and 1, seems like bad data?
									continue;
								}

								AddressInfo absAddr = new AddressInfo() { Address = addr, Type = SnesMemoryType.PrgRom };
								_addressByLine[_sourceFiles[fileId].Name + "_" + lineNumber.ToString()] = absAddr;
								_linesByAddress[absAddr.Type.ToString() + absAddr.Address.ToString()] = new SourceCodeLocation() { File = _sourceFiles[fileId], LineNumber = lineNumber };
							}
						} else {
							break;
						}
					}
				}
			}

			LabelManager.SetLabels(labels.Values, true);
		}
	}
}
