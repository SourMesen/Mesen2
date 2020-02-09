using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using Mesen.GUI.Debugger.Labels;
using Mesen.GUI.Debugger.Workspace;
using Mesen.GUI.Forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public partial class frmGoToAll : BaseForm
	{
		private const int MaxResultCount = 30;

		private List<ctrlSearchResult> _results = new List<ctrlSearchResult>();
		private int _selectedResult = 0;
		private int _resultCount = 0;
		private ISymbolProvider _symbolProvider;
		private bool _allowOutOfScope;
		private bool _showFilesAndConstants;

		public GoToDestination Destination { get; private set; }

		public frmGoToAll(bool allowOutOfScope, bool showFilesAndConstants)
		{
			InitializeComponent();

			Icon = Properties.Resources.Find;
			_symbolProvider = DebugWorkspaceManager.GetSymbolProvider();
			_allowOutOfScope = allowOutOfScope;
			_showFilesAndConstants = showFilesAndConstants;

			tlpResults.SuspendLayout();
			for(int i = 0; i < MaxResultCount; i++) {
				ctrlSearchResult searchResult = new ctrlSearchResult();
				searchResult.Dock = DockStyle.Top;
				searchResult.BackColor = i % 2 == 0 ? SystemColors.ControlLight : SystemColors.ControlLightLight;
				searchResult.Visible = false;
				searchResult.Click += SearchResult_Click;
				searchResult.DoubleClick += SearchResult_DoubleClick;
				tlpResults.Controls.Add(searchResult, 0, i);
				tlpResults.RowStyles.Add(new RowStyle(SizeType.AutoSize));

				_results.Add(searchResult);
			}
			tlpResults.ResumeLayout();

			UpdateResults();
		}

		private void SearchResult_Click(object sender, EventArgs e)
		{
			SelectedResult = _results.IndexOf(sender as ctrlSearchResult);
		}

		private void SearchResult_DoubleClick(object sender, EventArgs e)
		{
			SelectedResult = _results.IndexOf(sender as ctrlSearchResult);
			SelectAndClose();
		}

		protected override bool ProcessCmdKey(ref Message msg, Keys keyData)
		{
			if(keyData == Keys.Up) {
				SelectedResult--;
				return true;
			} else if(keyData == Keys.Down) {
				SelectedResult++;
				return true;
			} else if(keyData == Keys.PageUp) {
				SelectedResult -= pnlResults.ClientSize.Height / _results[0].Height;
				return true;
			} else if(keyData == Keys.PageDown) {
				SelectedResult += pnlResults.ClientSize.Height / _results[0].Height;
				return true;
			} else if(keyData == Keys.Enter) {
				SelectAndClose();
			} else if(keyData == Keys.Escape) {
				Close();
			}

			return base.ProcessCmdKey(ref msg, keyData);
		}

		private int SelectedResult
		{
			get { return _selectedResult; }
			set
			{
				//Reset currently highlighted element's color
				_results[_selectedResult].BackColor = _selectedResult % 2 == 0 ? SystemColors.ControlLight : SystemColors.ControlLightLight;

				_selectedResult = Math.Max(0, Math.Min(_resultCount - 1, value));
				if(_resultCount == 0) {
					_results[0].BackColor = SystemColors.ControlLight;
				} else {
					_results[_selectedResult].BackColor = Color.LightBlue;
				}

				if(_resultCount > 0) {
					if(Program.IsMono) {
						//Use this logic to replace ScrollControlIntoView (which doesn't work properly on Mono)
						int startPos = (_results[0].Height + 1) * _selectedResult;
						int endPos = startPos + _results[0].Height + 1;

						int minVisiblePos = pnlResults.VerticalScroll.Value;
						int maxVisiblePos = pnlResults.Height + pnlResults.VerticalScroll.Value;

						if(startPos < minVisiblePos) {
							pnlResults.VerticalScroll.Value = startPos;
						} else if(endPos > maxVisiblePos) {
							pnlResults.VerticalScroll.Value = endPos - pnlResults.Height;
						}
					} else {
						pnlResults.ScrollControlIntoView(_results[_selectedResult]);
					}
				}
			}
		}

		private bool Contains(string label, List<string> searchStrings)
		{
			label = label.ToLower();
			if(searchStrings.Count == 1) {
				return label.Contains(searchStrings[0]);
			} else {
				for(int i = 1; i < searchStrings.Count; i++) {
					if(!label.Contains(searchStrings[i])) {
						return false;
					}
				}
				return true;
			}
		}

		private void UpdateResults()
		{
			string searchString = txtSearch.Text.Trim();

			List<string> searchStrings = new List<string>();
			searchStrings.Add(searchString.ToLower());
			searchStrings.AddRange(searchString.ToLower().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries));
			for(int i = 0; i < searchString.Length; i++) {
				char ch = searchString[i];
				if(ch >= 'A' && ch <= 'Z') {
					searchString = searchString.Remove(i, 1).Insert(i, " " + (char)(ch + 'a' - 'A'));
				}
			}
			searchStrings.AddRange(searchString.ToLower().Split(new char[] { ' ' }, StringSplitOptions.RemoveEmptyEntries));
			searchStrings = searchStrings.Distinct().ToList();

			_resultCount = 0;

			int size = DebugApi.GetMemorySize(SnesMemoryType.PrgRom);
			byte[] cdlData = DebugApi.GetCdlData(0, (uint)size, SnesMemoryType.PrgRom);

			List<SearchResultInfo> searchResults = new List<SearchResultInfo>();
			bool isEmptySearch = string.IsNullOrWhiteSpace(searchString);
			if(!isEmptySearch) {
				if(_symbolProvider != null) {
					if(_showFilesAndConstants) {
						foreach(SourceFileInfo file in _symbolProvider.SourceFiles) {
							if(Contains(file.Name, searchStrings)) {
								searchResults.Add(new SearchResultInfo() {
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

					foreach(SourceSymbol symbol in _symbolProvider.GetSymbols()) {
						if(Contains(symbol.Name, searchStrings)) {
							SourceCodeLocation def = _symbolProvider.GetSymbolDefinition(symbol);
							AddressInfo? addressInfo = _symbolProvider.GetSymbolAddressInfo(symbol);
							int value = 0;
							AddressInfo? relAddress = null;
							bool isConstant = addressInfo == null;
							if(!_showFilesAndConstants && isConstant) {
								continue;
							}

							if(addressInfo != null) {
								value = DebugApi.GetMemoryValue(addressInfo.Value.Type, (uint)addressInfo.Value.Address);
								relAddress = DebugApi.GetRelativeAddress(addressInfo.Value);
							} else {
								//For constants, the address field contains the constant's value
								value = symbol.Address ?? 0;
							}

							SearchResultType resultType = SearchResultType.Data;
							if(isConstant) {
								resultType = SearchResultType.Constant;
							} else if(addressInfo?.Type == SnesMemoryType.PrgRom && addressInfo.Value.Address < cdlData.Length) {
								if((cdlData[addressInfo.Value.Address] & (byte)CdlFlags.JumpTarget) != 0) {
									resultType = SearchResultType.JumpTarget;
								} else if((cdlData[addressInfo.Value.Address] & (byte)CdlFlags.SubEntryPoint) != 0) {
									resultType = SearchResultType.Function;
								}
							}

							searchResults.Add(new SearchResultInfo() {
								Caption = symbol.Name,
								AbsoluteAddress = addressInfo,
								Length = _symbolProvider.GetSymbolSize(symbol),
								SearchResultType = resultType,
								Value = value,
								File = def?.File,
								SourceLocation = def,
								RelativeAddress = relAddress,
								CodeLabel = LabelManager.GetLabel(symbol.Name)
							});
						}
					}
				} else {
					foreach(CodeLabel label in LabelManager.GetLabels(CpuType.Cpu)) { //TODO
						if(Contains(label.Label, searchStrings)) {
							SearchResultType resultType = SearchResultType.Data;
							AddressInfo addressInfo = label.GetAbsoluteAddress();
							if(addressInfo.Type == SnesMemoryType.PrgRom && addressInfo.Address < cdlData.Length) {
								if((cdlData[addressInfo.Address] & (byte)CdlFlags.JumpTarget) != 0) {
									resultType = SearchResultType.JumpTarget;
								} else if((cdlData[addressInfo.Address] & (byte)CdlFlags.SubEntryPoint) != 0) {
									resultType = SearchResultType.Function;
								}
							}

							AddressInfo relAddress = label.GetRelativeAddress();
							searchResults.Add(new SearchResultInfo() {
								Caption = label.Label,
								AbsoluteAddress = label.GetAbsoluteAddress(),
								Length = (int)label.Length,
								Value = label.GetValue(),
								SearchResultType = resultType,
								File = null,
								Disabled = !_allowOutOfScope && relAddress.Address < 0,
								RelativeAddress = relAddress,
								CodeLabel = label
							});
						}
					}
				}
			}

			searchResults.Sort((SearchResultInfo a, SearchResultInfo b) => {
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

			_resultCount = Math.Min(searchResults.Count, MaxResultCount);
			SelectedResult = 0;

			lblResultCount.Visible = !isEmptySearch;
			lblResultCount.Text = searchResults.Count.ToString() + (searchResults.Count == 1 ? " result" : " results");
			if(searchResults.Count > MaxResultCount) {
				lblResultCount.Text += " (" + MaxResultCount.ToString() + " shown)";
			}

			if(searchResults.Count == 0 && !isEmptySearch) {
				_resultCount++;
				searchResults.Add(new SearchResultInfo() { Caption = "No results found." });
				pnlResults.BackColor = SystemColors.ControlLight;
			} else {
				pnlResults.BackColor = SystemColors.ControlDarkDark;
			}

			if(Program.IsMono) {
				pnlResults.Visible = false;
			} else {
				//Suspend layout causes a crash on Mono
				tlpResults.SuspendLayout();
			}

			for(int i = 0; i < _resultCount; i++) {
				_results[i].Initialize(searchResults[i]);
				_results[i].Tag = searchResults[i];
				_results[i].Visible = true;
			}

			for(int i = _resultCount; i < MaxResultCount; i++) {
				_results[i].Visible = false;
			}

			pnlResults.VerticalScroll.Value = 0;
			tlpResults.Height = (_results[0].Height + 1) * _resultCount;
			
			pnlResults.ResumeLayout();
			if(Program.IsMono) {
				pnlResults.Visible = true;
				tlpResults.Width = pnlResults.ClientSize.Width - 17;
			} else {
				tlpResults.ResumeLayout();
				tlpResults.Width = pnlResults.ClientSize.Width - 1;
			}
		}

		private void txtSearch_TextChanged(object sender, EventArgs e)
		{
			UpdateResults();
		}

		private void SelectAndClose()
		{
			if(_resultCount > 0) {
				SearchResultInfo searchResult = _results[_selectedResult].Tag as SearchResultInfo;
				if(!searchResult.Disabled) {
					Destination = new GoToDestination() {
						AbsoluteAddress = searchResult.AbsoluteAddress,
						RelativeAddress = searchResult.RelativeAddress,
						Label = searchResult.CodeLabel,
						File = searchResult.File,
						SourceLocation = searchResult.SourceLocation
					};
					DialogResult = DialogResult.OK;
					Close();
				}
			}
		}
	}

	public delegate void GoToDestinationEventHandler(GoToDestination dest);

	public class GoToDestination
	{
		public CodeLabel Label;
		public AddressInfo? AbsoluteAddress;
		public AddressInfo? RelativeAddress;
		public SourceFileInfo File;
		public SourceCodeLocation SourceLocation;
	}
}
