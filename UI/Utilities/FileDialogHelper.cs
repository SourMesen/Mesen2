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
		public const string LabelFileExt = "[[LABELFILES]]";
		public const string MesenMovieExt = "mmo";
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
		public const string ElfFileExt = "elf";
		public const string SymFileExt = "sym";
		public const string CdbFileExt = "cdb";
		public const string MesenLabelExt = "mlb";
		public const string NesAsmLabelExt = "fns";
		public const string BinExt = "bin";
		public const string NesExt = "nes";
		public const string SufamiTurboExt = "st";

		public static async Task<string?> OpenFile(string? initialFolder, IRenderRoot? parent, params string[] extensions)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			try {
				List<FilePickerFileType> filter = new List<FilePickerFileType>();
				foreach(string ext in extensions) {
					if(ext == FileDialogHelper.RomExt) {
						filter.Add(new FilePickerFileType("All ROM files") { Patterns = new List<string>() { 
							"*.sfc", "*.fig", "*.smc", "*.bs", "*.st", "*.spc",
							"*.nes", "*.fds", "*.qd", "*.unif", "*.unf", "*.studybox", "*.nsf", "*.nsfe",
							"*.gb", "*.gbc", "*.gbx", "*.gbs",
							"*.pce", "*.sgx", "*.cue", "*.hes",
							"*.sms", "*.gg", "*.sg", "*.col",
							"*.gba",
							"*.ws", "*.wsc",
							"*.zip", "*.7z"
						} });
						filter.Add(new FilePickerFileType("SNES ROM files") { Patterns = new List<string>() { "*.sfc", "*.fig", "*.smc", "*.bs", "*.st", "*.spc" } });
						filter.Add(new FilePickerFileType("NES ROM files") { Patterns = new List<string>() { "*.nes", "*.fds", "*.qd", "*.unif", "*.unf", "*.studybox", "*.nsf", "*.nsfe" } });
						filter.Add(new FilePickerFileType("GB ROM files") { Patterns = new List<string>() { "*.gb", "*.gbc", "*.gbx", "*.gbs" } });
						filter.Add(new FilePickerFileType("GBA ROM files") { Patterns = new List<string>() { "*.gba" } });
						filter.Add(new FilePickerFileType("PC Engine ROM files") { Patterns = new List<string>() { "*.pce", "*.sgx", "*.cue", "*.hes" } });
						filter.Add(new FilePickerFileType("SMS / GG ROM files") { Patterns = new List<string>() { "*.sms", "*.gg" } });
						filter.Add(new FilePickerFileType("SG-1000 ROM files") { Patterns = new List<string>() { "*.sg" } });
						filter.Add(new FilePickerFileType("ColecoVision ROM files") { Patterns = new List<string>() { "*.col" } });
						filter.Add(new FilePickerFileType("WonderSwan ROM files") { Patterns = new List<string>() { "*.ws", "*.wsc" } });
					} else if(ext == FileDialogHelper.FirmwareExt) {
						filter.Add(new FilePickerFileType("All firmware files") { Patterns = new List<string>() { "*.sfc", "*.pce", "*.nes", "*.bin", "*.rom", "*.col", "*.sms", "*.gg", "*.gba" } });
					} else if(ext == FileDialogHelper.LabelFileExt) {
						filter.Add(new FilePickerFileType("All label files") { Patterns = new List<string>() { "*.mlb", "*.sym", "*.dbg", "*.fns", "*.elf", "*.cdb" } });
					} else {
						filter.Add(new FilePickerFileType(ext.ToUpper() + " files") { Patterns = new List<string>() { "*." + ext } });
					}
				}
				filter.Add(new FilePickerFileType("All files") { Patterns = new List<string>() { "*" } });

				IReadOnlyList<IStorageFile> files = await wnd.StorageProvider.OpenFilePickerAsync(new FilePickerOpenOptions() {
					SuggestedStartLocation = initialFolder != null ? await wnd.StorageProvider.TryGetFolderFromPathAsync(initialFolder) : null,
					AllowMultiple = false,
					FileTypeFilter = filter
				});

				if(files.Count > 0) {
					return files[0].Path.LocalPath;
				}
			} catch(Exception ex) {
				await MesenMsgBox.ShowException(ex);
			}
			return null;
		}

		public static async Task<string?> SaveFile(string? initialFolder, string? initialFile, IRenderRoot? parent, params string[] extensions)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			try {
				List<FilePickerFileType> filter = new List<FilePickerFileType>();
				foreach(string ext in extensions) {
					filter.Add(new FilePickerFileType(ext.ToUpper() + " files") { Patterns = new List<string>() { "*." + ext } });
				}
				filter.Add(new FilePickerFileType("All files") { Patterns = new List<string>() { "*" } });

				IStorageFolder? startLocation = initialFolder != null ? await wnd.StorageProvider.TryGetFolderFromPathAsync(initialFolder) : null;
				if(OperatingSystem.IsLinux()) {
					//TODOv2 - setting a start location appears to cause crashes on Linux (dbus crash), force it to null for now
					startLocation = null;
				}

				IStorageFile? file = await wnd.StorageProvider.SaveFilePickerAsync(new FilePickerSaveOptions() {
					SuggestedStartLocation = startLocation,
					DefaultExtension = extensions[0],
					ShowOverwritePrompt = true,
					SuggestedFileName = initialFile,
					FileTypeChoices = filter
				});

				if(file != null) {
					return file.Path.LocalPath;
				}
			} catch(Exception ex) {
				await MesenMsgBox.ShowException(ex);
			}
			return null;
		}

		public static async Task<string?> OpenFolder(IRenderRoot? parent)
		{
			if(!((parent ?? ApplicationHelper.GetMainWindow()) is Window wnd)) {
				throw new Exception("Invalid parent window");
			}

			try {
				IReadOnlyList<IStorageFolder> folders = await wnd.StorageProvider.OpenFolderPickerAsync(new FolderPickerOpenOptions() {
					AllowMultiple = false
				});

				if(folders.Count > 0) {
					return folders[0].Path.LocalPath;
				}
			} catch(Exception ex) {
				await MesenMsgBox.ShowException(ex);
			}
			return null;
		}
	}
}
