using System;
using System.Runtime.InteropServices;
using System.Text;

namespace Mesen.Utilities
{
	public class Utf8Utilities
	{
		public static string GetStringFromArray(byte[] strArray)
		{
			for(int i = 0; i < strArray.Length; i++) {
				if(strArray[i] == 0) {
					return Encoding.UTF8.GetString(strArray, 0, i);
				}
			}
			return Encoding.UTF8.GetString(strArray);
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

		public delegate void StringApiDelegate(IntPtr ptr, Int32 size);
		public unsafe static string CallStringApi(StringApiDelegate callback, int maxLength = 100000)
		{
			byte[] outBuffer = new byte[maxLength];
			fixed(byte* ptr = outBuffer) {
				callback((IntPtr)ptr, maxLength);
				return Utf8Utilities.PtrToStringUtf8((IntPtr)ptr);
			}
		}
	}	
}
