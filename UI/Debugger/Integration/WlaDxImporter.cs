using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;

namespace Mesen.Debugger.Integration;

public abstract class WlaDxImporter : ISymbolProvider
{
	private static Regex _labelRegex = new Regex(@"^([0-9a-fA-F]{2,4}):([0-9a-fA-F]{4}) ([^\s]*)", RegexOptions.Compiled);
	private static Regex _fileRegex = new Regex(@"^([0-9a-fA-F]{4}) ([0-9a-fA-F]{8}) (.*)", RegexOptions.Compiled);
	private static Regex _addrRegex = new Regex(@"^([0-9a-fA-F]{2,4}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", RegexOptions.Compiled);
	private static Regex _fileV2Regex = new Regex(@"^([0-9a-fA-F]{4}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{8}) (.*)", RegexOptions.Compiled);
	private static Regex _addrV2Regex = new Regex(@"^([0-9a-fA-F]{8}) ([0-9a-fA-F]{2}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}):([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", RegexOptions.Compiled);

	private Dictionary<int, SourceFileInfo> _sourceFiles = new();
	private Dictionary<string, AddressInfo> _addressByLine = new();
	private Dictionary<string, SourceCodeLocation> _linesByAddress = new();
	private List<SymbolInfo> _symbols = new();

	public DateTime SymbolFileStamp { get; private set; }
	public string SymbolPath { get; private set; } = "";

	public List<SourceFileInfo> SourceFiles { get { return _sourceFiles.Values.ToList(); } }

	public AddressInfo? GetLineAddress(SourceFileInfo file, int lineIndex)
	{
		AddressInfo address;
		if(_addressByLine.TryGetValue(file.Name.ToString() + "_" + lineIndex.ToString(), out address)) {
			return address;
		}
		return null;
	}

	public AddressInfo? GetLineEndAddress(SourceFileInfo file, int lineIndex)
	{
		return null;
	}

	public string GetSourceCodeLine(int prgRomAddress)
	{
		throw new NotImplementedException();
	}

	public SourceCodeLocation? GetSourceCodeLineInfo(AddressInfo address)
	{
		string key = address.Type.ToString() + address.Address.ToString();
		SourceCodeLocation location;
		if(_linesByAddress.TryGetValue(key, out location)) {
			return location;
		}
		return null;
	}

	public SourceSymbol? GetSymbol(string word, int scopeStart, int scopeEnd)
	{
		foreach(SymbolInfo symbol in _symbols) {
			if(symbol.Name == word) {
				//TODOv2 SCOPE
				return symbol.SourceSymbol;
			}
		}
		return null;
	}

	public AddressInfo? GetSymbolAddressInfo(SourceSymbol symbol)
	{
		if(symbol.InternalSymbol is SymbolInfo dbgSymbol) {
			return dbgSymbol.Address;
		}
		return null;
	}

	public SourceCodeLocation? GetSymbolDefinition(SourceSymbol symbol)
	{
		return null;
	}

	public List<SourceSymbol> GetSymbols()
	{
		return _symbols.Select(s => s.SourceSymbol).ToList();
	}

	public int GetSymbolSize(SourceSymbol srcSymbol)
	{
		return 1;
	}

	public void Import(string path, bool showResult)
	{
		SymbolFileStamp = File.GetLastWriteTime(path);

		string basePath = Path.GetDirectoryName(path) ?? "";
		SymbolPath = basePath;

		string[] lines = File.ReadAllLines(path);

		Dictionary<string, CodeLabel> labels = new Dictionary<string, CodeLabel>();

		int errorCount = 0;
		bool isAsar = false;

		//When [labels] tag isn't found, the symbols were output using the "nocash" symbol format (-s vs -S command line option)
		bool labelsOnly = !lines.Contains("[labels]");

		for(int i = 0; i < lines.Length; i++) {
			string str = lines[i].Trim();
			if(labelsOnly || str == "[labels]") {
				for(; i < lines.Length; i++) {
					if(lines[i].Length > 0) {
						Match m = _labelRegex.Match(lines[i]);
						if(m.Success) {
							int bank = Int32.Parse(m.Groups[1].Value, System.Globalization.NumberStyles.HexNumber);
							int addr = Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
							string label = m.Groups[3].Value;
							label = label.Replace('.', '_').Replace(':', '_').Replace('$', '_');

							if(!LabelManager.LabelRegex.IsMatch(label)) {
								//ignore labels that don't respect the label naming restrictions
								errorCount++;
								continue;
							}

							AddressInfo absAddr = GetLabelAddress(bank, addr);

							if(absAddr.Address < 0) {
								errorCount++;
								continue;
							}

							SymbolInfo symbol = new(label, absAddr);
							_symbols.Add(symbol);

							string orgLabel = label;
							int j = 1;
							while(labels.ContainsKey(label)) {
								label = orgLabel + j.ToString();
								j++;
							}

							if(ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(absAddr.Type)) {
								labels[label] = new CodeLabel() {
									Label = label,
									Address = (UInt32)absAddr.Address,
									MemoryType = absAddr.Type,
									Comment = "",
									Flags = CodeLabelFlags.None,
									Length = 1
								};
							}
						}
					} else {
						break;
					}
				}
			} else if(str == "[source files]" || str == "[source files v2]") {
				int file_idx = 1;
				int path_idx = 3;
				Regex regex = _fileRegex;

				// Conversion of indices for supporting WLA-DX V2
				if(str == "[source files v2]") {
					file_idx = 2;
					path_idx = 4;
					regex = _fileV2Regex;
				}

				for(; i < lines.Length; i++) {
					if(lines[i].Length > 0) {
						Match m = regex.Match(lines[i]);
						if(m.Success) {
							int fileId = Int32.Parse(m.Groups[file_idx].Value, System.Globalization.NumberStyles.HexNumber);
							//int fileCrc = Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
							string filePath = m.Groups[path_idx].Value;

							string fullPath = Path.Combine(basePath, filePath);
							_sourceFiles[fileId] = new SourceFileInfo(filePath, true, new WlaDxFile() { Data = File.Exists(fullPath) ? File.ReadAllLines(fullPath) : new string[0] });
						}
					} else {
						break;
					}
				}
			} else if(str == "[addr-to-line mapping]" || str == "[addr-to-line mapping v2]") {
				int bank_idx = 1;
				int addr_idx = 2;
				int field_idx = 3;
				int line_idx = 4;
				Regex regex = _addrRegex;

				// Conversion of indices for supporting WLA-DX V2
				if(str == "[addr-to-line mapping v2]") {
					bank_idx = 2;
					addr_idx = 3;
					field_idx = 6;
					line_idx = 7;
					regex = _addrV2Regex;
				}

				for(; i < lines.Length; i++) {
					if(lines[i].Length > 0) {
						Match m = regex.Match(lines[i]);
						if(m.Success) {
							int bank = Int32.Parse(m.Groups[bank_idx].Value, System.Globalization.NumberStyles.HexNumber);
							int addr = (bank << 16) | Int32.Parse(m.Groups[addr_idx].Value, System.Globalization.NumberStyles.HexNumber);

							int fileId = Int32.Parse(m.Groups[field_idx].Value, System.Globalization.NumberStyles.HexNumber);
							int lineNumber = Int32.Parse(m.Groups[line_idx].Value, System.Globalization.NumberStyles.HexNumber);
							if(isAsar) {
								lineNumber--;
							}

							if(!isAsar && fileId == 0) {
								//WLA-DX can generate invalid file mappings if a file is optimized away. Ignore these mappings.
								errorCount++;
								continue;
							}

							if(lineNumber <= 1) {
								//Ignore line number 0 and 1, seems like bad data?
								errorCount++;
								continue;
							}

							AddressInfo absAddr = GetLabelAddress(bank, addr);
							if(absAddr.Address >= 0) {
								_addressByLine[_sourceFiles[fileId].Name + "_" + lineNumber.ToString()] = absAddr;
								_linesByAddress[absAddr.Type.ToString() + absAddr.Address.ToString()] = new SourceCodeLocation(_sourceFiles[fileId], lineNumber);
							}
						}
					} else {
						break;
					}
				}
			} else if(str == "; generated by asar") {
				isAsar = true;
			}
		}

		LabelManager.SetLabels(labels.Values, true);

		if(showResult) {
			if(errorCount > 0) {
				MesenMsgBox.Show(null, "ImportLabelsWithErrors", MessageBoxButtons.OK, MessageBoxIcon.Warning, labels.Count.ToString(), errorCount.ToString());
			} else {
				MesenMsgBox.Show(null, "ImportLabels", MessageBoxButtons.OK, MessageBoxIcon.Info, labels.Count.ToString());
			}
		}
	}

	protected abstract AddressInfo GetLabelAddress(int bank, int addr);

	class WlaDxFile : IFileDataProvider
	{
		public string[] Data { get; init; } = Array.Empty<string>();
	}

	private readonly struct SymbolInfo
	{
		public string Name { get; }
		public AddressInfo Address { get; }
		public SourceSymbol SourceSymbol { get => new SourceSymbol(Name, Address.Address, this); }

		public SymbolInfo(string name, AddressInfo address)
		{
			Name = name;
			Address = address;
		}
	}
}

public class SnesWlaDxImporter : WlaDxImporter
{
	protected override AddressInfo GetLabelAddress(int bank, int addr)
	{
		AddressInfo relAddr = new AddressInfo() { Address = (bank << 16) | addr, Type = MemoryType.SnesMemory };
		return DebugApi.GetAbsoluteAddress(relAddr);
	}
}

public class GbWlaDxImporter : WlaDxImporter
{
	protected override AddressInfo GetLabelAddress(int bank, int addr)
	{
		if(addr >= 0x8000) {
			AddressInfo relAddr = new AddressInfo() { Address = addr, Type = MemoryType.GameboyMemory };
			return DebugApi.GetAbsoluteAddress(relAddr);
		} else {
			return new AddressInfo() { Address = bank * 0x4000 + (addr & 0x3FFF), Type = MemoryType.GbPrgRom };
		}
	}
}

public class PceWlaDxImporter : WlaDxImporter
{
	protected override AddressInfo GetLabelAddress(int bank, int addr)
	{
		//TODOv2 RAM labels seem to be missing from .sym file?
		return new AddressInfo() { Address = bank * 0x2000 + (addr & 0x1FFF), Type = MemoryType.PcePrgRom };
	}
}

public class SmsWlaDxImporter : WlaDxImporter
{
	protected override AddressInfo GetLabelAddress(int bank, int addr)
	{
		if(addr >= 0xC000) {
			return new AddressInfo() { Address = addr - 0xC000, Type = MemoryType.SmsWorkRam };
		} else {
			return new AddressInfo() { Address = bank * 0x4000 + (addr & 0x3FFF), Type = MemoryType.SmsPrgRom };
		}
	}
}
