using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public class HexUtilities
	{
		private static byte[] _hexLookup = new byte[256];

		static HexUtilities()
		{
			_hexLookup['0'] = 0;
			_hexLookup['1'] = 1;
			_hexLookup['2'] = 2;
			_hexLookup['3'] = 3;
			_hexLookup['4'] = 4;
			_hexLookup['5'] = 5;
			_hexLookup['6'] = 6;
			_hexLookup['7'] = 7;
			_hexLookup['8'] = 8;
			_hexLookup['9'] = 9;
			_hexLookup['a'] = 10;
			_hexLookup['b'] = 11;
			_hexLookup['c'] = 12;
			_hexLookup['d'] = 13;
			_hexLookup['e'] = 14;
			_hexLookup['f'] = 15;
			_hexLookup['A'] = 10;
			_hexLookup['B'] = 11;
			_hexLookup['C'] = 12;
			_hexLookup['D'] = 13;
			_hexLookup['E'] = 14;
			_hexLookup['F'] = 15;
		}

		public static int FromHex(string hex)
		{
			int value = 0;
			for(int i = 0; i < hex.Length; i++) {
				value <<= 4;
				value |= _hexLookup[hex[i]];
			}
			return value;
		}

		public static byte[] HexToArray(string hex)
		{
			hex = string.Join("", hex.Split(" ", StringSplitOptions.RemoveEmptyEntries).Select(x => x.Length % 2 == 1 ? ("0" + x) : x));
			byte[] result = new byte[hex.Length / 2];
			for(int i = 0; i < hex.Length; i += 2) {
				byte value = 0;
				value |= _hexLookup[hex[i]];
				value <<= 4;
				value |= _hexLookup[hex[i+1]];
				result[i / 2] = value;
			}
			return result;
		}

		public static short[] HexToArrayWithWildcards(string hex)
		{
			string[] sections = hex.Split("?");

			StringBuilder sb = new();
			for(int i = 0; i < sections.Length; i++) {
				sb.Append(string.Join("", sections[i].Split(" ", StringSplitOptions.RemoveEmptyEntries).Select(x => x.Length % 2 == 1 ? ("0" + x) : x)));
				if(i < sections.Length - 1) {
					sb.Append("??");
				}
			}

			hex = sb.ToString();

			short[] result = new short[hex.Length / 2];
			for(int i = 0; i < hex.Length; i += 2) {
				if(hex[i] == '?' && hex[i + 1] == '?') {
					result[i / 2] = -1;
				} else {
					byte value = 0;
					value |= _hexLookup[hex[i]];
					value <<= 4;
					value |= _hexLookup[hex[i + 1]];
					result[i / 2] = value;
				}
			}
			return result;
		}
	}
}
