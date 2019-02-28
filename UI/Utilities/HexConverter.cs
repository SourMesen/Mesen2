using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI
{
	class HexConverter
	{
		private static byte[] _hexLookup = new byte[256];

		static HexConverter()
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
	}
}
