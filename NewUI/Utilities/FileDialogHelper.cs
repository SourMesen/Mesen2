using Avalonia.Controls;
using Avalonia.Platform.Storage;
using Avalonia.Platform.Storage.FileIO;
using Avalonia.Rendering;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class FileDialogHelper
	{
		public const string RomExt = "[[ROMFILES]]";
		public const string FirmwareExt = "[[FIRMWAREFILES]]";
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

			List<FilePickerFileType> filter = new List<FilePickerFileType>();
			foreach(string ext in extensions) {
				if(ext == FileDialogHelper.RomExt) {
					filter.Add(new FilePickerFileType("All ROM Files") { Patterns = new List<string>() { "*.sfc", "*.fig", "*.smc", "*.spc", "*.nes", "*.fds", "*.unif", "*.studybox", "*.nsf", "*.nsfe", "*.gb", "*.gbc", "*.gbs", "*.pce", "*.cue", "*.zip", "*.7z" } });
					filter.Add(new FilePickerFileType("SNES ROM Files") { Patterns = new List<string>() { "*.sfc", "*.fig", "*.smc", "*.spc" } });
					filter.Add(new FilePickerFileType("NES ROM Files") { Patterns = new List<string>() { "*.nes", "*.fds", "*.unif", "*.studybox", "*.nsf", "*.nsfe" } });
					filter.Add(new FilePickerFileType("GB ROM Files") { Patterns = new List<string>() { "*.gb", "*.gbc", "*.gbs" } });
					filter.Add(new FilePickerFileType("PC Engine ROM ROM Files") { Patterns = new List<string>() { "*.pce", "*.cue" } });
				} else if(ext == FileDialogHelper.FirmwareExt) {
					filter.Add(new FilePickerFileType("All firmware Files") { Patterns = new List<string>() { "*.sfc", "*.pce", "*.nes", "*.bin", "*.rom" } });
				} else {
					filter.Add(new FilePickerFileType(ext.ToUpper() + " files") { Patterns = new List<string>() { "*." + ext } });
				}
			}
			filter.Add(new FilePickerFileType("All files") { Patterns = new List<string>() { "*" } });

			IReadOnlyList<IStorageFile> files = await wnd.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions() {
				SuggestedStartLocation = initialFolder != null ? new BclStorageFolder(new DirectoryInfo(initialFolder)) : null,
				AllowMultiple = false,
				FileTypeFilter = filter
			});

			if(files.Count > 0) {
				if(files[0].TryGetUri(out Uri? filePath)) {
					return filePath.LocalPath;
				}
			}
			return null;
		}

		public static async Task<string?> SaveFile(string? initialFolder, string? initialFile, IRenderRoot? parent, params string[] extensions)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			List<FilePickerFileType> filter = new List<FilePickerFileType>();
			foreach(string ext in extensions) {
				filter.Add(new FilePickerFileType(ext.ToUpper() + " files") { Patterns = new List<string>() { "*." + ext } });
			}
			filter.Add(new FilePickerFileType("All files") { Patterns = new List<string>() { "*" } });

			IStorageFile? file = await wnd.StorageProvider.SaveFilePickerAsync(new FilePickerSaveOptions() {
				SuggestedStartLocation = initialFolder != null ? new BclStorageFolder(new DirectoryInfo(initialFolder)) : null,
				DefaultExtension = extensions[0],
				ShowOverwritePrompt = true,
				SuggestedFileName = initialFile,
				FileTypeChoices = filter
			});

			if(file != null) {
				if(file.TryGetUri(out Uri? filePath)) {
					return filePath.LocalPath;
				}
			}
			return null;
		}

		public static async Task<string?> OpenFolder(IRenderRoot? parent)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			IReadOnlyList<IStorageFolder> folders = await wnd.StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions() {
				AllowMultiple = false
			});

			if(folders.Count > 0) {
				if(folders[0].TryGetUri(out Uri? folderPath)) {
					return folderPath.LocalPath;
				}
			}
			return null;
		}
	}
}
