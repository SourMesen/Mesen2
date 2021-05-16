using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class AudioPlayerConfig : BaseConfig<AudioPlayerConfig>
	{
		[Reactive] public UInt32 Volume { get; set; } = 100;
		[Reactive] public bool Repeat { get; set; } = false;
		[Reactive] public bool Shuffle { get; set; } = false;

		public void ApplyConfig()
		{
			ConfigApi.SetAudioPlayerConfig(new InteropAudioPlayerConfig() {
				Volume = Volume,
				Repeat = Repeat,
				Shuffle = Shuffle,
			});
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	public struct InteropAudioPlayerConfig
	{
		public UInt32 Volume;
		[MarshalAs(UnmanagedType.I1)] public bool Repeat;
		[MarshalAs(UnmanagedType.I1)] public bool Shuffle;
	}
}