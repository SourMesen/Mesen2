using System;
using System.Linq;

namespace Mesen.Debugger.Integration
{
	public static class DbgReader
	{
		public delegate void ReadEntryCallback(ref ReadOnlySpan<char> name, ref ReadOnlySpan<char> data);
		
		public static void ReadEntry(string row, ReadEntryCallback callback)
		{
			bool readingTag = false;
			bool readingData = false;
			int startIndex = 0;
			ReadOnlySpan<char> tagName = new();

			for(int i = 0; i < row.Length + 1; i++) {
				char c = i < row.Length ? row[i] : ',';
				switch(c) {
					case '\t':
					case ' ':
					case ',':
						if(readingData) {
							ReadOnlySpan<char> tagData = row.AsSpan(startIndex, i - startIndex);
							callback(ref tagName, ref tagData);
						}
						readingData = false;
						readingTag = true;
						startIndex = i + 1;
						break;

					case '=':
						if(readingTag) {
							readingTag = false;
							tagName = row.AsSpan(startIndex, i - startIndex);
							readingData = true;
							startIndex = i + 1;
						}
						break;
				}
			}
		}

		public static int[] ReadIntArray(ReadOnlySpan<char> data)
		{
			int count = data.Count('+') + 1;
			int[] spanIds = new int[count];
			foreach(Range range in new SplitEnumerator(data, '+')) {
				spanIds[count - 1] = Int32.Parse(data.Slice(range.Start.Value, range.End.Value - range.Start.Value));
				count--;
			}

			return spanIds;
		}

		private ref struct SplitEnumerator
		{
			private readonly ReadOnlySpan<char> _buffer;
			private readonly char _separator;

			private int _start;
			private int _end;
			private int _nextStart;

			public SplitEnumerator GetEnumerator() => this;
			public Range Current => new Range(_start, _end);

			public SplitEnumerator(ReadOnlySpan<char> span, char separator)
			{
				_buffer = span;
				_separator = separator;
				_start = 0;
				_end = 0;
				_nextStart = 0;
			}

			public bool MoveNext()
			{
				if(_nextStart > _buffer.Length) {
					return false;
				}

				ReadOnlySpan<char> slice = _buffer.Slice(_nextStart);
				_start = _nextStart;

				int separatorIndex = slice.IndexOf(_separator);
				int elementLength = (separatorIndex != -1 ? separatorIndex : slice.Length);

				_end = _start + elementLength;
				_nextStart = _end + 1;
				return true;
			}
		}
	}

	public static class SpanExtensions
	{
		public static bool IsEqual(this ReadOnlySpan<char> span, string str)
		{
			return span.SequenceEqual(str.AsSpan());
		}

		public static int Count(this ReadOnlySpan<char> span, char c)
		{
			int count = 0;
			for(int i = 0; i < span.Length; i++) {
				if(span[i] == '+') {
					count++;
				}
			}
			return count;
		}
	}
}
