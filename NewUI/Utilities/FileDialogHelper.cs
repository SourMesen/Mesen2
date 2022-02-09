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
		public static string RomExt = "[[ROMFILES]]";
		public static string MesenMovieExt = "msm";
		public static string TblExt = "tbl";
		public static string PaletteExt = "pal";
		public static string TraceExt = "txt";
		public static string ZipExt = "zip";
		public static string GifExt = "gif";
		public static string AviExt = "avi";
		public static string WaveExt = "wav";
		public static string MesenSaveStateExt = "mss";
		public static string WatchFileExt = "txt";
		public static string LuaExt = "lua";
		public static string PngExt = "png";
		public static string DmpExt = "dmp";
		public static string IpsExt = "ips";

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
					ofd.Filters.Add(new() { Name = "All ROM Files", Extensions = { "sfc", "fig", "smc", "spc", "nes", "fds", "unif", "nsf", "nsfe", "gb", "gbc", "gbs", "zip", "7z" } });
					ofd.Filters.Add(new() { Name = "SNES ROM Files", Extensions = { "sfc", "fig", "smc", "spc" } });
					ofd.Filters.Add(new() { Name = "NES ROM Files", Extensions = { "nes", "fds", "unif", "nsf", "nsfe" } });
					ofd.Filters.Add(new() { Name = "GB ROM Files", Extensions = { "gb", "gbc", "gbs" } });
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
