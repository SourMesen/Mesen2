using Avalonia.Controls;
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
		private static string GetFileHash(string filename)
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
							List<string> expectedHashes = msg.GetFileHashes();
							if(new FileInfo(selectedFile).Length != msg.Size) {
								await MesenMsgBox.Show(wnd, "FirmwareFileWrongSize", MessageBoxButtons.OK, MessageBoxIcon.Error, msg.Size.ToString());
								continue;
							}

							string filehash = GetFileHash(selectedFile);
							if(!expectedHashes.Contains(filehash)) {
								if(await MesenMsgBox.Show(wnd, "FirmwareMismatch", MessageBoxButtons.OKCancel, MessageBoxIcon.Warning, ResourceHelper.GetEnumText(msg.Firmware), expectedHashes[0], filehash) != DialogResult.OK) {
									//Files don't match and user cancelled the action
									return;
								}
							}

							string destination = Path.Combine(ConfigManager.FirmwareFolder, filename);
							if(selectedFile != destination) {
								File.Copy(selectedFile, destination, true);
							}
						} catch(Exception ex) {
							await MesenMsgBox.ShowException(ex);
						}
					}

					//Only loop if user picks a file with the wrong size
					break;
				}
			}
		}
	}
}
