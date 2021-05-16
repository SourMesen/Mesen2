using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Mesen.Utilities
{
	public class Utf8Utilities
	{
		public static string GetStringFromIntPtr(IntPtr pNativeData)
		{
			int offset = 0;
			byte b = 0;
			do {
				b = Marshal.ReadByte(pNativeData, offset);
				offset++;
			} while(b != 0);

			int length = offset - 1;

			// should not be null terminated
			byte[] strbuf = new byte[length];
			// skip the trailing null
			Marshal.Copy((IntPtr)pNativeData, strbuf, 0, length);
			string data = Encoding.UTF8.GetString(strbuf);
			return data;
		}

		public static string PtrToStringUtf8(IntPtr ptr)
		{
			if(ptr == IntPtr.Zero) {
				return "";
			}

			int len = 0;
			while(Marshal.ReadByte(ptr, len) != 0) {
				len++;
			}

			if(len == 0) {
				return "";
			}

			byte[] array = new byte[len];
			Marshal.Copy(ptr, array, 0, len);
			return Encoding.UTF8.GetString(array);
		}
	}	
}
