using Mesen.Config;
using Octokit;
using System;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Threading.Tasks;

namespace Mesen.Utilities;

public static class LspServerHelper
{
	public const string LspDirectoryName = "LuaLSP";
	public static string ExecutableName => OperatingSystem.IsWindows() ? "lua-language-server.exe" : "lua-language-server";
	public static string ExecutableFullName => Path.Combine(ConfigManager.HomeFolder, LspDirectoryName, "bin", ExecutableName);

	public static async Task<string?> GetDownloadUrl()
	{
		var system = OperatingSystem.IsWindows()
								? "win32"
								: OperatingSystem.IsLinux()
								? "linux"
								: OperatingSystem.IsMacOS()
								? "darwin"
								: "?";

		string arch = "?";
		switch(RuntimeInformation.ProcessArchitecture) {
			case Architecture.Arm64:
				arch = "arm64";
				break;
			case Architecture.X64:
				arch = "x64";
				break;
			case Architecture.X86:
				arch = "ia32";
				break;
		}

		if(arch.Equals("?") || system.Equals("?")) return null;

		string ext = OperatingSystem.IsWindows() ? "zip" : "tar.gz";

		var client = new GitHubClient(new ProductHeaderValue("mesen2"));
		var releases = await client.Repository.Release.GetAll("LuaLS", "lua-language-server");
		var downloadUrl = releases.FirstOrDefault()?.Assets.FirstOrDefault(x => x.BrowserDownloadUrl.EndsWith($"{system}-{arch}.{ext}"))?.BrowserDownloadUrl;

		return downloadUrl;
	}
}
