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
	private static Regex _labelRegex = new Regex(@"^([0-9a-fA-F]{2,4})[:]{0,1}([0-9a-fA-F]{4}) ([^\s]*)", RegexOptions.Compiled);
	private static Regex _fileRegex = new Regex(@"^([0-9a-fA-F]{4}) ([0-9a-fA-F]{8}) (.*)", RegexOptions.Compiled);
	private static Regex _addrRegex = new Regex(@"^([0-9a-fA-F]{2,4}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", RegexOptions.Compiled);
	private static Regex _fileV2Regex = new Regex(@"^([0-9a-fA-F]{4}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{8}) (.*)", RegexOptions.Compiled);
	private static Regex _addrV2Regex = new Regex(@"^([0-9a-fA-F]{8}) ([0-9a-fA-F]{2}):([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}) ([0-9a-fA-F]{4}):([0-9a-fA-F]{4}):([0-9a-fA-F]{8})", RegexOptions.Compiled);
	private static Regex _filePathRegex = new Regex(@"^(""([^;""]*)""\s*;{0,1}\s*(.*))|(.*)", RegexOptions.Compiled);

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

		byte[] cdlData = new byte[DebugApi.GetMemorySize(MemoryType.PcePrgRom)];

		int errorCount = 0;
		bool isAsar = false;

		//When [labels] tag isn't found, the symbols were output using the "nocash" symbol format (-s vs -S command line option)
		bool labelsOnly = !lines.Contains("[labels]");

		for(int i = 0; i < lines.Length; i++) {
			string str = lines[i].Trim();
			if(labelsOnly || str == "[labels]") {
				for(; i < lines.Length; i++) {
					str = lines[i].Trim();
					int commentStart = str.IndexOf(';');
					if(commentStart >= 0) {
						str = str.Substring(0, commentStart);
					}

					if(str.Length > 0) {
						Match m = _labelRegex.Match(str);
						if(m.Success) {
							int bank = Int32.Parse(m.Groups[1].Value, System.Globalization.NumberStyles.HexNumber);
							int addr = Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
							string originalLabel = m.Groups[3].Value;
							string label = LabelManager.InvalidLabelRegex.Replace(originalLabel, "_");

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

							SymbolInfo symbol = new(originalLabel, absAddr);
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
				int fileIndex = 1;
				int pathIndex = 3;
				Regex regex = _fileRegex;

				// Conversion of indices for supporting WLA-DX V2
				if(str == "[source files v2]") {
					fileIndex = 2;
					pathIndex = 4;
					regex = _fileV2Regex;
				}

				for(; i < lines.Length; i++) {
					if(lines[i].Length > 0) {
						Match m = regex.Match(lines[i]);
						if(m.Success) {
							int fileId = Int32.Parse(m.Groups[fileIndex].Value, System.Globalization.NumberStyles.HexNumber);
							//int fileCrc = Int32.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);

							Match fileMatch = _filePathRegex.Match(m.Groups[pathIndex].Value);
							if(fileMatch.Success) {
								string filePath = fileMatch.Groups[2].Success ? fileMatch.Groups[2].Value : fileMatch.Groups[4].Value;
								string comment = fileMatch.Groups[3].Value;
								string fullPath;
								if(Path.IsPathFullyQualified(filePath)) {
									fullPath = filePath;
								} else {
									string? srcBasePath = basePath;
									fullPath = Path.Combine(srcBasePath, filePath);
									while(!File.Exists(fullPath)) {
										//Go back up folder structure to attempt to find the file
										string oldPath = srcBasePath;
										srcBasePath = Path.GetDirectoryName(srcBasePath);
										if(srcBasePath == null || srcBasePath == oldPath) {
											break;
										}
										fullPath = Path.Combine(srcBasePath, filePath);
									}
								}
								string[] fileData = File.Exists(fullPath) ? File.ReadAllLines(fullPath) : Array.Empty<string>();
								_sourceFiles[fileId] = new SourceFileInfo(filePath, true, new WlaDxFile() { Data = fileData });
							}
						}
					} else {
						break;
					}
				}
			} else if(str == "[addr-to-line mapping]" || str == "[addr-to-line mapping v2]") {
				int bankIndex = 1;
				int addrIndex = 2;
				int fieldIndex = 3;
				int lineIndex = 4;
				Regex regex = _addrRegex;

				// Conversion of indices for supporting WLA-DX V2
				if(str == "[addr-to-line mapping v2]") {
					bankIndex = 2;
					addrIndex = 3;
					fieldIndex = 6;
					lineIndex = 7;
					regex = _addrV2Regex;
				}

				for(; i < lines.Length; i++) {
					if(lines[i].Length > 0) {
						Match m = regex.Match(lines[i]);
						if(m.Success) {
							int bank = Int32.Parse(m.Groups[bankIndex].Value, System.Globalization.NumberStyles.HexNumber);
							int addr = (bank << 16) | Int32.Parse(m.Groups[addrIndex].Value, System.Globalization.NumberStyles.HexNumber);

							int fileId = Int32.Parse(m.Groups[fieldIndex].Value, System.Globalization.NumberStyles.HexNumber);
							int lineNumber = Int32.Parse(m.Groups[lineIndex].Value, System.Globalization.NumberStyles.HexNumber);
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
	private RomFormat _format;

	public PceWlaDxImporter()
	{
		_format = EmuApi.GetRomInfo().Format;
	}

	protected override AddressInfo GetLabelAddress(int bank, int addr)
	{
		if(bank == 0xF8 && bank <= 0xFB) {
			return new AddressInfo() {
				Address = (bank - 0xF8) * 0x2000 + (addr & 0x1FFF),
				Type = MemoryType.PceWorkRam
			};
		} else if(bank == 0xF7) {
			return new AddressInfo() {
				Address = (addr & 0x1FFF),
				Type = MemoryType.PceSaveRam
			};
		} else if(bank == 0xFF) {
			return new AddressInfo() {
				Address = (addr & 0x1FFF),
				Type = MemoryType.PceMemory
			};
		} else if(_format == RomFormat.PceCdRom && (bank >= 0x68 && bank <= 0x7F)) {
			return new AddressInfo() {
				Address = (bank - 0x68) * 0x2000 + (addr & 0x1FFF),
				Type = MemoryType.PceCardRam
			};	
		} else if(_format == RomFormat.PceCdRom && (bank >= 0x80 && bank <= 0x87)) {
			return new AddressInfo() {
				Address = (bank - 0x80) * 0x2000 + (addr & 0x1FFF),
				Type = MemoryType.PceCdromRam
			};
		} else if(bank > 0xFF) {
			return new AddressInfo() {
				Address = (bank - 0x80) * 0x2000 + (addr & 0x1FFF),
				Type = MemoryType.PcePrgRom
			};
		} else if(bank < 0x80) {
			return new AddressInfo() {
				Address = bank * 0x2000 + (addr & 0x1FFF),
				Type = MemoryType.PcePrgRom
			};
		}
		return default;
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
