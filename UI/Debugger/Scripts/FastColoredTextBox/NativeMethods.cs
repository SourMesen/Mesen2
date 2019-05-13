using System;
using System.Collections.Generic;
using System.Text;
using System.Runtime.InteropServices;

namespace FastColoredTextBoxNS
{
	internal class NativeMethods
	{
		//[DllImport("user32.dll")]
		//internal static extern IntPtr GetOpenClipboardWindow(); // unused ?

		[DllImport("user32.dll")]
		protected static extern bool CloseClipboard(); // corrected intptr -> bool

		[DllImport("Imm32.dll")]
		protected static extern IntPtr ImmGetContext(IntPtr hWnd);

		[DllImport("Imm32.dll")]
		protected static extern IntPtr ImmAssociateContext(IntPtr hWnd, IntPtr hIMC);

		[DllImport("User32.dll")]
		protected static extern bool CreateCaret(IntPtr hWnd, int hBitmap, int nWidth, int nHeight);

		[DllImport("User32.dll")]
		protected static extern bool SetCaretPos(int x, int y);

		//[DllImport("User32.dll")]
		//internal static extern bool DestroyCaret(); // unused ?

		[DllImport("User32.dll")]
		protected static extern bool ShowCaret(IntPtr hWnd);

		[DllImport("User32.dll")]
		protected static extern bool HideCaret(IntPtr hWnd);

		[DllImport("User32.dll")]
		protected static extern IntPtr SendMessage(IntPtr hwnd, int wMsg, int wParam, int lParam); // change corrent

		[DllImport("kernel32.dll")]
		protected static extern void GetNativeSystemInfo(ref SYSTEM_INFO lpSystemInfo);

		[DllImport("kernel32.dll")]
		protected static extern void GetSystemInfo(ref SYSTEM_INFO lpSystemInfo);

		[StructLayout(LayoutKind.Sequential)]
		internal struct SYSTEM_INFO
		{
			public ushort wProcessorArchitecture;
			public ushort wReserved;
			public uint dwPageSize;
			public IntPtr lpMinimumApplicationAddress;
			public IntPtr lpMaximumApplicationAddress;
			public UIntPtr dwActiveProcessorMask;
			public uint dwNumberOfProcessors;
			public uint dwProcessorType;
			public uint dwAllocationGranularity;
			public ushort wProcessorLevel;
			public ushort wProcessorRevision;
		};
	}
}
