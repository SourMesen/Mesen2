using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static Mesen.GUI.Debugger.TblLoader;

namespace Be.Windows.Forms
{
	/// <summary>
	/// The default <see cref="IByteCharConverter"/> implementation.
	/// </summary>
	class TblByteCharConverter : IByteCharConverter
	{
		Dictionary<TblKey, string> _tblRules;
		Dictionary<string, TblKey> _reverseTblRules;
		List<string> _stringList;
		bool[] _hasPotentialRule = new bool[256];

		public TblByteCharConverter(Dictionary<TblKey, string> tblRules)
		{
			this._tblRules = tblRules;
			foreach(KeyValuePair<TblKey, string> kvp in tblRules) {
				_hasPotentialRule[kvp.Key.Key & 0xFF] = true;
			}

			this._stringList = new List<string>();
			this._reverseTblRules = new Dictionary<string, TblKey>();
			foreach(KeyValuePair<TblKey, string> kvp in tblRules) {
				this._stringList.Add(kvp.Value);
				this._reverseTblRules[kvp.Value] = kvp.Key;
			}
			this._stringList.Sort(new Comparison<string>((a, b) => {
				if(a.Length > b.Length) {
					return 1;
				} else if(a.Length < b.Length) {
					return -1;
				} else {
					return string.Compare(a, b);
				}
			}));
		}

		/// <summary>
		/// Returns the character to display for the byte passed across.
		/// </summary>
		/// <param name="b"></param>
		/// <returns></returns>
		public virtual string ToChar(UInt64 value, out int keyLength)
		{
			if(!_hasPotentialRule[value & 0xFF]) {
				keyLength = 1;
				return ".";
			}

			int byteCount = 8;
			string result;
			while(!_tblRules.TryGetValue(new TblKey() { Key = value, Length = byteCount }, out result) && byteCount > 0) {
				value &= ~(((UInt64)0xFF) << (8 * (byteCount - 1)));
				byteCount--;
			}

			if(result != null) {
				keyLength = byteCount;
				return result;
			} else {
				keyLength = 1;
				return ".";
			}
		}

		public virtual byte[] GetBytes(string text)
		{
			List<byte> bytes = new List<byte>();
			
			bool match = false;
			while(text.Length > 0) {
				do {
					match = false;
					foreach(string key in _stringList) {
						//Without an ordinal comparison, it's possible for an empty string to "StartsWith" a non-empty string (e.g replacement char U+FFFD)
						//This causes a crash here (because key.Length > text.Length)
						if(text.StartsWith(key, StringComparison.Ordinal)) {
							bytes.AddRange(_reverseTblRules[key].GetBytes());
							text = text.Substring(key.Length);
							match = true;
							break;
						}
					}
				} while(match);

				if(!match && text.Length > 0) {
					bytes.Add(0);
					text = text.Substring(1);
				}
			}

			return bytes.ToArray();
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

		/// <summary>
		/// Returns a description of the byte char provider.
		/// </summary>
		/// <returns></returns>
		public override string ToString()
		{
			return "TBL";
		}
	}
}
