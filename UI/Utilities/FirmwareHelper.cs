using Avalonia.Controls;
using Avalonia.Rendering;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Windows;
using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public static class FirmwareHelper
	{
		public static string GetFileHash(string filename)
		{
			using(SHA256 sha256Hash = SHA256.Create()) {
				// ComputeHash - returns byte array  
				byte[]? fileData = FileHelper.ReadAllBytes(filename);
				if(fileData == null) {
					return "";
				}
				byte[] bytes = sha256Hash.ComputeHash(fileData);

				// Convert byte array to a string   
				StringBuilder builder = new StringBuilder();
				for(int i = 0; i < bytes.Length; i++) {
					builder.Append(bytes[i].ToString("X2"));
				}
				return builder.ToString();
			}
		}

		public static async Task RequestFirmwareFile(MissingFirmwareMessage msg)
		{
			string filename = Marshal.PtrToStringUTF8(msg.Filename) ?? "";
			Window? wnd = ApplicationHelper.GetMainWindow();

			if(await MesenMsgBox.Show(wnd, "FirmwareNotFound", MessageBoxButtons.OKCancel, MessageBoxIcon.Question, ResourceHelper.GetEnumText(msg.Firmware), filename, msg.Size.ToString()) == DialogResult.OK) {
				while(true) {
					string? selectedFile = await FileDialogHelper.OpenFile(null, wnd, FileDialogHelper.FirmwareExt);
					if(selectedFile != null) {
						try {
							if(await SelectFirmwareFile(msg.Firmware, selectedFile, ApplicationHelper.GetMainWindow())) {
								break;
							}
						} catch(Exception ex) {
							await MesenMsgBox.ShowException(ex);
						}
					} else {
						break;
					}
				}
			}
		}

		public static async Task<bool> SelectFirmwareFile(FirmwareType type, string selectedFile, IRenderRoot? wnd)
		{
			FirmwareFiles knownFirmwares = type.GetFirmwareInfo();

			long fileSize = new FileInfo(selectedFile).Length;

			bool foundSizeMatch = false;
			foreach(FirmwareFileInfo knownFirmware in knownFirmwares) {
				if(knownFirmware.Size == fileSize) {
					foundSizeMatch = true;
				}
			}

			if(!foundSizeMatch) {
				await MesenMsgBox.Show(wnd, "FirmwareFileWrongSize", MessageBoxButtons.OK, MessageBoxIcon.Error, knownFirmwares[0].Size.ToString());
				return false;
			}

			string fileHash = GetFileHash(selectedFile);
			bool hashMatches = false;
			foreach(FirmwareFileInfo knownFirmware in knownFirmwares) {
				if(Array.IndexOf(knownFirmware.Hashes, fileHash) >= 0) {
					hashMatches = true;
					break;
				}
			}

			if(!hashMatches) {
				if(await MesenMsgBox.Show(wnd, "FirmwareMismatch", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, ResourceHelper.GetEnumText(type), knownFirmwares[0].Hashes[0], fileHash) != DialogResult.OK) {
					//Files don't match and user cancelled the action, retry
					return false;
				}
			}

			string destination = Path.Combine(ConfigManager.FirmwareFolder, knownFirmwares.Names[0]);
			if(selectedFile != destination) {
				if(File.Exists(destination)) {
					if(await MesenMsgBox.Show(wnd, "OverwriteFirmware", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, destination) != DialogResult.OK) {
						//Don't overwrite the existing file
						return true;
					}
				}
				File.Copy(selectedFile, destination, true);
			}
			return true;
		}
	}
}
