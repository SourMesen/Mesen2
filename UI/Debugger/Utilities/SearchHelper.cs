using Avalonia.Media.Imaging;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Labels;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public static class SearchHelper
	{
		private static List<string> GetSearchTerms(string searchString)
		{
			searchString = searchString.Trim();

			HashSet<string> terms = new HashSet<string>();
			terms.Add(searchString.ToLower());
			
			//Add each word as a separate search term
			terms.UnionWith(searchString.ToLower().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries));
			
			//Add a space in front of each uppercase letter (and add each newly formed word as a search term)
			for(int i = 0; i < searchString.Length; i++) {
				char ch = searchString[i];
				if(ch >= 'A' && ch <= 'Z') {
					searchString = searchString.Insert(i, " ");
					i++;
				}
			}
			terms.UnionWith(searchString.ToLower().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries));

			List<string> results = terms.Where(x => x.Length > 1).ToList();
			if(results.Count == 0) {
				results = terms.ToList();
			}
			return results;
		}

		private static bool IsMatch(string label, List<string> searchTerms)
		{
			label = label.ToLower();
			if(searchTerms.Count == 1) {
				return label.Contains(searchTerms[0]);
			} else {
				for(int i = 1; i < searchTerms.Count; i++) {
					if(!label.Contains(searchTerms[i])) {
						return false;
					}
				}
				return true;
			}
		}

		public static List<SearchResultInfo> GetGoToAllResults(CpuType cpuType, string searchString, GoToAllOptions options, ISymbolProvider? symbolProvider = null)
		{
			List<string> searchTerms = GetSearchTerms(searchString);

			bool showFilesAndConstants = options.HasFlag(GoToAllOptions.ShowFilesAndConstants);
			bool showOutOfScope = options.HasFlag(GoToAllOptions.ShowOutOfScope);

			MemoryType? prgMemType = null;
			CdlFlags[]? cdlData = null;
			if(cpuType.ToMemoryType().SupportsCdl()) {
				prgMemType = cpuType.GetPrgRomMemoryType();
				int size = DebugApi.GetMemorySize(prgMemType.Value);
				cdlData = DebugApi.GetCdlData(0, (uint)size, prgMemType.Value);
			}

			List<SearchResultInfo> results = new List<SearchResultInfo>();
			bool isEmptySearch = string.IsNullOrWhiteSpace(searchString);
			if(!isEmptySearch) {
				if(symbolProvider != null) {
					if(showFilesAndConstants) {
						foreach(SourceFileInfo file in symbolProvider.SourceFiles) {
							if(IsMatch(file.Name, searchTerms)) {
								results.Add(new SearchResultInfo() {
									Caption = Path.GetFileName(file.Name),
									AbsoluteAddress = null,
									SearchResultType = SearchResultType.File,
									File = file,
									SourceLocation = null,
									RelativeAddress = null,
									CodeLabel = null
								});
							}
						}
					}

					foreach(SourceSymbol symbol in symbolProvider.GetSymbols()) {
						if(IsMatch(symbol.Name, searchTerms)) {
							SourceCodeLocation? def = symbolProvider.GetSymbolDefinition(symbol);
							AddressInfo? absAddr = symbolProvider.GetSymbolAddressInfo(symbol);
							int value = 0;
							AddressInfo? relAddr = null;
							bool isConstant = absAddr == null;
							if(!showFilesAndConstants && isConstant) {
								continue;
							}

							if(absAddr != null) {
								value = DebugApi.GetMemoryValue(absAddr.Value.Type, (uint)absAddr.Value.Address);
								relAddr = DebugApi.GetRelativeAddress(absAddr.Value, cpuType);
							} else {
								//For constants, the address field contains the constant's value
								value = symbol.Address ?? 0;
							}

							SearchResultType resultType = SearchResultType.Data;
							if(isConstant) {
								resultType = SearchResultType.Constant;
							} else if(cdlData != null && prgMemType != null && absAddr?.Type == prgMemType && absAddr.Value.Address < cdlData.Length) {
								CdlFlags flags = cdlData[absAddr.Value.Address];
								if(flags.HasFlag(CdlFlags.SubEntryPoint)) {
									resultType = SearchResultType.Function;
								} else if(flags.HasFlag(CdlFlags.JumpTarget)) {
									resultType = SearchResultType.JumpTarget;
								}
							}

							results.Add(new SearchResultInfo() {
								Caption = symbol.Name,
								AbsoluteAddress = absAddr,
								Length = symbolProvider.GetSymbolSize(symbol),
								SearchResultType = resultType,
								Value = value,
								File = def?.File,
								SourceLocation = def,
								RelativeAddress = relAddr?.Address >= 0 ? relAddr : null,
								CodeLabel = LabelManager.GetLabel(symbol.Name)
							});
						}
					}
				} else {
					foreach(CodeLabel label in LabelManager.GetLabels(cpuType)) {
						if(IsMatch(label.Label, searchTerms)) {
							SearchResultType resultType = SearchResultType.Data;
							AddressInfo absAddr = label.GetAbsoluteAddress();
							if(cdlData != null && absAddr.Type == prgMemType && absAddr.Address < cdlData.Length) {
								CdlFlags flags = cdlData[absAddr.Address];
								if(flags.HasFlag(CdlFlags.SubEntryPoint)) {
									resultType = SearchResultType.Function;
								} else if(flags.HasFlag(CdlFlags.JumpTarget)) {
									resultType = SearchResultType.JumpTarget;
								}
							}

							AddressInfo relAddr = label.GetRelativeAddress(cpuType);
							results.Add(new SearchResultInfo() {
								Caption = label.Label,
								AbsoluteAddress = absAddr,
								Length = (int)label.Length,
								Value = label.GetValue(),
								SearchResultType = resultType,
								File = null,
								Disabled = !showOutOfScope && relAddr.Address < 0,
								RelativeAddress = relAddr.Address >= 0 ? relAddr : null,
								CodeLabel = label
							});
						}
					}
				}
			}

			results.Sort((SearchResultInfo a, SearchResultInfo b) => {
				int comparison = a.Disabled.CompareTo(b.Disabled);

				if(comparison == 0) {
					bool aStartsWithSearch = a.Caption.StartsWith(searchString, StringComparison.InvariantCultureIgnoreCase);
					bool bStartsWithSearch = b.Caption.StartsWith(searchString, StringComparison.InvariantCultureIgnoreCase);

					comparison = bStartsWithSearch.CompareTo(aStartsWithSearch);
					if(comparison == 0) {
						comparison = a.Caption.CompareTo(b.Caption);
					}
				}
				return comparison;
			});

			return results;
		}
	}

	[Flags]
	public enum GoToAllOptions
	{
		None = 0,
		ShowFilesAndConstants = 1,
		ShowOutOfScope = 2
	}

	public enum SearchResultType
	{
		Function,
		JumpTarget,
		Constant,
		Data,
		File
	}

	public record SearchResultInfo
	{
		public string Caption = "";
		public AddressInfo? AbsoluteAddress;
		public AddressInfo? RelativeAddress;
		public int Value;
		public int Length;
		public SourceFileInfo? File;
		public SourceCodeLocation? SourceLocation;
		public SearchResultType SearchResultType;
		public CodeLabel? CodeLabel;
		public bool Disabled { get; set; }
	
		public string UiAbsAddress
		{
			get
			{
				if(AbsoluteAddress?.Address >= 0) {
					return "$" + AbsoluteAddress?.Address.ToString("X4") + ":$" + Value.ToString("X2");
				}
				return "";
			}
		}

		public string UiRelAddress
		{
			get
			{
				if(AbsoluteAddress?.Address >= 0) {
					if(RelativeAddress?.Address >= 0) {
						return "$" + RelativeAddress?.Address.ToString("X" + RelativeAddress?.Type.ToCpuType().GetAddressSize()) + ":$" + Value.ToString("X2");
					} else {
						return "<out of scope>";
					}
				} else if(SearchResultType == SearchResultType.Constant) {
					return "Value: $" + Value.ToString("X2");
				}
				return "";
			}
		}

		public Bitmap? UiIcon
		{
			get
			{
				string? img = null;
				if(AbsoluteAddress?.Address >= 0) {
					if(SearchResultType == SearchResultType.Function) {
						img = "Assets/Function.png";
					} else if(SearchResultType == SearchResultType.JumpTarget) {
						img = "Assets/JumpTarget.png";
					} else {
						img = "Assets/CheatCode.png";
					}
				} else if(SearchResultType == SearchResultType.File) {
					img = "Assets/LogWindow.png";
				} else if(SearchResultType == SearchResultType.Constant) {
					img = "Assets/Enum.png";
				}

				return img != null ? ImageUtilities.BitmapFromAsset(img) : null;
			}
		}

		public string UiLocation
		{
			get
			{
				if(!string.IsNullOrWhiteSpace(File?.Name)) {
					if(SearchResultType == SearchResultType.File) {
						return Path.GetFileName(File.Name);
					} else {
						return Path.GetFileName(File.Name) + ":" + (SourceLocation?.LineNumber + 1).ToString();
					}
				}
				
				return "";
			}
		}

		public string UiLabelName
		{
			get
			{
				string caption = Caption;
				if(Length > 1) {
					if(Length == 2) {
						caption += " (word)";
					} else {
						caption += $" ({Length} bytes)";
					}
				}
				return caption;
			}
		}

		public string UiMemType
		{
			get
			{
				if(AbsoluteAddress?.Address >= 0) {
					MemoryType memType = AbsoluteAddress.Value.Type;
					return ResourceHelper.GetEnumText(memType);
				} else if(SearchResultType == SearchResultType.File) {
					return "File";
				} else if(SearchResultType == SearchResultType.Constant) {
					return "Constant";
				}
				return "";
			}
		}

		public string UiCpuMemType
		{
			get
			{
				return AbsoluteAddress?.Address >= 0 ? "CPU" : "";
			}
		}

		public GoToDestination GetDestination()
		{
			return new GoToDestination() {
				AbsoluteAddress = AbsoluteAddress,
				RelativeAddress = RelativeAddress,
				Label = CodeLabel,
				File = File,
				SourceLocation = SourceLocation
			};
		}
	}


	public class GoToDestination
	{
		public CodeLabel? Label;
		public AddressInfo? AbsoluteAddress;
		public AddressInfo? RelativeAddress;
		public SourceFileInfo? File;
		public SourceCodeLocation? SourceLocation;
	}
}
