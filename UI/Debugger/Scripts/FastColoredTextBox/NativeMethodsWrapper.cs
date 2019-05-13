using System;
using System.Collections.Generic;
using System.Text;

namespace FastColoredTextBoxNS
{
	internal class NativeMethodsWrapper : NativeMethods
	{
		internal new static bool CloseClipboard()
		{
			if (MonoUtility.IsLinux)
			{
				return true; // TODO
			}
			else
			{
				return NativeMethods.CloseClipboard();
			}
		}

		internal new static IntPtr ImmGetContext(IntPtr hWnd)
		{
			if (MonoUtility.IsLinux)
			{
				return IntPtr.Zero;
			}
			else
			{
				return NativeMethods.ImmGetContext(hWnd);
			}
		}

		internal new static IntPtr ImmAssociateContext(IntPtr hWnd, IntPtr hIMC)
		{
			if (MonoUtility.IsLinux)
			{
				return IntPtr.Zero;
			}
			else
			{
				return NativeMethods.ImmAssociateContext(hWnd, hIMC);
			}
		}

		internal new static bool CreateCaret(IntPtr hWnd, int hBitmap, int nWidth, int nHeight)
		{
			if (MonoUtility.IsLinux)
			{
				return true; // TODO
			}
			else
			{
				return NativeMethods.CreateCaret(hWnd, hBitmap, nWidth, nHeight);
			}
		}

		internal new static bool SetCaretPos(int x, int y)
		{
			if (MonoUtility.IsLinux)
			{
				return true; // TODO
			}
			else
			{
				return NativeMethods.SetCaretPos(x, y);
			}
		}

		internal new static bool ShowCaret(IntPtr hWnd)
		{
			if (MonoUtility.IsLinux)
			{
				return true; // TODO
			}
			else
			{
				return NativeMethods.ShowCaret(hWnd);
			}
		}

		internal new static bool HideCaret(IntPtr hWnd)
		{
			if (MonoUtility.IsLinux)
			{
				return true; // TODO
			}
			else
			{
				return NativeMethods.HideCaret(hWnd);
			}
		}

		internal new static IntPtr SendMessage(IntPtr hWnd, int wMsg, int wParam, int lParam)
		{
			if (MonoUtility.IsLinux)
			{
				return IntPtr.Zero; // TODO
			}
			else
			{
				return NativeMethods.SendMessage(hWnd, wMsg, wParam, lParam);
			}
		}

		internal new static void GetNativeSystemInfo(ref SYSTEM_INFO lpSystemInfo)
		{
			if (MonoUtility.IsLinux)
			{
				throw new ApplicationException("This method is not supported in mono");
			}
			else
			{
				NativeMethods.GetNativeSystemInfo(ref lpSystemInfo);
			}
		}

		internal new static void GetSystemInfo(ref SYSTEM_INFO lpSystemInfo)
		{
			if (MonoUtility.IsLinux)
			{
				throw new ApplicationException("This method is not supported in mono");
			}
			else
			{
				NativeMethods.GetSystemInfo(ref lpSystemInfo);
			}
		}
	}
}
