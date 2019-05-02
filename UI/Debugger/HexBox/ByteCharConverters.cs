using System;
using System.Collections.Generic;
using System.Text;

namespace Be.Windows.Forms
{
	/// <summary>
	/// The interface for objects that can translate between characters and bytes.
	/// </summary>
	public interface IByteCharConverter
	{
		/// <summary>
		/// Returns the character to display for the byte passed across.
		/// </summary>
		/// <param name="b"></param>
		/// <returns></returns>
		string ToChar(UInt64 value, out int keyLength);

		/// <summary>
		/// Returns the byte to use when the character passed across is entered during editing.
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		byte ToByte(char c);

		byte[] GetBytes(string str);
	}

	/// <summary>
	/// The default <see cref="IByteCharConverter"/> implementation.
	/// </summary>
	public class DefaultByteCharConverter : IByteCharConverter
	{
		/// <summary>
		/// Returns the character to display for the byte passed across.
		/// </summary>
		/// <param name="b"></param>
		/// <returns></returns>
		public virtual string ToChar(UInt64 value, out int keyLength)
		{
			keyLength = 1;
			byte b = (byte)value;
			return new string((b > 0x1F && b <= 0x7E) ? (char)b : '.', 1);
		}

		/// <summary>
		/// Returns the byte to use for the character passed across.
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		public virtual byte ToByte(char c)
		{
			return (byte)c;
		}

		public virtual byte[] GetBytes(string str)
		{
			List<byte> bytes = new List<byte>(str.Length);
			foreach(char c in str) {
				bytes.Add((byte)c);
			}
			return bytes.ToArray();
		}

		/// <summary>
		/// Returns a description of the byte char provider.
		/// </summary>
		/// <returns></returns>
		public override string ToString()
		{
			return "ANSI (Default)";
		}
	}

	/// <summary>
	/// A byte char provider that can translate bytes encoded in codepage 500 EBCDIC
	/// </summary>
	public class EbcdicByteCharProvider : IByteCharConverter
	{
		/// <summary>
		/// The IBM EBCDIC code page 500 encoding. Note that this is not always supported by .NET,
		/// the underlying platform has to provide support for it.
		/// </summary>
		private Encoding _ebcdicEncoding = Encoding.GetEncoding(500);

		/// <summary>
		/// Returns the EBCDIC character corresponding to the byte passed across.
		/// </summary>
		/// <param name="b"></param>
		/// <returns></returns>
		public virtual string ToChar(UInt64 value, out int keyLength)
		{
			keyLength = 1;
			byte b = (byte)value;
			string encoded = _ebcdicEncoding.GetString(new byte[] { b });
			return new string(encoded.Length > 0 ? encoded[0] : '.', 1);
		}

		/// <summary>
		/// Returns the byte corresponding to the EBCDIC character passed across.
		/// </summary>
		/// <param name="c"></param>
		/// <returns></returns>
		public virtual byte ToByte(char c)
		{
			byte[] decoded = _ebcdicEncoding.GetBytes(new char[] { c });
			return decoded.Length > 0 ? decoded[0] : (byte)0;
		}

		public virtual byte[] GetBytes(string str)
		{
			List<byte> bytes = new List<byte>(str.Length);
			foreach(char c in str) {
				bytes.Add((byte)c);
			}
			return bytes.ToArray();
		}

		/// <summary>
		/// Returns a description of the byte char provider.
		/// </summary>
		/// <returns></returns>
		public override string ToString()
		{
			return "EBCDIC (Code Page 500)";
		}
	}
}