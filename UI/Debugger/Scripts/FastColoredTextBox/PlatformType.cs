using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace FastColoredTextBoxNS
{
    public static class PlatformType
    {
        const ushort PROCESSOR_ARCHITECTURE_INTEL = 0;
        const ushort PROCESSOR_ARCHITECTURE_IA64 = 6;
        const ushort PROCESSOR_ARCHITECTURE_AMD64 = 9;
        const ushort PROCESSOR_ARCHITECTURE_UNKNOWN = 0xFFFF;

        public static Platform GetOperationSystemPlatform()
        {
            if(MonoUtility.IsLinux)
            {
                return Platform.Unknown;
            }
            else
            {
                var sysInfo = new NativeMethods.SYSTEM_INFO();

                // WinXP and older - use GetNativeSystemInfo
                if (Environment.OSVersion.Version.Major > 5 ||
                    (Environment.OSVersion.Version.Major == 5 && Environment.OSVersion.Version.Minor >= 1))
                {
                    NativeMethodsWrapper.GetNativeSystemInfo(ref sysInfo);
                }
                // else use GetSystemInfo
                else
                {
                    NativeMethodsWrapper.GetSystemInfo(ref sysInfo);
                }

                switch (sysInfo.wProcessorArchitecture)
                {
                    case PROCESSOR_ARCHITECTURE_IA64:
                    case PROCESSOR_ARCHITECTURE_AMD64:
                        return Platform.X64;

                    case PROCESSOR_ARCHITECTURE_INTEL:
                        return Platform.X86;

                    default:
                        return Platform.Unknown;
                }
            }
        }
    }

    public enum Platform
    {
        X86,
        X64,
        Unknown
    }

}
