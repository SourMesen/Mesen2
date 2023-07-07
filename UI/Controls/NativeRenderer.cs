using Avalonia;
using Avalonia.Controls;
using Avalonia.Layout;
using Avalonia.Platform;
using Mesen.Config;
using Mesen.Interop;
using Mesen.ViewModels;
using System;

namespace Mesen
{
	public class NativeRenderer : NativeControlHost
	{
		public NativeRenderer()
		{
			Focusable = true;
		}

		public IntPtr Handle { get; private set; }

		protected override IPlatformHandle CreateNativeControlCore(IPlatformHandle parent)
		{
			var handle = base.CreateNativeControlCore(parent);
			Handle = handle.Handle;
			return handle;
		}
	}
}
