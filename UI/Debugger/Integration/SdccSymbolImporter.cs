using Mesen.Config;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;

namespace Mesen.Debugger.Integration;

public abstract class SdccSymbolImporter : ISymbolProvider
{
	private static Regex _globalSymbolRegex = new Regex(@"^L:G\$([^$]+)\$([0-9_]+)\$(\d+):([0-9A-Fa-f]+)$", RegexOptions.Compiled);
	private static Regex _srcMappingRegex = new Regex(@"^L:C\$([^$]+)\$(\d+)\$([0-9_]+)\$(\d+):([0-9A-Fa-f]+)$", RegexOptions.Compiled);
	private static Regex _symRegex = new Regex(@"^([0-9a-fA-F]{2,4})[:]{0,1}([0-9a-fA-F]{4}) ([^\s]*)", RegexOptions.Compiled);

	private Dictionary<string, SourceFileInfo> _sourceFiles = new();
	private Dictionary<string, AddressInfo> _addressByLine = new();
	private Dictionary<string, SourceCodeLocation> _linesByAddress = new();
	private Dictionary<string, SymbolInfo> _symbols = new();

	public DateTime SymbolFileStamp { get; private set; }
	public string SymbolPath { get; private set; } = "";

	public List<SourceFileInfo> SourceFiles { get { return _sourceFiles.Values.ToList(); } }

	protected SdccSymbolImporter()
	{
	}

	public static SdccSymbolImporter? Import(RomFormat romFormat, string path, bool showResult)
	{
		SdccSymbolImporter? importer = romFormat switch {
			RomFormat.iNes or RomFormat.Nsf or RomFormat.VsSystem or RomFormat.VsDualSystem => new NesSdccSymbolImporter(),
			RomFormat.Gb => new GbSdccSymbolImporter(),
			RomFormat.Sms or RomFormat.GameGear => new SmsSdccSymbolImporter(),
			_ => null
		};

		if(importer == null) {
			EmuApi.WriteLogEntry("[Debugger] Error: Attempted to load .cdb file for an unsupported file format: " + romFormat);
		}

		importer?.Import(path, showResult);

		return importer;
	}

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
		foreach(SymbolInfo symbol in _symbols.Values) {
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
		return _symbols.Values.Select(s => s.SourceSymbol).ToList();
	}

	public int GetSymbolSize(SourceSymbol srcSymbol)
	{
		return 1;
	}

	protected abstract bool TryDecodeAddress(int addr, out AddressInfo absAddr);

	private string? FindInFolder(string basePath, string folder, string filename)
	{
		if(!Directory.Exists(Path.Combine(basePath, folder))) {
			return null;
		}

		string ? foundPath = Directory.EnumerateFiles(Path.Combine(basePath, folder), filename, SearchOption.AllDirectories).FirstOrDefault();
		if(File.Exists(foundPath)) {
			return foundPath;
		}
		return null;
	}

	private bool FindFile(string path, string filename, [MaybeNullWhen(false)] out string result)
	{
		string? searchPath = Path.GetDirectoryName(path) ?? "";
		string sourceFile = Path.Combine(searchPath, filename);
		result = null;

		while(!File.Exists(sourceFile)) {
			//Go back up folder structure to attempt to find the file
			result = FindInFolder(searchPath, "src", filename) ?? FindInFolder(searchPath, "include", filename);
			if(result != null) {
				return true;
			}

			string oldPath = searchPath;
			searchPath = Path.GetDirectoryName(searchPath);
			if(searchPath == null || searchPath == oldPath) {
				return false;
			}
			sourceFile = Path.Combine(searchPath, filename);
		}

		return false;
	}

	public void Import(string path, bool showResult)
	{
		SymbolFileStamp = File.GetLastWriteTime(path);

		string basePath = Path.GetDirectoryName(path) ?? "";
		SymbolPath = basePath;

		string[] lines = File.ReadAllLines(path);

		int errorCount = 0;

		string prevFilename = "";
		int prevLineAddr = -1;
		SourceCodeLocation prevLoc = new();

		Dictionary<string, CodeLabel> labels = new();

		for(int i = 0; i < lines.Length; i++) {
			string str = lines[i].Trim();
			Match m;
			if((m = _srcMappingRegex.Match(str)).Success) {
				string filename = m.Groups[1].Value;
				SourceFileInfo? srcFile;
				if(!_sourceFiles.TryGetValue(filename, out srcFile)) {
					if(!FindFile(path, filename, out string? sourceFile)) {
						continue;
					}

					string[] data = File.ReadAllLines(sourceFile);
					srcFile = new SourceFileInfo(filename, false, new SdccSourceFile() { Data = data });
					_sourceFiles[filename] = srcFile;
				}

				int lineNumber = int.Parse(m.Groups[2].Value) - 1;
				int baseAddr = int.Parse(m.Groups[5].Value, System.Globalization.NumberStyles.HexNumber);

				if(!TryDecodeAddress(baseAddr, out AddressInfo absAddr)) {
					continue;
				}

				int length = filename == prevFilename && prevLineAddr >= 0 && prevLineAddr < absAddr.Address ? absAddr.Address - prevLineAddr : 1;

				for(int j = 0; j < length; j++) {
					_linesByAddress[absAddr.Type.ToString() + (prevLineAddr + j).ToString()] = prevLoc;
				}

				_addressByLine[filename + "_" + lineNumber] = absAddr;

				prevLineAddr = absAddr.Address;
				prevFilename = filename;

				SourceCodeLocation loc = new SourceCodeLocation(srcFile, lineNumber);
				prevLoc = loc;
				_linesByAddress[absAddr.Type.ToString() + absAddr.Address.ToString()] = loc;
			} else if((m = _globalSymbolRegex.Match(str)).Success) {
				string name = m.Groups[1].Value;
				int baseAddr = int.Parse(m.Groups[4].Value, System.Globalization.NumberStyles.HexNumber);

				if(!TryDecodeAddress(baseAddr, out AddressInfo absAddr)) {
					continue;
				}

				string label = LabelManager.InvalidLabelRegex.Replace(name, "_");
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

				_symbols[name] = new SymbolInfo(name, absAddr);
			}
		}

		string symPath = Path.ChangeExtension(path, ".sym");
		if(File.Exists(symPath)) {
			//Also load the no$ format symbol file, too (some symbols are missing from the .cdb files)
			LoadSymFile(symPath, labels);
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

	private record MemAddress(int addr, MemoryType type);

	private void LoadSymFile(string path, Dictionary<string, CodeLabel> labels)
	{
		string[] lines = File.ReadAllLines(path);

		HashSet<MemAddress> existingLabels = labels.Values.Select(l => new MemAddress((int)l.Address, l.MemoryType)).ToHashSet();
		HashSet<MemAddress> existingSymbols = _symbols.Values.Where(l => l.Address != null).Select(l => new MemAddress(l.Address!.Value.Address, l.Address.Value.Type)).ToHashSet();

		for(int i = 0; i < lines.Length; i++) {
			string str = lines[i].Trim();
			Match m;
			if((m = _symRegex.Match(str)).Success) {
				string name = m.Groups[3].Value;
				if(name.StartsWith("C$") || name.StartsWith("A$") || name.StartsWith("s_") || name.StartsWith("l_") || name.Contains("$sloc") || name.StartsWith("XG$")) {
					continue;
				}

				if(name.StartsWith("G$")) {
					string[] nameParts = name.Split('$');
					if(nameParts.Length > 1) {
						name = nameParts[1];
					} else {
						continue;
					}
				} else {
					int index = name.IndexOf("$");
					if(index > 0) {
						name = name.Substring(0, index);
					}
				}

				int bank = int.Parse(m.Groups[1].Value, System.Globalization.NumberStyles.HexNumber);
				int addr = int.Parse(m.Groups[2].Value, System.Globalization.NumberStyles.HexNumber);
				
				if(!TryDecodeAddress(addr, out AddressInfo absAddr)) {
					continue;
				}

				MemAddress memAddr = new MemAddress(absAddr.Address, absAddr.Type);
				string baseLabel = LabelManager.InvalidLabelRegex.Replace(name, "_");

				if(!existingLabels.Contains(memAddr) && ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(absAddr.Type)) {
					string label = baseLabel;
					int count = 0;
					while(labels.ContainsKey(label)) {
						//Ensure labels are unique
						label = baseLabel + "_" + count.ToString();
						count++;
					}
					
					labels[label] = new CodeLabel() {
						Label = label,
						Address = (UInt32)absAddr.Address,
						MemoryType = absAddr.Type,
						Comment = "",
						Flags = CodeLabelFlags.None,
						Length = 1
					};

					existingLabels.Add(memAddr);
				}

				if(!existingSymbols.Contains(memAddr)) {
					string symName = name;
					int count = 0;
					while(_symbols.ContainsKey(symName)) {
						//Ensure labels are unique
						symName = name + "_" + count.ToString();
						count++;
					}
					_symbols[symName] = new SymbolInfo(symName, absAddr);
					existingSymbols.Add(memAddr);
				}
			}
		}
	}

	class SdccSourceFile : IFileDataProvider
	{
		public string[] Data { get; init; } = Array.Empty<string>();
	}

	private readonly struct SymbolInfo
	{
		public string Name { get; }
		public int? Value { get; } = null;
		public AddressInfo? Address { get; } = null;
		public SourceSymbol SourceSymbol { get => new SourceSymbol(Name, Address?.Address ?? Value, this); }

		public SymbolInfo(string name, AddressInfo address)
		{
			Name = name;
			Address = address;
		}

		public SymbolInfo(string name, int value)
		{
			Name = name;
			Value = value;
		}
	}
}

public class NesSdccSymbolImporter : SdccSymbolImporter
{
	private int _romSize;

	public NesSdccSymbolImporter()
	{
		_romSize = DebugApi.GetMemorySize(MemoryType.NesPrgRom);
	}

	protected override bool TryDecodeAddress(int addr, out AddressInfo absAddr)
	{
		int bank = addr >> 16;
		addr &= 0xFFFF;

		if(addr >= 0xC000) {
			absAddr = new AddressInfo() { Address = _romSize - 0x4000 + (addr & 0x3FFF), Type = MemoryType.NesPrgRom };
		} else if(addr >= 0x8000) {
			absAddr = new AddressInfo() { Address = bank * 0x4000 + (addr & 0x3FFF), Type = MemoryType.NesPrgRom };
		} else if(addr < 0x2000) {
			absAddr = new AddressInfo() { Address = addr & 0x7FF, Type = MemoryType.NesInternalRam };
		} else {
			absAddr = new();
			return false;
		}
		return true;
	}
}

public class SmsSdccSymbolImporter : SdccSymbolImporter
{
	protected override bool TryDecodeAddress(int addr, out AddressInfo absAddr)
	{
		int bank = addr >> 16;
		addr &= 0xFFFF;

		if(addr >= 0xC000) {
			absAddr = new AddressInfo() { Address = (addr & 0x1FFF), Type = MemoryType.SmsWorkRam };
		} else if(addr < 0xC000) {
			absAddr = new AddressInfo() { Address = bank * 0x4000 + (addr & 0x3FFF), Type = MemoryType.SmsPrgRom };
		} else {
			absAddr = new();
			return false;
		}
		return true;
	}
}

public class GbSdccSymbolImporter : SdccSymbolImporter
{
	protected override bool TryDecodeAddress(int addr, out AddressInfo absAddr)
	{
		int bank = addr >> 16;
		addr &= 0xFFFF;

		if(addr >= 0xC000) {
			absAddr = new AddressInfo() { Address = (addr & 0x1FFF), Type = MemoryType.GbWorkRam };
		} else if(addr < 0xC000) {
			absAddr = new AddressInfo() { Address = bank * 0x4000 + (addr & 0x3FFF), Type = MemoryType.GbPrgRom };
		} else {
			absAddr = new();
			return false;
		}
		return true;
	}
}
