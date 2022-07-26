using Avalonia.Controls;
using Avalonia.Rendering;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class FileDialogHelper
	{
		public const string RomExt = "[[ROMFILES]]";
		public const string MesenMovieExt = "msm";
		public const string TblExt = "tbl";
		public const string PaletteExt = "pal";
		public const string TraceExt = "txt";
		public const string ZipExt = "zip";
		public const string GifExt = "gif";
		public const string AviExt = "avi";
		public const string WaveExt = "wav";
		public const string MesenSaveStateExt = "mss";
		public const string WatchFileExt = "txt";
		public const string LuaExt = "lua";
		public const string PngExt = "png";
		public const string DmpExt = "dmp";
		public const string IpsExt = "ips";
		public const string CdlExt = "cdl";
		public const string DbgFileExt = "dbg";
		public const string MesenLabelExt = "mlb";
		public const string BinExt = "bin";
		public const string NesExt = "nes";

		public static async Task<string?> OpenFile(string? initialFolder, IRenderRoot? parent, params string[] extensions)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			OpenFileDialog ofd = new OpenFileDialog();
			ofd.Directory = initialFolder;
			ofd.Filters = new List<FileDialogFilter>();
			foreach(string ext in extensions) {
				if(ext == FileDialogHelper.RomExt) {
					ofd.Filters.Add(new() { Name = "All ROM Files", Extensions = { "sfc", "fig", "smc", "spc", "nes", "fds", "unif", "studybox", "nsf", "nsfe", "gb", "gbc", "gbs", "pce", "cue", "zip", "7z" } });
					ofd.Filters.Add(new() { Name = "SNES ROM Files", Extensions = { "sfc", "fig", "smc", "spc" } });
					ofd.Filters.Add(new() { Name = "NES ROM Files", Extensions = { "nes", "fds", "unif", "studybox", "nsf", "nsfe" } });
					ofd.Filters.Add(new() { Name = "GB ROM Files", Extensions = { "gb", "gbc", "gbs" } });
					ofd.Filters.Add(new() { Name = "PC Engine ROM Files", Extensions = { "pce", "cue" } });
				} else {
					ofd.Filters.Add(new FileDialogFilter() { Name = ext.ToUpper() + " files", Extensions = { ext } });
				}
			}
			ofd.Filters.Add(new FileDialogFilter() { Name = "All files", Extensions = { "*" } });

			string[]? filenames = await ofd.ShowAsync(wnd);
			if(filenames?.Length > 0 && filenames[0].Length > 0) {
				return filenames[0];
			}
			return null;
		}

		public static async Task<string?> SaveFile(string? initialFolder, string? initialFile, IRenderRoot? parent, params string[] extensions)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			SaveFileDialog sfd = new SaveFileDialog();
			sfd.Directory = initialFolder;
			sfd.InitialFileName = initialFile;
			sfd.Filters = new List<FileDialogFilter>();
			foreach(string ext in extensions) {
				sfd.Filters.Add(new FileDialogFilter() { Name = ext.ToUpper() + " files", Extensions = { ext } });
			}
			sfd.Filters.Add(new FileDialogFilter() { Name = "All files", Extensions = { "*" } });

			string? filename = await sfd.ShowAsync(wnd);
			if(filename?.Length > 0) {
				return filename;
			}
			return null;
		}
	}
}
