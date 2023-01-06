using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using Mesen.Windows;
using ReactiveUI.Fody.Helpers;
using System;
using System.Security.Cryptography;
using System.Diagnostics;
using System.IO;
using System.IO.Compression;
using System.Net;
using System.Net.Http;
using System.Runtime.InteropServices;
using System.Text.Json;
using System.Threading.Tasks;
using System.Xml;
using System.Runtime.Intrinsics.Arm;

namespace Mesen.ViewModels
{
	public class UpdatePromptViewModel : ViewModelBase
	{
		public Version LatestVersion { get; }
		public Version InstalledVersion { get; }
		public string Changelog { get; }

		[Reactive] public bool IsUpdating { get; internal set; }
		[Reactive] public int Progress { get; internal set; }

		private UpdateInfo _updateInfo;

		public UpdatePromptViewModel(UpdateInfo updateInfo)
		{
			LatestVersion = updateInfo.LatestVersion;
			Changelog = updateInfo.ReleaseNotes;
			InstalledVersion = EmuApi.GetMesenVersion();

			_updateInfo = updateInfo;
		}

		public static async Task<UpdatePromptViewModel?> GetUpdateInformation()
		{
			UpdateInfo? updateInfo = null;
			try {
				using(var client = new HttpClient()) {
					string platform = RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ? "win" : "linux";
					string updateData = await client.GetStringAsync("https://www.mesen.ca/Services/v2/latestversion." + platform + ".json");
					updateInfo = JsonSerializer.Deserialize<UpdateInfo>(updateData);

					if(updateInfo == null || (!updateInfo.DownloadUrl.StartsWith("https://www.mesen.ca/") && !updateInfo.DownloadUrl.StartsWith("https://github.com/SourMesen/"))) {
						return null;
					}
				}
			} catch(Exception ex) {
				await MesenMsgBox.ShowException(ex);
			}

			return updateInfo != null ? new UpdatePromptViewModel(updateInfo) : null;
		}

		public async Task<bool> UpdateMesen()
		{
			string downloadPath = Path.Combine(ConfigManager.BackupFolder, "Mesen." + LatestVersion.ToString(3));

			using(var client = new HttpClient()) {
				HttpResponseMessage response = await client.GetAsync(_updateInfo.DownloadUrl, HttpCompletionOption.ResponseHeadersRead);
				response.EnsureSuccessStatusCode();

				using Stream contentStream = await response.Content.ReadAsStreamAsync();
				using MemoryStream memoryStream = new MemoryStream();
				long? length = response.Content.Headers.ContentLength;
				if(length == null || length == 0) {
					return false;
				}

				byte[] buffer = new byte[0x10000];
				while(true) {
					int byteCount = await contentStream.ReadAsync(buffer, 0, buffer.Length);
					if(byteCount == 0) {
						break;
					} else {
						await memoryStream.WriteAsync(buffer, 0, byteCount);
						Dispatcher.UIThread.Post(() => {
							Progress = (int)((double)memoryStream.Length / length * 100);
						});
					}
				}

				byte[] exeData = memoryStream.ToArray();
				if(exeData.Length == _updateInfo.FileSize) {
					using ZipArchive archive = new ZipArchive(memoryStream);
					foreach(var entry in archive.Entries) {
						downloadPath += Path.GetExtension(entry.Name);
						entry.ExtractToFile(downloadPath, true);

						string hash;
						using(SHA256 sha256 = SHA256.Create()) {
							using(FileStream fileStream = File.OpenRead(downloadPath)) {
								hash = BitConverter.ToString(sha256.ComputeHash(fileStream)).Replace("-", "");
							}
						}
						if(hash != _updateInfo.Hash) {
							File.Delete(downloadPath);
						}
						break;
					}
				}
			}

			return UpdateHelper.LaunchUpdate(downloadPath);
		}
	}

	public class UpdateInfo
	{
		public Version LatestVersion { get; set; } = new();
		public string ReleaseNotes { get; set; } = "";
		public string DownloadUrl { get; set; } = "";
		public int FileSize { get; set; } = 0;
		public string Hash { get; set; } = "";
	}
}
