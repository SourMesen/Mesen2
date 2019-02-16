using System;
using System.Collections.Generic;
using System.Text;

namespace Be.Windows.Forms
{
	/// <summary>
	/// Defines the type of the Find operation.
	/// </summary>
	public enum FindType
	{
		/// <summary>
		/// Used for Text Find operations
		/// </summary>
		Text,
		/// <summary>
		/// Used for Hex Find operations
		/// </summary>
		Hex
	}

	/// <summary>
	/// Defines all state information nee
	/// </summary>
	public class FindOptions
	{
		public bool WrapSearch { get; set; }
		public bool HasWildcard { get; set; }

		/// <summary>
		/// Gets or sets whether the Find options are valid
		/// </summary>
		public bool IsValid { get; set; }
		/// <summary>
		/// Gets the Find buffer used for case insensitive Find operations. This is the binary representation of Text.
		/// </summary>
		internal byte[] FindBuffer { get; private set; }
		/// <summary>
		/// Gets the Find buffer used for case sensitive Find operations. This is the binary representation of Text in lower case format.
		/// </summary>
		internal byte[] FindBufferLowerCase { get; private set; }
		/// <summary>
		/// Gets the Find buffer used for case sensitive Find operations. This is the binary representation of Text in upper case format.
		/// </summary>
		internal byte[] FindBufferUpperCase { get; private set; }
		/// <summary>
		/// Contains the MatchCase value
		/// </summary>
		bool _matchCase;
		/// <summary>
		/// Gets or sets the value, whether the Find operation is case sensitive or not.
		/// </summary>
		public bool MatchCase
		{
			get { return _matchCase; }
			set
			{
				_matchCase = value;
			}
		}
		/// <summary>
		/// Contains the text that should be found.
		/// </summary>
		string _text;
		/// <summary>
		/// Gets or sets the text that should be found. Only used, when Type is FindType.Hex.
		/// </summary>
		public string Text
		{
			get { return _text; }
			set
			{
				_text = value;
			}
		}
		/// <summary>
		/// Gets or sets the hex buffer that should be found. Only used, when Type is FindType.Hex.
		/// </summary>
		public byte[] Hex { get; set; }
		/// <summary>
		/// Gets or sets the type what should be searched.
		/// </summary>
		public FindType Type { get; set; }
		/// <summary>
		/// Updates the find buffer.
		/// </summary>
		internal void UpdateFindBuffer(IByteCharConverter byteCharConverter)
		{
			string text = this.Text != null ? this.Text : string.Empty;
			FindBuffer = byteCharConverter.GetBytes(text);
			FindBufferLowerCase = byteCharConverter.GetBytes(text.ToLower());
			FindBufferUpperCase = byteCharConverter.GetBytes(text.ToUpper());
		}
	}
}
