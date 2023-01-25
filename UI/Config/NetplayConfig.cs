using ReactiveUI.Fody.Helpers;
using System;

namespace Mesen.Config
{
	public class NetplayConfig : BaseConfig<NetplayConfig>
	{
		[Reactive] public string Host { get; set; } = "localhost";
		[Reactive] public UInt16 Port { get; set; } = 8888;
		[Reactive] public string Password { get; set; } = "";

		[Reactive] public UInt16 ServerPort { get; set; } = 8888;
		[Reactive] public string ServerPassword { get; set; } = "";
	}
}
