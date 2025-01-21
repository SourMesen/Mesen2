using Mesen.Config;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;

namespace Mesen.Debugger.Integration
{
	public abstract class DbgImporter : ISymbolProvider
	{
		private static Regex _segmentRegex = new Regex("^seg\tid=([0-9]+),.*start=0x([0-9a-fA-F]+),.*size=0x([0-9A-Fa-f]+)", RegexOptions.Compiled);
		private static Regex _segmentPrgRomRegex = new Regex("^seg\tid=([0-9]+),.*start=0x([0-9a-fA-F]+),.*size=0x([0-9A-Fa-f]+),.*ooffs=([0-9]+)", RegexOptions.Compiled);
		private static Regex _fileRegex = new Regex("^file\tid=([0-9]+),.*name=\"([^\"]+)\"", RegexOptions.Compiled);
		private static Regex _scopeRegex = new Regex("^scope\tid=([0-9]+),.*name=\"([0-9a-zA-Z@_-]+)\"(,.*sym=([0-9+]+)){0,1}(,.*span=([0-9+]+)){0,1}", RegexOptions.Compiled);
		private static Regex _cSymbolRegex = new Regex("^csym\tid=([0-9]+),.*name=\"([0-9a-zA-Z@_-]+)\"(,.*sym=([0-9+]+)){0,1}", RegexOptions.Compiled);

		private static Regex _asmFirstLineRegex = new Regex(";(.*)", RegexOptions.Compiled);
		private static Regex _asmPreviousLinesRegex = new Regex("^\\s*;(.*)", RegexOptions.Compiled);
		private static Regex _cFirstLineRegex = new Regex("(.*)", RegexOptions.Compiled);
		private static Regex _cPreviousLinesRegex = new Regex("^\\s*//(.*)", RegexOptions.Compiled);

		private Dictionary<int, SegmentInfo> _segments = new Dictionary<int, SegmentInfo>();
		private Dictionary<int, FileInfo> _files = new Dictionary<int, FileInfo>();
		private Dictionary<int, LineInfo> _lines = new Dictionary<int, LineInfo>();
		private Dictionary<int, SpanInfo> _spans = new Dictionary<int, SpanInfo>();
		private Dictionary<int, ScopeInfo> _scopes = new Dictionary<int, ScopeInfo>();
		private Dictionary<int, SymbolInfo> _symbols = new Dictionary<int, SymbolInfo>();
		private Dictionary<int, CSymbolInfo> _cSymbols = new Dictionary<int, CSymbolInfo>();
		
		private List<SourceFileInfo> _sourceFiles = new List<SourceFileInfo>();

		private HashSet<int> _usedFileIds = new HashSet<int>();
		private HashSet<string> _usedLabels = new HashSet<string>();

		private Dictionary<MemoryType, Dictionary<int, CodeLabel>> _labelsByType = new();

		private HashSet<string> _filesNotFound = new HashSet<string>();
		private int _errorCount = 0;

		private Dictionary<int, SourceCodeLocation> _linesByPrgAddress = new Dictionary<int, SourceCodeLocation>();
		private Dictionary<int, LineInfo?[]> _linesByFile = new Dictionary<int, LineInfo?[]>();
		private Dictionary<string, int> _prgAddressByLine = new Dictionary<string, int>();
		private Dictionary<string, int> _prgAddressEndByLine = new Dictionary<string, int>();
		protected HashSet<int> _scopeSpans = new HashSet<int>();

		private Dictionary<int, ScopeInfo> _scopesBySymbol = new Dictionary<int, ScopeInfo>();

		public DateTime SymbolFileStamp { get; private set; }
		public string SymbolPath { get; private set; } = "";

		public List<SourceFileInfo> SourceFiles { get { return _sourceFiles; } }

		private CpuType _cpuType;
		private MemoryType _cpuMemType;
		private MemoryType _prgMemType;
		private List<MemoryType> _memTypesToImport;
		private int _headerSize;

		public DbgImporter(CpuType type, RomFormat format, List<MemoryType> memTypesToImport)
		{
			_cpuType = type;
			_cpuMemType = _cpuType.ToMemoryType();
			_prgMemType = _cpuType.GetPrgRomMemoryType();
			_headerSize = format == RomFormat.iNes ? 16 : 0;
			_memTypesToImport = memTypesToImport;
		}

		public AddressInfo? GetLineAddress(SourceFileInfo file, int lineIndex)
		{
			if(file.InternalFile is FileInfo dbgFile) {
				return GetPrgAddress(dbgFile.ID, lineIndex);
			}
			return null;
		}

		public AddressInfo? GetLineEndAddress(SourceFileInfo file, int lineIndex)
		{
			if(file.InternalFile is FileInfo dbgFile) {
				return GetPrgEndAddress(dbgFile.ID, lineIndex);
			}
			return null;
		}

		private AddressInfo? GetPrgAddress(int fileID, int lineIndex)
		{
			int prgAddress;
			if(_prgAddressByLine.TryGetValue(fileID.ToString() + "_" + lineIndex.ToString(), out prgAddress)) {
				return new AddressInfo() { Address = prgAddress, Type = _prgMemType };
			}
			return null;
		}

		private AddressInfo? GetPrgEndAddress(int fileID, int lineIndex)
		{
			int prgAddress;
			if(_prgAddressEndByLine.TryGetValue(fileID.ToString() + "_" + lineIndex.ToString(), out prgAddress)) {
				return new AddressInfo() { Address = prgAddress, Type = _prgMemType };
			}
			return null;
		}

		private int GetPrgAddress(SpanInfo span)
		{
			SegmentInfo segment;
			if(_segments.TryGetValue(span.SegmentID, out segment)) {
				if(!segment.IsRam && span.Size != segment.Size) {
					return span.Offset + segment.FileOffset - _headerSize;
				}
			}
			return -1;
		}

		public SourceCodeLocation? GetSourceCodeLineInfo(AddressInfo address)
		{
			if(address.Type == _prgMemType && _linesByPrgAddress.TryGetValue(address.Address, out SourceCodeLocation line)) {
				return line;
			}
			return null;
		}

		public string? GetSourceCodeLine(int prgRomAddress)
		{
			if(prgRomAddress >= 0) {
				try {
					if(_linesByPrgAddress.TryGetValue(prgRomAddress, out SourceCodeLocation line)) {
						string output = "";
						SourceFileInfo file = line.File;
						if(file.Data == null) {
							return string.Empty;
						}

						output += file.Data[line.LineNumber];
						return output;
					}
				} catch { }
			}
			return null;
		}

		private SourceCodeLocation? GetReferenceInfo(int referenceId)
		{
			if(_lines.TryGetValue(referenceId, out LineInfo line)) {
				string lineContent = "";
				if(_files.TryGetValue(line.FileID, out FileInfo? file) && file.Data != null && file.Data.Length > line.LineNumber) {
					lineContent = file.Data[line.LineNumber];
				}

				return new SourceCodeLocation(_files[line.FileID].SourceFile, line.LineNumber, line);
			}

			return null;
		}

		public SourceCodeLocation? GetSymbolDefinition(SourceSymbol symbol)
		{
			if(symbol.InternalSymbol is SymbolInfo dbgSymbol) {
				foreach(int definition in dbgSymbol.Definitions) {
					SourceCodeLocation? refInfo = GetReferenceInfo(definition);
					if(refInfo != null) {
						return refInfo;
					}
				}
			}

			return null;
		}

		private List<SourceCodeLocation?> GetSymbolReferences(SymbolInfo symbol)
		{
			List<SourceCodeLocation?> references = new();
			foreach(int reference in symbol.References) {
				SourceCodeLocation? refInfo = GetReferenceInfo(reference);
				if(refInfo != null) {
					references.Add(refInfo);
				}
			}
			return references;
		}

		private SpanInfo? GetSymbolDefinitionSpan(SymbolInfo symbol)
		{
			foreach(int definition in symbol.Definitions) {
				if(_lines.TryGetValue(definition, out LineInfo definitionLine)) {
					LineInfo? line = definitionLine;
					if(_files.TryGetValue(definitionLine.FileID, out FileInfo? file)) {
						int lineNumber = definitionLine.LineNumber;
						while(!(line?.SpanIDs.Length > 0) && lineNumber < _linesByFile[file.ID].Length - 1) {
							//Definition line contains no code, try the next line
							lineNumber++;
							line = _linesByFile[file.ID][lineNumber];
						}

						if(line != null && definitionLine.SpanIDs.Length > 0) {
							if(_spans.TryGetValue(definitionLine.SpanIDs[0], out SpanInfo span)) {
								return span;
							}
						}
					}
				}
			}
			return null;
		}

		private SymbolInfo? GetMatchingSymbol(SymbolInfo symbol, int rangeStart, int rangeEnd)
		{
			AddressInfo? symbolAddress = GetSymbolAddressInfo(symbol);
			if(symbolAddress != null && symbolAddress.Value.Type == _prgMemType && symbolAddress.Value.Address >= rangeStart && symbolAddress.Value.Address <= rangeEnd) {
				//If the range start/end matches the symbol's definition, return it
				return symbol;
			}

			foreach(int reference in symbol.References) {
				LineInfo line = _lines[reference];

				foreach(int spanID in line.SpanIDs) {
					SpanInfo span = _spans[spanID];
					SegmentInfo seg = _segments[span.SegmentID];

					if(!seg.IsRam) {
						int spanPrgOffset = seg.FileOffset - _headerSize + span.Offset;
						if(rangeStart < spanPrgOffset + span.Size && rangeEnd >= spanPrgOffset) {
							if(symbol.ExportSymbolID != null && symbol.Address == null) {
								return _symbols[symbol.ExportSymbolID.Value];
							} else {
								return symbol;
							}
						}
					}
				}
			}
			return null;
		}

		public List<SourceSymbol> GetSymbols()
		{
			return _symbols.Values.Select(s => s.SourceSymbol).ToList();
		}

		public SourceSymbol? GetSymbol(string word, int scopeStart, int scopeEnd)
		{
			try {
				foreach(CSymbolInfo symbol in _cSymbols.Values) {
					if(symbol.Name == word && symbol.SymbolID.HasValue) {
						SymbolInfo? matchingSymbol = GetMatchingSymbol(_symbols[symbol.SymbolID.Value], scopeStart, scopeEnd);
						if(matchingSymbol != null) {
							return matchingSymbol.Value.SourceSymbol;
						}
					}
				}

				foreach(SymbolInfo symbol in _symbols.Values) {
					if(symbol.Name == word) {
						SymbolInfo? matchingSymbol = GetMatchingSymbol(symbol, scopeStart, scopeEnd);
						if(matchingSymbol != null) {
							return matchingSymbol.Value.SourceSymbol;
						}
					}
				}
			} catch { }

			return null;
		}

		public AddressInfo? GetSymbolAddressInfo(SourceSymbol symbol)
		{
			if(symbol.InternalSymbol is SymbolInfo dbgSymbol) {
				return GetSymbolAddressInfo(dbgSymbol);
			}
			return null;
		}

		private AddressInfo? GetSymbolAddressInfo(SymbolInfo symbol)
		{
			if(symbol.SegmentID == null || symbol.Address == null) {
				return null;
			}

			SegmentInfo segment = _segments[symbol.SegmentID.Value];
			if(segment.IsRam) {
				if(segment.MemType != null) {
					return new AddressInfo() { Address = symbol.Address.Value, Type = segment.MemType.Value };
				} else {
					int labelAddress;
					MemoryType? addressType;
					GetRamLabelAddressAndType(symbol.Address.Value, out labelAddress, out addressType);
					if(addressType.HasValue) {
						return new AddressInfo() { Address = labelAddress, Type = addressType.Value };
					} else {
						return null;
					}
				}
			} else {
				return new AddressInfo() { Address = symbol.Address.Value - segment.Start + segment.FileOffset - _headerSize, Type = _prgMemType };
			}
		}

		private bool LoadSegments(string row)
		{
			Match match = _segmentRegex.Match(row);
			if(match.Success) {
				int id = Int32.Parse(match.Groups[1].Value);
				int start = Int32.Parse(match.Groups[2].Value, NumberStyles.HexNumber);
				int size = Int32.Parse(match.Groups[3].Value, NumberStyles.HexNumber);
				bool isRam = true;
				int fileOffset = 0;
				MemoryType? memType = null;

				match = _segmentPrgRomRegex.Match(row);
				if(match.Success) {
					fileOffset = Int32.Parse(match.Groups[4].Value);
					isRam = false;

					if(row.Contains("type=rw")) {
						//TODOv2 fix this
						//Assume a RW segment inside the .sfc file is SPC code
						isRam = true;
						memType = MemoryType.SpcRam;
					}
				}

				SegmentInfo segment = new SegmentInfo(id, start, size, isRam, fileOffset, memType);
				_segments.Add(segment.ID, segment);
				return true;
			} else if(row.StartsWith("seg")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match seg");
			}

			return false;
		}

		private bool LoadFiles(string row, string basePath)
		{
			Match match = _fileRegex.Match(row);
			if(match.Success) {
				string filename = Path.GetFullPath(Path.Combine(basePath, match.Groups[2].Value.Replace('/', Path.DirectorySeparatorChar).Replace('\\', Path.DirectorySeparatorChar))).Replace(basePath + Path.DirectorySeparatorChar, "");
				string ext = Path.GetExtension(filename).ToLower();
				bool isAsm = ext != ".c" && ext != ".h";
				int id = Int32.Parse(match.Groups[1].Value);

				FileInfo file = new FileInfo(id, filename, isAsm);
				_sourceFiles.Add(file.SourceFile);

				_files.Add(file.ID, file);
				return true;
			} else if(row.StartsWith("file")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match file");
			}

			return false;
		}

		private bool LoadLines(string row)
		{
			int? id = null;
			int? fileID = null;
			int? lineNumber = null;
			LineType type = LineType.Assembly;
			int[] spanIds = Array.Empty<int>();

			DbgReader.ReadEntry(row, (ref ReadOnlySpan<char> name, ref ReadOnlySpan<char> data) => {
				if(name.IsEqual("id")) {
					id = Int32.Parse(data);
				} else if(name.IsEqual("file")) {
					fileID = Int32.Parse(data);
				} else if(name.IsEqual("line")) {
					//Sometimes line is set to 0 in the .dbg file (might only occur for C projects?)
					lineNumber = Math.Max(0, Int32.Parse(data) - 1);
				} else if(name.IsEqual("type")) {
					type = (LineType)Int32.Parse(data);
				} else if(name.IsEqual("span")) {
					spanIds = DbgReader.ReadIntArray(data);
				}
			});

			if(id != null && fileID != null && lineNumber != null) {
				LineInfo line = new LineInfo(id.Value, fileID.Value, lineNumber.Value, type, _files[fileID.Value].SourceFile, spanIds);
				_usedFileIds.Add(line.FileID);
				_lines.Add(line.ID, line);
				return true;
			} else {
				System.Diagnostics.Debug.Fail("Regex doesn't match line");
			}

			return false;
		}

		private bool LoadSpans(string row)

		{
			int? id = null;
			int? segmentID = null;
			int? offset = null;
			int? size = null;
			bool isData = false;

			DbgReader.ReadEntry(row, (ref ReadOnlySpan<char> name, ref ReadOnlySpan<char> data) => {
				if(name.IsEqual("id")) {
					id = Int32.Parse(data);
				} else if(name.IsEqual("seg")) {
					segmentID = Int32.Parse(data);
				} else if(name.IsEqual("start")) {
					offset = Int32.Parse(data);
				} else if(name.IsEqual("size")) {
					size = Int32.Parse(data);
				} else if(name.IsEqual("type")) {
					isData = true;
				}
			});

			if(id != null && segmentID != null && offset != null && size != null) {
				SpanInfo span = new SpanInfo(id.Value, segmentID.Value, offset.Value, size.Value, isData);
				_spans.Add(span.ID, span);
				return true;
			} else {
				System.Diagnostics.Debug.Fail("Regex doesn't match span");
			}

			return false;
		}

		private bool LoadCSymbols(string row)
		{
			Match match = _cSymbolRegex.Match(row);
			if(match.Success) {
				int id = Int32.Parse(match.Groups[1].Value);
				string name = match.Groups[2].Value;
				int? symbolID = match.Groups[4].Success ? (int?)Int32.Parse(match.Groups[4].Value) : null;

				CSymbolInfo symbol = new CSymbolInfo(id, name, symbolID);
				_cSymbols.Add(symbol.ID, symbol);
				return true;
			} else {
				System.Diagnostics.Debug.Fail("Regex doesn't match csym");
			}

			return false;
		}

		private bool LoadScopes(string row)
		{
			Match match = _scopeRegex.Match(row);
			if(match.Success) {
				int id = Int32.Parse(match.Groups[1].Value);
				string name = match.Groups[2].Value;
				int? symbolID = match.Groups[4].Success ? (int?)Int32.Parse(match.Groups[4].Value) : null;

				int[] spans = Array.Empty<int>();
				if(match.Groups[6].Success) {
					spans = match.Groups[6].Value.Split('+').Select(o => Int32.Parse(o)).ToArray();
					Array.ForEach(spans, id => _scopeSpans.Add(id));
				}

				ScopeInfo scope = new ScopeInfo(id, name, symbolID, spans);
				if(scope.SymbolID.HasValue) {
					_scopesBySymbol[scope.SymbolID.Value] = scope;
				}
				_scopes.Add(scope.ID, scope);
				return true;
			} else {
				//System.Diagnostics.Debug.Fail("Regex doesn't match scope");
			}

			return false;
		}

		private bool LoadSymbols(string row)
		{
			int? id = null;
			string? symbolName = null;
			int? address = null;
			int? segmentId = null;
			int? exportSymbolId = null;
			int? size = null;
			int[] definitions = Array.Empty<int>();
			int[] references = Array.Empty<int>();
			string? type = null;

			DbgReader.ReadEntry(row, (ref ReadOnlySpan<char> name, ref ReadOnlySpan<char> data) => {
				if(name.IsEqual("id")) {
					id = Int32.Parse(data);
				} else if(name.IsEqual("name")) {
					symbolName = data.Slice(1, data.Length - 2).ToString();
				} else if(name.IsEqual("size")) {
					size = Int32.Parse(data);
				} else if(name.IsEqual("val")) {
					address = Int32.Parse(data.Slice(2), NumberStyles.HexNumber);
				} else if(name.IsEqual("seg")) {
					segmentId = Int32.Parse(data);
				} else if(name.IsEqual("exp")) {
					exportSymbolId = Int32.Parse(data);
				} else if(name.IsEqual("ref")) {
					references = DbgReader.ReadIntArray(data);
				} else if(name.IsEqual("def")) {
					definitions = DbgReader.ReadIntArray(data);
				} else if(name.IsEqual("type")) {
					type = data.ToString();					
				}
			});

			if(id != null && symbolName != null) {
				//Ignore negative addresses (e.g 0xFFFF8000)
				if((address == null || address >= 0) && (size == null || size >= 0)) {
					SymbolInfo symbol = new SymbolInfo(id.Value, symbolName, address, segmentId, exportSymbolId, size, definitions, references, type);
					_symbols.Add(symbol.ID, symbol);
				}
				return true;
			} else {
				System.Diagnostics.Debug.Fail("Regex doesn't match sym");
			}

			return false;
		}

		public int GetSymbolSize(SourceSymbol srcSymbol)
		{
			if(srcSymbol.InternalSymbol is SymbolInfo dbgSymbol) {
				return GetSymbolSize(dbgSymbol);
			}
			return 1;
		}
		
		private int GetSymbolSize(SymbolInfo symbol)
		{
			if(symbol.SegmentID != null && _segments.ContainsKey(symbol.SegmentID.Value)) {
				SegmentInfo segment = _segments[symbol.SegmentID.Value];
				SpanInfo? defSpan = null;
				if(!segment.IsRam) {
					defSpan = GetSymbolDefinitionSpan(symbol);
				}

				if(_scopesBySymbol.ContainsKey(symbol.ID)) {
					//This symbol actually denotes the start of a scope (.scope or .proc) and isn't actually data, return a size of 1
					return 1;
				} else {
					return (defSpan == null || defSpan.Value.IsData) ? (symbol.Size ?? 1) : 1;
				}
			}

			return 1;
		}

		private void GetRamLabelAddressAndType(int address, out int absoluteAddress, out MemoryType? memoryType)
		{
			if(address < 0) {

			}
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = _cpuMemType });
			absoluteAddress = absAddress.Address;
			if(absoluteAddress >= 0) {
				memoryType = absAddress.Type;
			} else {
				memoryType = null;
			}
		}

		private CodeLabel CreateLabel(Int32 address, MemoryType memoryType, UInt32 length)
		{
			if(!_labelsByType.TryGetValue(memoryType, out Dictionary<int, CodeLabel>? labels)) {
				labels = new();
				_labelsByType[memoryType] = labels;
			}

			CodeLabel? label = null;
			if(!labels.TryGetValue(address, out label)) {
				label = new CodeLabel() { Address = (UInt32)address, MemoryType = memoryType, Comment = string.Empty, Label = string.Empty, Length = length };
				labels[address] = label;
			}

			return label;
		}

		private void LoadLabels()
		{
			Dictionary<MemoryType, Dictionary<int, List<string>>> labelAliases = GatherLabelAliases();

			foreach(KeyValuePair<int, SymbolInfo> kvp in _symbols) {
				try {
					SymbolInfo symbol = kvp.Value;
					if(symbol.SegmentID == null) {
						continue;
					}

					if(_segments.ContainsKey(symbol.SegmentID.Value)) {
						SegmentInfo segment = _segments[symbol.SegmentID.Value];

						int count = 2;
						string orgSymbolName = symbol.Name;
						if(!LabelManager.LabelRegex.IsMatch(orgSymbolName)) {
							//ignore labels that don't respect the label naming restrictions
							continue;
						}

						string newName = symbol.Name;
						while(!_usedLabels.Add(newName)) {
							//Ensure labels are unique
							newName = orgSymbolName + "_" + count.ToString();
							count++;
						}

						AddressInfo? addressInfo = GetSymbolAddressInfo(symbol);
						if(addressInfo != null && (symbol.Type == "lab" || symbol.Size != null)) {
							CodeLabel label = this.CreateLabel(addressInfo.Value.Address, addressInfo.Value.Type, (uint)GetSymbolSize(symbol));
							label.Label = newName;
							label.Length = (uint)GetSymbolSize(symbol);

							//Add aliases to comment if aliases exist
							if(labelAliases.TryGetValue(addressInfo.Value.Type, out Dictionary<int, List<string>>? aliases)) {
								if(aliases.TryGetValue(addressInfo.Value.Address, out List<string>? aliasList)) {
									StringBuilder aliasComment = new();
									foreach(string alias in aliasList) {
										if(alias != newName) {
											if(aliasComment.Length == 0) {
												aliasComment.AppendLine("Aliases:");
											}
											aliasComment.AppendLine(alias);
										}
									}
									label.Comment = aliasComment.ToString().Trim();
								}
							}
						}
					}
				} catch {
					_errorCount++;
				}
			}
		}

		private Dictionary<MemoryType, Dictionary<int, List<string>>> GatherLabelAliases()
		{
			//Generate a list of all known aliases for addresses (type=equ or size undefined)
			Dictionary<MemoryType, Dictionary<int, List<string>>> labelAliases = new();
			foreach(KeyValuePair<int, SymbolInfo> kvp in _symbols) {
				try {
					SymbolInfo symbol = kvp.Value;
					if(symbol.Type != "lab" || symbol.Size == null) {
						AddressInfo? addr = GetSymbolAddressInfo(symbol);
						if(addr != null) {
							if(!labelAliases.TryGetValue(addr.Value.Type, out Dictionary<int, List<string>>? aliases)) {
								aliases = new();
								labelAliases[addr.Value.Type] = aliases;
							}

							List<string>? aliasList = null;
							if(aliases.TryGetValue(addr.Value.Address, out aliasList)) {
								aliasList.Add(symbol.Name);
							} else {
								aliasList = new() { symbol.Name };
							}

							aliases[addr.Value.Address] = aliasList;
						}
					}
				} catch { }
			}

			return labelAliases;
		}

		private void LoadComments()
		{
			SortedDictionary<string, int> constants = GetConstants();
			foreach(KeyValuePair<int, LineInfo> kvp in _lines) {
				try {
					LineInfo line = kvp.Value;
					if(line.SpanIDs.Length == 0) {
						continue;
					}

					SpanInfo span = _spans[line.SpanIDs[0]];
					SegmentInfo segment = _segments[span.SegmentID];

					if(_files[line.FileID].IsFileMissing) {
						//File was not found.
						if(_filesNotFound.Add(_files[line.FileID].Name)) {
							_errorCount++;
						}
						continue;
					}

					bool isAsm = _files[line.FileID].IsAssembly;

					string comment = "";
					for(int i = line.LineNumber; i >= 0; i--) {
						if(i >= _files[line.FileID].Data.Length) {
							//Invalid line number, usually caused by a mismatch between the DBG file and source files
							_errorCount++;
							continue;
						}

						string sourceCodeLine = _files[line.FileID].Data[i];

						Regex regex;
						if(i == line.LineNumber) {
							regex = isAsm ? _asmFirstLineRegex : _cFirstLineRegex;
						} else {
							regex = isAsm ? _asmPreviousLinesRegex : _cPreviousLinesRegex;
						}

						Match match = regex.Match(sourceCodeLine);
						if(match.Success) {
							string matchedComment = match.Groups[1].Value.Replace("\t", " ");
							if(string.IsNullOrWhiteSpace(comment)) {
								comment = matchedComment;
							} else {
								comment = matchedComment + Environment.NewLine + comment;
							}
						} else if(i != line.LineNumber) {
							break;
						}
					}

					if(comment.Length > 0) {
						int address = -1;
						MemoryType? memoryType;
						if(segment.IsRam) {
							if(segment.MemType != null) {
								address = span.Offset + segment.Start;
								memoryType = segment.MemType.Value;
							} else {
								GetRamLabelAddressAndType(span.Offset + segment.Start, out address, out memoryType);
							}
						} else {
							address = GetPrgAddress(span);
							memoryType = _prgMemType;
						}

						if(address >= 0 && memoryType != null) {
							CodeLabel label = this.CreateLabel(address, memoryType.Value, 1);
							if(label != null) {
								label.Comment = ParseAsserts(constants, comment);
							}
						}
					}
				} catch {
					_errorCount++;
				}
			}
		}

		private string[] _splitOnNewLine = { Environment.NewLine };
		private string ParseAsserts(SortedDictionary<string, int> constants, string comment)
		{
			//Parse and replace content of asserts as needed
			string[] commentLines = comment.Split(_splitOnNewLine, StringSplitOptions.None);
			for(int i = 0; i < commentLines.Length; i++) {
				Match m = LabelManager.AssertRegex.Match(commentLines[i]);
				if(m.Success) {
					foreach(KeyValuePair<string, int> entry in constants) {
						commentLines[i] = commentLines[i].Replace(entry.Key, entry.Value.ToString());
					}
				}
			}

			return string.Join(Environment.NewLine, commentLines);
		}

		private SortedDictionary<string, int> GetConstants()
		{
			SortedDictionary<string, int> constants = new SortedDictionary<string, int>(Comparer<string>.Create((string a, string b) => {
				if(a.Length == b.Length) {
					return a.CompareTo(b);
				}
				return b.Length - a.Length;
			}));

			foreach(SymbolInfo symbol in _symbols.Values) {
				AddressInfo? addressInfo = GetSymbolAddressInfo(symbol);
				if(!addressInfo.HasValue && symbol.Address.HasValue) {
					constants[symbol.Name] = symbol.Address.Value;
				}
			}

			return constants;
		}

		private void LoadFileData(string path)
		{
			Dictionary<int, int> maxLineCountByFile = new Dictionary<int, int>();

			foreach(LineInfo line in _lines.Values) {
				int currentMax;
				if(maxLineCountByFile.TryGetValue(line.FileID, out currentMax)) {
					if(currentMax < line.LineNumber) {
						maxLineCountByFile[line.FileID] = line.LineNumber;
					}
				} else {
					maxLineCountByFile[line.FileID] = line.LineNumber;
				}
			}

			foreach(LineInfo line in _lines.Values) {
				if(!_linesByFile.TryGetValue(line.FileID, out LineInfo?[]? fileLines)) {
					fileLines = new LineInfo?[maxLineCountByFile[line.FileID] + 1];
					_linesByFile[line.FileID] = fileLines;
				}
				fileLines[line.LineNumber] = line;
			}

			foreach(FileInfo file in _files.Values) {
				if(_usedFileIds.Contains(file.ID)) {
					try {
						string? basePath = path;
						string sourceFile = Path.Combine(basePath, file.Name);
						while(!File.Exists(sourceFile)) {
							//Go back up folder structure to attempt to find the file
							string oldPath = basePath;
							basePath = Path.GetDirectoryName(basePath);
							if(basePath == null || basePath == oldPath) {
								break;
							}
							sourceFile = Path.Combine(basePath, file.Name);
						}

						if(File.Exists(sourceFile)) {
							file.SetSourceFile(sourceFile);
						}
					} catch {
						_errorCount++;
					}
				}
			}
		}

		private void BuildCdlData()
		{
			int prgSize = DebugApi.GetMemorySize(_prgMemType);
			if(prgSize <= 0) {
				return;
			}

			byte[] cdlFile = new byte[prgSize];
			byte[] prgRomContent = DebugApi.GetMemoryState(_prgMemType);

			//Mark data/code regions
			foreach(SpanInfo span in _spans.Values) {
				if(_scopeSpans.Contains(span.ID)) {
					//Skip any span used by a scope, they don't correspond to an actual line of code
					continue;
				}

				int prgAddress = GetPrgAddress(span);
				if(prgAddress >= 0 && prgAddress < prgSize) {
					for(int i = 0; i < span.Size; i++) {
						if(cdlFile[prgAddress + i] != (byte)CdlFlags.Data && !span.IsData && span.Size <= 4) {
							byte opCode = prgRomContent[prgAddress];
							cdlFile[prgAddress + i] = (byte)(CdlFlags.Code | GetOpFlags(opCode, span.Size));
						} else if(span.IsData) {
							cdlFile[prgAddress + i] = (byte)CdlFlags.Data;
						} else if(cdlFile[prgAddress + i] == 0) {
							//Mark bytes as tentative data, until we know that the bytes are actually code
							cdlFile[prgAddress + i] = 0x04;
						}
					}
				}
			}
			for(int i = 0; i < cdlFile.Length; i++) {
				if(cdlFile[i] == 0x04) {
					//Mark all bytes marked as tentative data as data
					cdlFile[i] = (byte)CdlFlags.Data;
				}
			}

			//Find/identify functions and jump targets
			foreach(SymbolInfo symbol in _symbols.Values) {
				if(!symbol.SegmentID.HasValue) {
					//This is a constant, ignore it
					continue;
				}

				foreach(int reference in symbol.References) {
					if(_lines.TryGetValue(reference, out LineInfo line) && line.SpanIDs.Length > 0) {
						if(_spans.TryGetValue(line.SpanIDs[0], out SpanInfo span) && !span.IsData && span.Size <= 4) {
							int referencePrgAddr = GetPrgAddress(span);
							if(referencePrgAddr >= 0 && referencePrgAddr < prgRomContent.Length) {
								byte opCode = prgRomContent[referencePrgAddr];
								if(IsBranchInstruction(opCode)) {
									//This symbol is used with a JSR/jump instruction, so it's either a function or jump target
									SpanInfo? definitionSpan = GetSymbolDefinitionSpan(symbol);
									if(definitionSpan != null) {
										int definitionPrgAddr = GetPrgAddress(definitionSpan.Value);
										if(definitionPrgAddr >= 0 && definitionPrgAddr < prgRomContent.Length) {
											bool isJsr = IsJumpToSubroutine(opCode);
											cdlFile[definitionPrgAddr] |= (byte)(isJsr ? CdlFlags.SubEntryPoint : CdlFlags.JumpTarget);
											break;
										}
									}
								}
							}
						}
					}
				}
			}

			DebugApi.SetCdlData(_prgMemType, cdlFile, cdlFile.Length);
		}

		protected abstract bool IsBranchInstruction(byte opCode);
		protected abstract bool IsJumpToSubroutine(byte opCode);
		protected abstract CdlFlags GetOpFlags(byte opCode, int opSize);

		public static DbgImporter? Import(RomFormat romFormat, string path, bool importComments, bool showResult)
		{
			DbgImporter? importer = romFormat switch {
				RomFormat.Sfc => new SnesDbgImporter(romFormat),
				RomFormat.iNes or RomFormat.Nsf or RomFormat.VsSystem or RomFormat.VsDualSystem => new NesDbgImporter(romFormat),
				RomFormat.Pce or RomFormat.PceHes => new PceDbgImporter(romFormat),
				_ => null
			};

			if(importer == null) {
				EmuApi.WriteLogEntry("[Debugger] Error: Attempted to load .dbg file for an unsupported file format: " + romFormat);
			}

			importer?.Import(path, importComments, showResult);

			return importer;
		}

		private void Import(string path, bool importComments, bool showResult)
		{
			SymbolFileStamp = File.GetLastWriteTime(path);

			string basePath = Path.GetDirectoryName(path) ?? "";
			SymbolPath = basePath;

			foreach(string row in File.ReadLines(path)) {
				try {
					switch(row.Substring(0, 3)) {
						case "lin": LoadLines(row); break;
						case "spa": LoadSpans(row); break;
						case "sym": LoadSymbols(row); break;
						case "csy": LoadCSymbols(row); break;
						case "sco": LoadScopes(row); break;
						case "fil": LoadFiles(row, basePath); break;
						case "seg": LoadSegments(row); break;
					}
				} catch {
					_errorCount++;
				}
			}

			LoadFileData(basePath);

			BuildCdlData();
			
			int prgSize = DebugApi.GetMemorySize(_prgMemType);
			foreach(LineInfo line in _lines.Values) {
				SourceCodeLocation location = line.GetLocation();
				foreach(int spanID in line.SpanIDs) {
					if(_spans.TryGetValue(spanID, out SpanInfo span)) {
						if(_segments.TryGetValue(span.SegmentID, out SegmentInfo segment) && !segment.IsRam) {
							for(int i = 0; i < span.Size; i++) {
								int prgAddress = segment.FileOffset - _headerSize + span.Offset + i;
								if(prgAddress < 0 || prgAddress >= prgSize) {
									//Negative addresses can be generated if the current span/segment is the .nes file header
									//Addresses beyond the PRG size can also be generated when CHR data is included on that row (NES)
									continue;
								}

								if(_linesByPrgAddress.TryGetValue(prgAddress, out SourceCodeLocation existingLine)) {
									//If the line was already assigned to a line marked as "External", keep that instead of overwriting it
									//This is usually used to denote lines that are C code (rather than assembly)
									if(existingLine.InternalLine is LineInfo internalLine && internalLine.Type == LineType.External) {
										continue;
									}
								}

								_linesByPrgAddress[prgAddress] = location;
								if(i == 0 && spanID == line.SpanIDs[0]) {
									//Mark the first byte of the first span representing this line as the PRG address for this line of code
									FileInfo file = _files[line.FileID];
									_prgAddressByLine[file.ID.ToString() + "_" + line.LineNumber.ToString()] = prgAddress;
									_prgAddressEndByLine[file.ID.ToString() + "_" + line.LineNumber.ToString()] = prgAddress + span.Size - 1;
								}
							}
						}
					}
				}
			}

			LoadLabels();

			if(importComments) {
				LoadComments();
			}
			
			List<CodeLabel> labelsToImport = new List<CodeLabel>();
			foreach(MemoryType memType in _memTypesToImport) {
				if(_labelsByType.TryGetValue(memType, out var labels) && ConfigManager.Config.Debug.Integration.IsMemoryTypeImportEnabled(memType)) {
					labelsToImport.AddRange(labels.Values);
				}
			}

			LabelManager.SetLabels(labelsToImport, true);

			if(showResult) {
				if(_errorCount > 0) {
					_errorCount -= _filesNotFound.Count;
					StringBuilder missingFiles = new();
					foreach(string file in _filesNotFound) {
						missingFiles.AppendLine(file);
					}

					if(_errorCount > 0) {
						if(_filesNotFound.Count > 0) {
							MesenMsgBox.Show(null, "ImportLabelsWithErrorsAndMissingFiles", MessageBoxButtons.OK, MessageBoxIcon.Warning, labelsToImport.Count.ToString(), _errorCount.ToString(), missingFiles.ToString());
						} else {
							MesenMsgBox.Show(null, "ImportLabelsWithErrors", MessageBoxButtons.OK, MessageBoxIcon.Warning, labelsToImport.Count.ToString(), _errorCount.ToString());
						}
					} else {
						MesenMsgBox.Show(null, "ImportLabelsWithMissingFiles", MessageBoxButtons.OK, MessageBoxIcon.Warning, labelsToImport.Count.ToString(), missingFiles.ToString());
					}
				} else {
					MesenMsgBox.Show(null, "ImportLabels", MessageBoxButtons.OK, MessageBoxIcon.Info, labelsToImport.Count.ToString());
				}
			}
		}

		private readonly struct SegmentInfo
		{
			public int ID { get; }
			public int Start { get; }
			public int Size { get; }
			public int FileOffset { get; }
			public bool IsRam { get; }
			public MemoryType? MemType { get; }

			public SegmentInfo(int id, int start, int size, bool isRam, int fileOffset, MemoryType? memType)
			{
				ID = id;
				Start = start;
				Size = size;
				IsRam = isRam;
				FileOffset = fileOffset;
				MemType = memType;
			}
		}

		private class FileInfo : IFileDataProvider
		{
			public int ID { get; }
			public string Name { get; }
			public bool IsAssembly { get; }
			public bool IsFileMissing { get; private set; }
			public SourceFileInfo SourceFile { get; }

			private string[]? _data = null;
			private string? _sourceFile = null;
			public string[] Data
			{
				get
				{
					if(_data != null) {
						return _data;
					} else if(_sourceFile == null || !File.Exists(_sourceFile)) {
						_data = Array.Empty<string>();
					} else {
						_data = File.ReadAllLines(_sourceFile);
					}

					return _data;
				}
			}

			public FileInfo(int id, string filename, bool isAsm)
			{
				ID = id;
				Name = filename;
				IsAssembly = isAsm;
				IsFileMissing = true;
				SourceFile = new SourceFileInfo(Name, IsAssembly, this);
			}

			public void SetSourceFile(string sourceFile)
			{
				_sourceFile = sourceFile;
				IsFileMissing = false;
			}
		}

		private readonly struct LineInfo
		{
			public int ID { get; }
			public int FileID { get; }
			public int[] SpanIDs { get; }
			public LineType Type { get; }
			public int LineNumber { get; }
			public SourceFileInfo SourceFile { get; }
			
			public SourceCodeLocation GetLocation() => new SourceCodeLocation(SourceFile, LineNumber, this);

			public LineInfo(int id, int fileID, int lineNumber, LineType type, SourceFileInfo sourceFile, int[] spanIds)
			{
				ID = id;
				FileID = fileID;
				LineNumber = lineNumber;
				Type = type;
				SourceFile = sourceFile;
				SpanIDs = spanIds;
			}
		}

		private enum LineType
		{
			Assembly = 0,
			External = 1, //i.e C source file
			Macro = 2
		}

		private readonly struct SpanInfo
		{
			public int ID { get; }
			public int SegmentID { get; }
			public int Offset { get; }
			public int Size { get; }
			public bool IsData { get; }

			public SpanInfo(int id, int segmentID, int offset, int size, bool isData)
			{
				ID = id;
				SegmentID = segmentID;
				Offset = offset;
				Size = size;
				IsData = isData;
			}
		}

		private readonly struct SymbolInfo
		{
			public int ID { get; }
			public string Name { get; }
			public int? Address { get; }
			public int? SegmentID { get; }
			public int? ExportSymbolID { get; }
			public int? Size { get; }
			public string? Type { get; } //"lab", "equ" or "imp" (label, equate, import?)
			public int[] References { get; }
			public int[] Definitions { get; }
			public SourceSymbol SourceSymbol { get => new SourceSymbol(Name, Address, this); }

			public SymbolInfo(int id, string name, int? address, int? segmentId, int? exportSymbolId, int? size, int[] definitions, int[] references, string? type)
			{
				ID = id;
				Name = name;
				Address = address;
				SegmentID = segmentId;
				ExportSymbolID = exportSymbolId;
				Size = size;
				Definitions = definitions;
				References = references;
				Type = type;
			}
		}

		private readonly struct ScopeInfo
		{
			public ScopeInfo(int id, string name, int? symbolID, int[] spans)
			{
				ID = id;
				Name = name;
				SymbolID = symbolID;
				Spans = spans;
			}

			public int ID { get; }
			public string Name { get; }
			public int? SymbolID { get; }
			public int[] Spans { get; }
		}

		private class CSymbolInfo
		{
			public int ID { get; }
			public string Name { get; }
			public int? SymbolID { get; }

			public CSymbolInfo(int id, string name, int? symbolID)
			{
				ID = id;
				Name = name;
				SymbolID = symbolID;
			}
		}
	}
}
