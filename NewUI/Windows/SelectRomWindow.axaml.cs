using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.ApplicationLifetimes;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using Mesen.GUI.Utilities;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Threading.Tasks;

namespace Mesen.Windows
{
	public partial class SelectRomWindow : Window
	{
		private ListBox _listBox;
		private TextBox _searchBox;

		public SelectRomWindow()
		{
			InitializeComponent();
#if DEBUG
			this.AttachDevTools();
#endif

			_searchBox = this.FindControl<TextBox>("Search");
			_listBox = this.FindControl<ListBox>("ListBox");
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnOpened(EventArgs e)
		{
			base.OnOpened(e);

			//Post this to allow focus to work properly when drag and dropping file
			Dispatcher.UIThread.Post(() => {
				Activate();
				_searchBox.Focus();
			});
		}

		protected override void OnKeyDown(KeyEventArgs e)
		{
			if(DataContext is SelectRomViewModel model) {
				if(e.Key == Key.Down || e.Key == Key.Up) {
					if(_searchBox.IsKeyboardFocusWithin) {
						if(model.FilteredEntries.Count() > 1) {
							model.SelectedEntry = model.FilteredEntries.ElementAt(1);
							_listBox.ItemContainerGenerator.ContainerFromIndex(1).Focus();
						} else {
							model.SelectedEntry = model.FilteredEntries.ElementAt(0);
							_listBox.ItemContainerGenerator.ContainerFromIndex(1).Focus();
						}
					}
				} else if(e.Key == Key.Enter && model.SelectedEntry != null) {
					model.Cancelled = false;
					Close();
				} else if(e.Key == Key.Escape) {
					Close();
				}
			}
			base.OnKeyDown(e);
		}

		public static async Task<ResourcePath?> Show(string file)
		{
			List<ArchiveRomEntry> entries = ArchiveHelper.GetArchiveRomList(file);
			if(entries.Count == 0) {
				return file;
			} else if(entries.Count == 1) {
				return new ResourcePath() { Path = file, InnerFile = entries[0].Filename, InnerFileIndex = entries[0].IsUtf8 ? 0 : 1 };
			}

			SelectRomViewModel model = new(entries) { SelectedEntry = entries[0] };
			SelectRomWindow wnd = new SelectRomWindow() { DataContext = model };

			if(Application.Current?.ApplicationLifetime is IClassicDesktopStyleApplicationLifetime desktop) {
				wnd.WindowStartupLocation = WindowStartupLocation.CenterOwner;
				await wnd.ShowDialog(desktop.MainWindow);
			}

			if(model.Cancelled || model.SelectedEntry == null) {
				return null;
			}

			int innerFileIndex = 0;
			if(!model.SelectedEntry.IsUtf8) {
				innerFileIndex = entries.IndexOf(model.SelectedEntry) + 1;
			}

			return new ResourcePath() { Path = file, InnerFile = model.SelectedEntry.Filename, InnerFileIndex = innerFileIndex };
		}

		private void OnOkClick(object sender, RoutedEventArgs e)
		{
			if(DataContext is SelectRomViewModel model && model.SelectedEntry != null) {
				model.Cancelled = false;
				Close();
			}
		}

		private void OnCancelClick(object sender, RoutedEventArgs e)
		{
			Close();
		}

		private void OnDoubleClick(object sender, RoutedEventArgs e)
		{
			if(DataContext is SelectRomViewModel model && model.SelectedEntry != null) {
				model.Cancelled = false;
				Close();
			}
		}
	}

	public class SelectRomViewModel : ViewModelBase
	{
		private readonly List<ArchiveRomEntry> _entries;
		[Reactive] public IEnumerable<ArchiveRomEntry> FilteredEntries { get; set; }
		[Reactive] public string SearchString { get; set; } = "";
		[Reactive] public ArchiveRomEntry? SelectedEntry { get; set; }
		[Reactive] public bool Cancelled { get; set; } = true;

		public SelectRomViewModel(List<ArchiveRomEntry> entries)
		{
			_entries = entries;
			FilteredEntries = entries;
			this.WhenAnyValue(x => x.SearchString).Subscribe(x => {
				if(string.IsNullOrWhiteSpace(x)) {
					FilteredEntries = _entries;
				} else {
					FilteredEntries = _entries.Where(e => e.Filename.Contains(x, StringComparison.OrdinalIgnoreCase));
				}

				SelectedEntry = FilteredEntries.FirstOrDefault();
			});
		}
	}
}
