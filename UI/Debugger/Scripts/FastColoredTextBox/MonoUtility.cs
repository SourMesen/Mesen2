using System;
using System.Collections.Generic;
using System.Text;

namespace FastColoredTextBoxNS
{
	internal class MonoUtility
	{
		// .Net 1.0 and 1.1 didn't have the PlatformID value for Unix, so Mono used the value 128.
		private const PlatformID MonoUnix = (PlatformID)128;

		public static bool IsLinux
		{
			get
			{
				PlatformID p = Environment.OSVersion.Platform;
				return (p == PlatformID.Unix) || (p == PlatformID.MacOSX) || (p == MonoUnix);
			}
		}
	}
}
