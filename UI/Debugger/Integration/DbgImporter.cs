using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Windows.Forms;
using Mesen.GUI.Config;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Forms;

namespace Mesen.GUI.Debugger.Integration
{
	public class DbgImporter : ISymbolProvider
	{
		private int _headerSize = 0;

		private Dictionary<int, SegmentInfo> _segments = new Dictionary<int, SegmentInfo>();
		private Dictionary<int, FileInfo> _files = new Dictionary<int, FileInfo>();
		private Dictionary<int, LineInfo> _lines = new Dictionary<int, LineInfo>();
		private Dictionary<int, SpanInfo> _spans = new Dictionary<int, SpanInfo>();
		private Dictionary<int, ScopeInfo> _scopes = new Dictionary<int, ScopeInfo>();
		private Dictionary<int, SymbolInfo> _symbols = new Dictionary<int, SymbolInfo>();
		private Dictionary<int, CSymbolInfo> _cSymbols = new Dictionary<int, CSymbolInfo>();
		
		private List<SourceSymbol> _sourceSymbols = new List<SourceSymbol>();
		private List<SourceFileInfo> _sourceFiles = new List<SourceFileInfo>();

		private HashSet<int> _usedFileIds = new HashSet<int>();
		private HashSet<string> _usedLabels = new HashSet<string>();
		private Dictionary<int, CodeLabel> _ramLabels = new Dictionary<int, CodeLabel>();
		private Dictionary<int, CodeLabel> _workRamLabels = new Dictionary<int, CodeLabel>();
		private Dictionary<int, CodeLabel> _saveRamLabels = new Dictionary<int, CodeLabel>();
		private Dictionary<int, CodeLabel> _spcRamLabels = new Dictionary<int, CodeLabel>();
		private Dictionary<int, CodeLabel> _romLabels = new Dictionary<int, CodeLabel>();

		private HashSet<string> _filesNotFound = new HashSet<string>();
		private int _errorCount = 0;

		private static Regex _segmentRegex = new Regex("^seg\tid=([0-9]+),.*start=0x([0-9a-fA-F]+),.*size=0x([0-9A-Fa-f]+)", RegexOptions.Compiled);
		private static Regex _segmentPrgRomRegex = new Regex("^seg\tid=([0-9]+),.*start=0x([0-9a-fA-F]+),.*size=0x([0-9A-Fa-f]+),.*ooffs=([0-9]+)", RegexOptions.Compiled);
		private static Regex _lineRegex = new Regex("^line\tid=([0-9]+),.*file=([0-9]+),.*line=([0-9]+)(,.*type=([0-9]+)){0,1}(,.*span=([0-9+]+)){0,1}", RegexOptions.Compiled);
		private static Regex _fileRegex = new Regex("^file\tid=([0-9]+),.*name=\"([^\"]+)\"", RegexOptions.Compiled);
		private static Regex _spanRegex = new Regex("^span\tid=([0-9]+),.*seg=([0-9]+),.*start=([0-9]+),.*size=([0-9]+)(,.*type=([0-9]+)){0,1}", RegexOptions.Compiled);
		private static Regex _scopeRegex = new Regex("^scope\tid=([0-9]+),.*name=\"([0-9a-zA-Z@_-]+)\"(,.*sym=([0-9+]+)){0,1}", RegexOptions.Compiled);
		private static Regex _symbolRegex = new Regex("^sym\tid=([0-9]+),.*name=\"([0-9a-zA-Z@_-]+)\"(,.*size=([0-9]+)){0,1}(,.*def=([0-9+]+)){0,1}(,.*ref=([0-9+]+)){0,1}(,.*val=0x([0-9a-fA-F]+)){0,1}(,.*seg=([0-9]+)){0,1}(,.*exp=([0-9]+)){0,1}", RegexOptions.Compiled);
		private static Regex _cSymbolRegex = new Regex("^csym\tid=([0-9]+),.*name=\"([0-9a-zA-Z@_-]+)\"(,.*sym=([0-9+]+)){0,1}", RegexOptions.Compiled);

		private static Regex _asmFirstLineRegex = new Regex(";(.*)", RegexOptions.Compiled);
		private static Regex _asmPreviousLinesRegex = new Regex("^\\s*;(.*)", RegexOptions.Compiled);
		private static Regex _cFirstLineRegex = new Regex("(.*)", RegexOptions.Compiled);
		private static Regex _cPreviousLinesRegex = new Regex("^\\s*//(.*)", RegexOptions.Compiled);

		private Dictionary<int, SourceCodeLocation> _linesByPrgAddress = new Dictionary<int, SourceCodeLocation>();
		private Dictionary<int, LineInfo[]> _linesByFile = new Dictionary<int, LineInfo[]>();
		private Dictionary<string, int> _prgAddressByLine = new Dictionary<string, int>();

		private Dictionary<int, ScopeInfo> _scopesBySymbol = new Dictionary<int, ScopeInfo>();

		public DateTime SymbolFileStamp { get; private set; }
		public string SymbolPath { get; private set; }

		public List<SourceFileInfo> SourceFiles { get { return _sourceFiles; } }

		public int GetPrgAddress(SourceFileInfo file, int lineIndex)
		{
			return GetPrgAddress((file.InternalFile as FileInfo).ID, lineIndex);
		}

		private int GetPrgAddress(int fileID, int lineIndex)
		{
			int prgAddress;
			if(_prgAddressByLine.TryGetValue(fileID.ToString() + "_" + lineIndex.ToString(), out prgAddress)) {
				return prgAddress;
			}
			return -1;
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

		public SourceCodeLocation GetSourceCodeLineInfo(int prgRomAddress)
		{
			SourceCodeLocation line;
			if(_linesByPrgAddress.TryGetValue(prgRomAddress, out line)) {
				return line;
			}
			return null;
		}

		public string GetSourceCodeLine(int prgRomAddress)
		{
			if(prgRomAddress >= 0) {
				try {
					SourceCodeLocation line;
					if(_linesByPrgAddress.TryGetValue(prgRomAddress, out line)) {
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

		private SourceCodeLocation GetReferenceInfo(int referenceId)
		{
			FileInfo file;
			LineInfo line;
			if(_lines.TryGetValue(referenceId, out line)) {
				string lineContent = "";
				if(_files.TryGetValue(line.FileID, out file) && file.Data != null && file.Data.Length > line.LineNumber) {
					lineContent = file.Data[line.LineNumber];
				}

				return new SourceCodeLocation() {
					File = _files[line.FileID].SourceFile,
					LineNumber = line.LineNumber
				};
			}

			return null;
		}

		public SourceCodeLocation GetSymbolDefinition(SourceSymbol symbol)
		{
			foreach(int definition in (symbol.InternalSymbol as SymbolInfo).Definitions) {
				SourceCodeLocation refInfo = GetReferenceInfo(definition);

				if(refInfo != null) {
					return refInfo;
				}
			}

			return null;
		}

		private List<SourceCodeLocation> GetSymbolReferences(SymbolInfo symbol)
		{
			List<SourceCodeLocation> references = new List<SourceCodeLocation>();
			foreach(int reference in symbol.References) {
				SourceCodeLocation refInfo = GetReferenceInfo(reference);
				if(refInfo != null) {
					references.Add(refInfo);
				}
			}
			return references;
		}

		private SpanInfo GetSymbolDefinitionSpan(SymbolInfo symbol)
		{
			foreach(int definition in symbol.Definitions) {
				LineInfo definitionLine;
				FileInfo file;
				if(_lines.TryGetValue(definition, out definitionLine)) {
					if(_files.TryGetValue(definitionLine.FileID, out file)) {
						int lineNumber = definitionLine.LineNumber;
						while(!(definitionLine?.SpanIDs.Count > 0) && lineNumber < _linesByFile[file.ID].Length - 1) {
							//Definition line contains no code, try the next line
							lineNumber++;
							definitionLine = _linesByFile[file.ID][lineNumber];
						}

						if(definitionLine != null && definitionLine.SpanIDs.Count > 0) {
							SpanInfo span;
							if(_spans.TryGetValue(definitionLine.SpanIDs[0], out span)) {
								return span;
							}
						}
					}
				}
			}
			return null;
		}

		private SymbolInfo GetMatchingSymbol(SymbolInfo symbol, int rangeStart, int rangeEnd)
		{
			AddressInfo? symbolAddress = GetSymbolAddressInfo(symbol);
			if(symbolAddress != null && symbolAddress.Value.Type == SnesMemoryType.PrgRom && symbolAddress.Value.Address >= rangeStart && symbolAddress.Value.Address <= rangeEnd) {
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

		private List<SymbolInfo> GetSymbols()
		{
			return _symbols.Values.ToList();
		}

		public SourceSymbol GetSymbol(string word, int prgStartAddress, int prgEndAddress)
		{
			try {
				foreach(CSymbolInfo symbol in _cSymbols.Values) {
					if(symbol.Name == word && symbol.SymbolID.HasValue) {
						SymbolInfo matchingSymbol = GetMatchingSymbol(_symbols[symbol.SymbolID.Value], prgStartAddress, prgEndAddress);
						if(matchingSymbol != null) {
							return matchingSymbol.SourceSymbol;
						}
					}
				}

				foreach(SymbolInfo symbol in _symbols.Values) {
					if(symbol.Name == word) {
						SymbolInfo matchingSymbol = GetMatchingSymbol(symbol, prgStartAddress, prgEndAddress);
						if(matchingSymbol != null) {
							return matchingSymbol.SourceSymbol;
						}
					}
				}
			} catch { }

			return null;
		}

		public AddressInfo? GetSymbolAddressInfo(SourceSymbol symbol)
		{
			return GetSymbolAddressInfo(symbol.InternalSymbol as SymbolInfo);
		}

		private AddressInfo? GetSymbolAddressInfo(SymbolInfo symbol)
		{
			if(symbol.SegmentID == null || symbol.Address == null) {
				return null;
			}

			SegmentInfo segment = _segments[symbol.SegmentID.Value];
			if(segment.IsRam) {
				if(segment.IsSpc) {
					return new AddressInfo() { Address = symbol.Address.Value, Type = SnesMemoryType.SpcRam };
				} else {
					int labelAddress;
					SnesMemoryType? addressType;
					GetRamLabelAddressAndType(symbol.Address.Value, out labelAddress, out addressType);
					if(addressType.HasValue) {
						return new AddressInfo() { Address = labelAddress, Type = addressType.Value };
					} else {
						return null;
					}
				}
			} else {
				return new AddressInfo() { Address = symbol.Address.Value - segment.Start + segment.FileOffset - _headerSize, Type = SnesMemoryType.PrgRom };
			}
		}

		private bool LoadSegments(string row)
		{
			Match match = _segmentRegex.Match(row);
			if(match.Success) {
				SegmentInfo segment = new SegmentInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					Start = Int32.Parse(match.Groups[2].Value, NumberStyles.HexNumber),
					Size = Int32.Parse(match.Groups[3].Value, NumberStyles.HexNumber),
					IsRam = true
				};

				match = _segmentPrgRomRegex.Match(row);
				if(match.Success) {
					segment.FileOffset = Int32.Parse(match.Groups[4].Value);
					segment.IsRam = false;

					if(row.Contains("type=rw")) {
						//Assume a RW segment inside the .sfc file is SPC code
						segment.IsRam = true;
						segment.IsSpc = true;
					}
				}

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

				FileInfo file = new FileInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					Name = filename,
					IsAssembly = isAsm
				};

				file.SourceFile = new SourceFileInfo() { Name = file.Name, InternalFile = file };
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
			Match match = _lineRegex.Match(row);
			if(match.Success) {
				LineInfo line = new LineInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					FileID = Int32.Parse(match.Groups[2].Value),
					LineNumber = Int32.Parse(match.Groups[3].Value) - 1,
					Type = match.Groups[5].Success ? (LineType)Int32.Parse(match.Groups[5].Value) : LineType.Assembly,
				};

				line.Location = new SourceCodeLocation() {
					File = _files[line.FileID].SourceFile,
					LineNumber = line.LineNumber
				};

				if(line.LineNumber < 0) {
					line.LineNumber = 0;
				}

				if(match.Groups[7].Success) {
					string[] spanIDs = match.Groups[7].Value.Split('+');
					line.SpanIDs = new List<int>(spanIDs.Length);
					for(int i = spanIDs.Length - 1; i >= 0; i--) {
						//Read them backwards to get them in order
						line.SpanIDs.Add(Int32.Parse(spanIDs[i]));
					}
				} else {
					line.SpanIDs = new List<int>();
				}

				_usedFileIds.Add(line.FileID);
				_lines.Add(line.ID, line);
				return true;
			} else if(row.StartsWith("line")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match line");
			}

			return false;
		}

		private bool LoadSpans(string row)
		{
			Match match = _spanRegex.Match(row);
			if(match.Success) {
				SpanInfo span = new SpanInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					SegmentID = Int32.Parse(match.Groups[2].Value),
					Offset = Int32.Parse(match.Groups[3].Value),
					Size = Int32.Parse(match.Groups[4].Value)
				};
				span.IsData = match.Groups[6].Success;

				_spans.Add(span.ID, span);
				return true;
			} else if(row.StartsWith("span")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match span");
			}

			return false;
		}

		private bool LoadCSymbols(string row)
		{
			Match match = _cSymbolRegex.Match(row);
			if(match.Success) {
				CSymbolInfo span = new CSymbolInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					Name = match.Groups[2].Value,
					SymbolID = match.Groups[4].Success ? (int?)Int32.Parse(match.Groups[4].Value) : null,
				};
				_cSymbols.Add(span.ID, span);
				return true;
			} else if(row.StartsWith("csym")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match csym");
			}

			return false;
		}

		private bool LoadScopes(string row)
		{
			Match match = _scopeRegex.Match(row);
			if(match.Success) {
				ScopeInfo scope = new ScopeInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					Name = match.Groups[2].Value,
					SymbolID = match.Groups[4].Success ? (int?)Int32.Parse(match.Groups[4].Value) : null,
				};

				if(scope.SymbolID.HasValue) {
					_scopesBySymbol[scope.SymbolID.Value] = scope;
				}

				_scopes.Add(scope.ID, scope);
				return true;
			} /*else if(row.StartsWith("scope")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match scope");
			}*/

			return false;
		}

		private bool LoadSymbols(string row)
		{
			Match match = _symbolRegex.Match(row);
			if(match.Success) {
				SymbolInfo symbol = new SymbolInfo() {
					ID = Int32.Parse(match.Groups[1].Value),
					Name = match.Groups[2].Value,
					Address = match.Groups[10].Success ? (int?)Int32.Parse(match.Groups[10].Value, NumberStyles.HexNumber) : null,
					SegmentID = match.Groups[12].Success ? (int?)Int32.Parse(match.Groups[12].Value) : null,
					ExportSymbolID = match.Groups[14].Success ? (int?)Int32.Parse(match.Groups[14].Value) : null
				};

				if(match.Groups[4].Success) {
					symbol.Size = Int32.Parse(match.Groups[4].Value);
				}

				if(match.Groups[6].Success) {
					symbol.Definitions = match.Groups[6].Value.Split('+').Select(o => Int32.Parse(o)).ToList();
				} else {
					symbol.Definitions = new List<int>();
				}

				if(match.Groups[8].Success) {
					symbol.References = match.Groups[8].Value.Split('+').Select(o => Int32.Parse(o)).ToList();
				} else {
					symbol.References = new List<int>();
				}

				symbol.SourceSymbol = new SourceSymbol() { Name = symbol.Name, InternalSymbol = symbol, Address = symbol.Address };

				_symbols.Add(symbol.ID, symbol);
				return true;
			} else if(row.StartsWith("sym")) {
				System.Diagnostics.Debug.Fail("Regex doesn't match sym");
			}

			return false;
		}

		private int GetSymbolSize(SymbolInfo symbol)
		{
			if(symbol.SegmentID != null && _segments.ContainsKey(symbol.SegmentID.Value)) {
				SegmentInfo segment = _segments[symbol.SegmentID.Value];
				SpanInfo defSpan = null;
				if(!segment.IsRam) {
					defSpan = GetSymbolDefinitionSpan(symbol);
				}

				ScopeInfo scope = null;
				if(_scopesBySymbol.TryGetValue(symbol.ID, out scope)) {
					//This symbol actually denotes the start of a scope (.scope or .proc) and isn't actually data, return a size of 1
					return 1;
				} else {
					return (defSpan == null || defSpan.IsData) ? (symbol.Size ?? 1) : 1;
				}
			}

			return 1;
		}

		private void GetRamLabelAddressAndType(int address, out int absoluteAddress, out SnesMemoryType? memoryType)
		{
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = address, Type = SnesMemoryType.CpuMemory });
			absoluteAddress = absAddress.Address;
			if(absoluteAddress >= 0) {
				memoryType = absAddress.Type;
			} else {
				memoryType = null;
			}
		}

		private CodeLabel CreateLabel(Int32 address, SnesMemoryType memoryType, UInt32 length)
		{
			CodeLabel label = null;
			if(memoryType == SnesMemoryType.WorkRam) {
				if(!_workRamLabels.TryGetValue(address, out label)) {
					label = new CodeLabel() { Address = (UInt32)address, MemoryType = SnesMemoryType.WorkRam, Comment = string.Empty, Label = string.Empty, Length = length };
					_workRamLabels[address] = label;
				}
			} else if(memoryType == SnesMemoryType.SaveRam) {
				if(!_saveRamLabels.TryGetValue(address, out label)) {
					label = new CodeLabel() { Address = (UInt32)address, MemoryType = SnesMemoryType.SaveRam, Comment = string.Empty, Label = string.Empty, Length = length };
					_saveRamLabels[address] = label;
				}
			} else if(memoryType == SnesMemoryType.SpcRam) {
				if(!_spcRamLabels.TryGetValue(address, out label)) {
					label = new CodeLabel() { Address = (UInt32)address, MemoryType = SnesMemoryType.SpcRam, Comment = string.Empty, Label = string.Empty, Length = length };
					_spcRamLabels[address] = label;
				}
			} else {
				if(!_romLabels.TryGetValue(address, out label)) {
					label = new CodeLabel() { Address = (UInt32)address, MemoryType = SnesMemoryType.PrgRom, Comment = string.Empty, Label = string.Empty, Length = length };
					_romLabels[address] = label;
				}
			}

			return label;
		}

		private void LoadLabels()
		{
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
						if(symbol.Address != null && symbol.Address >= 0) {
							CodeLabel label = this.CreateLabel(addressInfo.Value.Address, addressInfo.Value.Type, (uint)GetSymbolSize(symbol));
							if(label != null) {
								label.Label = newName;
							}
						}
					}
				} catch {
					_errorCount++;
				}
			}
		}

		private void LoadComments()
		{
			DbgIntegrationConfig config = ConfigManager.Config.Debug.DbgIntegration;
			foreach(KeyValuePair<int, LineInfo> kvp in _lines) {
				try {
					LineInfo line = kvp.Value;
					if(line.SpanIDs.Count == 0) {
						continue;
					}

					SpanInfo span = _spans[line.SpanIDs[0]];
					SegmentInfo segment = _segments[span.SegmentID];

					if(_files[line.FileID].Data == null) {
						//File was not found.
						if(_filesNotFound.Add(_files[line.FileID].Name)) {
							_errorCount++;
						}
						continue;
					}

					bool isAsm = _files[line.FileID].IsAssembly;

					string comment = "";
					for(int i = line.LineNumber; i >= 0; i--) {
						string sourceCodeLine = _files[line.FileID].Data[i];
						if(sourceCodeLine.Trim().Length == 0) {
							//Ignore empty lines
							continue;
						}

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
						SnesMemoryType? memoryType;
						if(segment.IsRam) {
							if(segment.IsSpc) {
								address = span.Offset + segment.Start;
								memoryType = SnesMemoryType.SpcRam;
							} else {
								GetRamLabelAddressAndType(span.Offset + segment.Start, out address, out memoryType);
							}
						} else {
							address = GetPrgAddress(span);
							memoryType = SnesMemoryType.PrgRom;
						}

						if(memoryType == SnesMemoryType.SpcRam && !config.ImportSpcComments) {
							continue;
						} else if(memoryType != SnesMemoryType.SpcRam && !config.ImportCpuComments) {
							continue;
						}

						if(address >= 0 && memoryType != null) {
							CodeLabel label = this.CreateLabel(address, memoryType.Value, 1);
							if(label != null) {
								label.Comment = comment;
							}
						}
					}
				} catch {
					_errorCount++;
				}
			}
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

			foreach(FileInfo file in _files.Values) {
				if(_usedFileIds.Contains(file.ID)) {
					try {
						string basePath = path;
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
							file.Data = File.ReadAllLines(sourceFile);
							file.SourceFile.Data = file.Data;
						}

						LineInfo[] fileInfos = new LineInfo[maxLineCountByFile[file.ID] + 1];
						foreach(LineInfo line in _lines.Values) {
							if(line.FileID == file.ID) {
								fileInfos[line.LineNumber] = line;
							}
						}
						_linesByFile[file.ID] = fileInfos;
					} catch {
						_errorCount++;
					}
				}
			}
		}

		private void BuildCdlData()
		{
			int prgSize = DebugApi.GetMemorySize(SnesMemoryType.PrgRom);
			if(prgSize <= 0) {
				return;
			}

			byte[] cdlFile = new byte[prgSize];

			//Mark data/code regions
			foreach(SpanInfo span in _spans.Values) {
				int prgAddress = GetPrgAddress(span);
				if(prgAddress >= 0 && prgAddress < prgSize) {
					for(int i = 0; i < span.Size; i++) {
						if(cdlFile[prgAddress + i] != (byte)CdlFlags.Data && !span.IsData && span.Size <= 4) {
							cdlFile[prgAddress + i] = (byte)CdlFlags.Code;
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
			byte[] prgRomContent = DebugApi.GetMemoryState(SnesMemoryType.PrgRom);
			foreach(SymbolInfo symbol in _symbols.Values) {
				LineInfo line;
				if(!symbol.SegmentID.HasValue) {
					//This is a constant, ignore it
					continue;
				}

				foreach(int reference in symbol.References) {
					if(_lines.TryGetValue(reference, out line) && line.SpanIDs.Count > 0) {
						SpanInfo span;
						if(_spans.TryGetValue(line.SpanIDs[0], out span) && !span.IsData && span.Size <= 3) {
							int referencePrgAddr = GetPrgAddress(span);
							if(referencePrgAddr >= 0 && referencePrgAddr < prgRomContent.Length) {
								byte opCode = prgRomContent[referencePrgAddr];
								if(opCode == 0x20 || opCode == 0x10 || opCode == 0x30 || opCode == 0x50 || opCode == 0x70 || opCode == 0x80 || opCode == 0x90 || opCode == 0xB0 || opCode == 0xD0 || opCode == 0xF0 || opCode == 0x4C || opCode == 0x20 || opCode == 0x4C || opCode == 0x5C || opCode == 0x6C) {
									//This symbol is used with a JSR/jump instruction, so it's either a function or jump target
									bool isJsr = opCode == 0x20 || opCode == 0x22; //JSR/JSL
									SpanInfo definitionSpan = GetSymbolDefinitionSpan(symbol);
									if(definitionSpan != null) {
										int definitionPrgAddr = GetPrgAddress(definitionSpan);
										if(definitionPrgAddr >= 0 && definitionPrgAddr < prgRomContent.Length) {
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

			DebugApi.SetCdlData(cdlFile, cdlFile.Length);
		}

		public void Import(string path, bool silent = false)
		{
			SymbolFileStamp = File.GetLastWriteTime(path);
			string[] fileRows = File.ReadAllLines(path);

			string basePath = Path.GetDirectoryName(path);
			SymbolPath = basePath;
			foreach(string row in fileRows) {
				try {
					if(LoadLines(row) || LoadSpans(row) || LoadSymbols(row) || LoadCSymbols(row) || LoadScopes(row) || LoadFiles(row, basePath) || LoadSegments(row)) {
						continue;
					}
				} catch {
					_errorCount++;
				}
			}

			LoadFileData(basePath);

			BuildCdlData();

			foreach(LineInfo line in _lines.Values) {
				foreach(int spanID in line.SpanIDs) {
					SpanInfo span;
					if(_spans.TryGetValue(spanID, out span)) {
						SegmentInfo segment;
						if(_segments.TryGetValue(span.SegmentID, out segment) && !segment.IsRam) {
							for(int i = 0; i < span.Size; i++) {
								int prgAddress = segment.FileOffset - _headerSize + span.Offset + i;

								/*SourceCodeLocation existingLine;
								if(_linesByPrgAddress.TryGetValue(prgAddress, out existingLine) && existingLine.Type == LineType.External) {
									//Give priority to lines that come from C files
									continue;
								}*/

								_linesByPrgAddress[prgAddress] = line.Location;
								if(i == 0 && spanID == line.SpanIDs[0]) {
									//Mark the first byte of the first span representing this line as the PRG address for this line of code
									FileInfo file = _files[line.FileID];
									_prgAddressByLine[file.ID.ToString() + "_" + line.LineNumber.ToString()] = prgAddress;
								}
							}
						}
					}
				}
			}

			LoadLabels();

			int labelCount = 0;

			DbgIntegrationConfig config = ConfigManager.Config.Debug.DbgIntegration;
			if(config.ImportCpuComments || config.ImportSpcComments) {
				LoadComments();
			}
			List<CodeLabel> labels = new List<CodeLabel>(_romLabels.Count + _ramLabels.Count + _workRamLabels.Count + _saveRamLabels.Count);
			if(config.ImportCpuPrgRomLabels) {
				labels.AddRange(_romLabels.Values);
				labelCount += _romLabels.Count;
			}
			if(config.ImportCpuWorkRamLabels) {
				labels.AddRange(_workRamLabels.Values);
				labelCount += _workRamLabels.Count;
			}
			if(config.ImportCpuSaveRamLabels) {
				labels.AddRange(_saveRamLabels.Values);
				labelCount += _saveRamLabels.Count;
			}
			if(config.ImportSpcRamLabels) {
				labels.AddRange(_spcRamLabels.Values);
				labelCount += _spcRamLabels.Count;
			}

			if(ConfigManager.Config.Debug.DbgIntegration.ResetLabelsOnImport) {
				DebugWorkspaceManager.ResetLabels();
			}
			LabelManager.SetLabels(labels, true);

			if(!silent) {
				if(_errorCount > 0) {
					_errorCount -= _filesNotFound.Count;
					string message = $"Import completed with {labelCount} labels imported";
					if(_errorCount > 0) {
						message += $"and {_errorCount} errors - please file a bug report and attach the DBG file you tried to import.";
					}
					if(_filesNotFound.Count > 0) {
						message += Environment.NewLine + Environment.NewLine + "The following files could not be found:";
						foreach(string file in _filesNotFound) {
							message += Environment.NewLine + file;
						}
					}
					MessageBox.Show(message, "Mesen-S", MessageBoxButtons.OK, MessageBoxIcon.Warning);
				} else {
					MessageBox.Show($"Import completed with {labelCount} labels imported.", "Mesen-S", MessageBoxButtons.OK, MessageBoxIcon.Information);
				}
			}
		}

		private class SegmentInfo
		{
			public int ID;
			public int Start;
			public int Size;
			public int FileOffset;
			public bool IsRam;
			public bool IsSpc;
		}

		private class FileInfo
		{
			public int ID;
			public string Name;
			public string[] Data;
			public bool IsAssembly;
			public SourceFileInfo SourceFile;
		}

		private class LineInfo
		{
			public int ID;
			public int FileID;
			public List<int> SpanIDs;
			public LineType Type;

			public int LineNumber;

			public SourceCodeLocation Location;
		}

		private enum LineType
		{
			Assembly = 0,
			External = 1, //i.e C source file
			Macro = 2
		}

		private class SpanInfo
		{
			public int ID;
			public int SegmentID;
			public int Offset;
			public int Size;
			public bool IsData;
		}

		private class SymbolInfo
		{
			public int ID;
			public string Name;
			public int? Address;
			public int? SegmentID;
			public int? ExportSymbolID;
			public int? Size;
			public List<int> References;
			public List<int> Definitions;

			public SourceSymbol SourceSymbol;
		}

		private class ScopeInfo
		{
			public int ID;
			public string Name;
			public int? SymbolID;
		}

		private class CSymbolInfo
		{
			public int ID;
			public string Name;
			public int? SymbolID;
		}
	}
}
