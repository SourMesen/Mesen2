using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Threading;
using System;
using System.ComponentModel;
using Avalonia.Data;
using Mesen.Interop;
using System.Collections.Generic;
using Avalonia.Input;
using Mesen.Utilities;

namespace Mesen.Windows
{
	public class AboutWindow : Window
	{
		public class AboutListEntry
		{
			public AboutListEntry(string name, string author, string note, string url)
			{
				Name = name;
				Author = author;
				Note = note;
				Url = url;
			}

			public string Name { get; set; } = "";
			public string Author { get; set; } = "";
			public string Note { get; set; } = "";
			public string Url { get; set; } = "";
		}

		public string Version { get; }
		public List<AboutListEntry> LibraryList { get; }
		public List<AboutListEntry> AcknowledgeList { get; }

		public AboutWindow()
		{
			Version = EmuApi.GetMesenVersion().ToString();

			LibraryList = new List<AboutListEntry>() {
				new("Avalonia", "", "MIT", "https://github.com/AvaloniaUI/Avalonia"),
				new("Dock.Avalonia", "", "MIT", "https://github.com/wieslawsoltes/Dock"),
				new("DataBox", "", "MIT", "https://github.com/wieslawsoltes/DataBox"),
				new("ReactiveUI", "", "MIT", "https://github.com/reactiveui/ReactiveUI"),
				new("SkiaSharp", "", "MIT", "https://github.com/mono/SkiaSharp"),
				new("Skia", "", "BSD", "https://github.com/google/skia"),
				new("DynamicData", "", "MIT", "https://github.com/reactivemarbles/DynamicData"),
				new("HarfBuzz", "", "MIT", "https://github.com/harfbuzz/harfbuzz"),
				new("miniz", "", "MIT", "https://github.com/richgel999/miniz"),
				new("libspng", "","BSD", "https://github.com/randy408/libspng"),
				new("Lua", "","MIT", "https://www.lua.org"),
				new("7-Zip", "","Public domain", "https://www.7-zip.org/"),
				new("DirectXTK", "", "MIT", "https://github.com/microsoft/DirectXTK"),
				new("scale2x", "","GPL2", "https://github.com/amadvance/scale2x/"),
				new("Super Eagle + 2xSai", "", "GPL3", "https://vdnoort.home.xs4all.nl/emulation/2xsai/"),
				new("xBRZ", "","GPL3", "https://sourceforge.net/projects/xbrz/"),
				new("blip_buf", "blargg", "LGPL", "http://slack.net/~ant/"),
				new("SPC_DSP", "blargg", "LGPL", "http://slack.net/~ant/"),
				new("nes_ntsc", "blargg", "LGPL", "http://slack.net/~ant/"),
				new("snes_ntsc", "blargg", "LGPL", "http://slack.net/~ant/"),
				new("stb_vorbis", "", "Public domain", "https://github.com/nothings/stb"),
				new("emu2413", "Mitsukata Okazaki", "MIT", "https://github.com/digital-sound-antiques/emu2413"),
				new("SDD-1 Decomp. (Andreas Naive)", "Andreas Naive", "Public domain", ""),
				new("LED Icons", "", "CC BY 4.0", "http://led24.de"),
				new("SDL2", "", "zlib", "https://www.libsdl.org/"),
				new("magic_enum", "", "MIT", "https://github.com/Neargye/magic_enum")
			};

			LibraryList.Sort((a, b) => a.Name.CompareTo(b.Name));

			AcknowledgeList = new List<AboutListEntry>() {
				new("ares (Near)", "Near", "ISC", "https://github.com/ares-emulator/ares"),
				new("Mighty Mo (Cheat database)", "", "", ""),
				new("NesDev Wiki/Forums Contributors", "", "", "https://www.nesdev.org/"),
				new("SameBoy (LIJI32)", "LIJI32", "MIT", "https://github.com/LIJI32/SameBoy"),
			};

			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void btnOk_OnClick(object? sender, RoutedEventArgs e)
		{
			Close();
		}

		private void OnLinkPressed(object? sender, PointerPressedEventArgs e)
		{
			if(sender is TextBlock text && text.DataContext is AboutListEntry entry) {
				ApplicationHelper.OpenBrowser(entry.Url);
			}
		}

		private void OnMesenLinkTapped(object? sender, TappedEventArgs e)
		{
			ApplicationHelper.OpenBrowser("https://www.mesen.ca");
		}
	}
}