using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Config;
using System.IO;
using System.Xml.Serialization;
using Mesen.Utilities;
using System.Reflection;
using Avalonia.Controls.Selection;
using ReactiveUI;
using Mesen.Interop;
using Avalonia.Controls;
using System.IO.Compression;
using System.Text.Json;

namespace Mesen.ViewModels
{
	public class CheatDatabaseViewModel : DisposableViewModel
	{
		private List<CheatDbGameEntry> _entries;

		[Reactive] public IEnumerable<CheatDbGameEntry> FilteredEntries { get; set; }
		[Reactive] public SelectionModel<CheatDbGameEntry?> SelectionModel { get; set; } = new();
		[Reactive] public string SearchString { get; set; } = "";

		[Obsolete("For designer only")]
		public CheatDatabaseViewModel() : this(ConsoleType.Snes) { }

		public CheatDatabaseViewModel(ConsoleType consoleType)
		{
			CheatDatabase cheatDb = new();
			try {
				using Stream? depStream = Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Dependencies.zip");
				if(depStream != null) {
					using ZipArchive zip = new(depStream);
					foreach(ZipArchiveEntry entry in zip.Entries) {
						if(entry.Name == "CheatDb." + consoleType.ToString() + ".json") {
							using Stream entryStream = entry.Open();
							using StreamReader reader = new StreamReader(entryStream);
							cheatDb = (CheatDatabase?)JsonSerializer.Deserialize(reader.ReadToEnd(), typeof(CheatDatabase), MesenCamelCaseSerializerContext.Default) ?? new CheatDatabase();
						}
					}
				}
			} catch { }

			_entries = cheatDb.Games;
			FilteredEntries = _entries;
			AddDisposable(this.WhenAnyValue(x => x.SearchString).Subscribe(x => {
				if(string.IsNullOrWhiteSpace(x)) {
					FilteredEntries = _entries;
				} else {
					FilteredEntries = _entries.Where(e => e.Name.Contains(x, StringComparison.OrdinalIgnoreCase));
				}

				SelectionModel.SelectedItem = FilteredEntries.FirstOrDefault();
			}));

			if(!Design.IsDesignMode) {
				string sha1 = EmuApi.GetRomHash(HashType.Sha1Cheat);
				SelectionModel.SelectedItem = _entries.Find(e => e.Sha1 == sha1);
			}
		}
	}
}
