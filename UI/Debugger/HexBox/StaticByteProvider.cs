using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Be.Windows.Forms
{
	public class StaticByteProvider : IByteProvider
	{
		/// <summary>
		/// Contains information about changes.
		/// </summary>
		bool _hasChanges;
		/// <summary>
		/// Contains a byte collection.
		/// </summary>
		byte[] _bytes;

		/// <summary>
		/// Initializes a new instance of the DynamicByteProvider class.
		/// </summary>
		/// <param name="data"></param>
		public StaticByteProvider(byte[] data)
		{
			_bytes = data;
		}

		public void SetData(byte[] data)
		{
			_bytes = data;
		}

		/// <summary>
		/// Raises the Changed event.
		/// </summary>
		void OnChanged(EventArgs e)
		{
			_hasChanges = true;

			if(Changed != null)
				Changed(this, e);
		}

		/// <summary>
		/// Raises the LengthChanged event.
		/// </summary>
		void OnLengthChanged(EventArgs e)
		{
			if(LengthChanged != null)
				LengthChanged(this, e);
		}

		/// <summary>
		/// Gets the byte collection.
		/// </summary>
		public byte[] Bytes
		{
			get { return _bytes; }
		}

		#region IByteProvider Members
		/// <summary>
		/// True, when changes are done.
		/// </summary>
		public bool HasChanges()
		{
			return _hasChanges;
		}

		/// <summary>
		/// Applies changes.
		/// </summary>
		public void ApplyChanges()
		{
			_hasChanges = false;
		}

		/// <summary>
		/// Occurs, when the write buffer contains new changes.
		/// </summary>
		public event EventHandler Changed;

		/// <summary>
		/// Occurs, when InsertBytes or DeleteBytes method is called.
		/// </summary>
		public event EventHandler LengthChanged;

		public delegate void ByteChangedHandler(int byteIndex, byte newValue, byte oldValue);
		public event ByteChangedHandler ByteChanged;

		public delegate void BytesChangedHandler(int byteIndex, byte[] values);
		public event BytesChangedHandler BytesChanged;

		/// <summary>
		/// Reads a byte from the byte collection.
		/// </summary>
		/// <param name="index">the index of the byte to read</param>
		/// <returns>the byte</returns>
		public byte ReadByte(long index)
		{
			if(_partialPos == index) {
				return _partialValue;
			} else {
				return _bytes[(int)index];
			}
		}

		/// <summary>
		/// Write a byte into the byte collection.
		/// </summary>
		/// <param name="index">the index of the byte to write.</param>
		/// <param name="value">the byte</param>
		public void WriteByte(long index, byte value)
		{
			if(index == _partialPos) {
				_partialPos = -1;
			}

			if(_bytes[(int)index] != value) {
				ByteChanged?.Invoke((int)index, value, _bytes[(int)index]);
				_bytes[(int)index] = value;
				OnChanged(EventArgs.Empty);
			}
		}

		public void WriteBytes(long index, byte[] values)
		{
			_partialPos = -1;
			BytesChanged?.Invoke((int)index, values);
			for(int i = 0; i < values.Length && index + i < _bytes.Length; i++) {
				_bytes[(int)index + i] = values[i];
			}
		}

		long _partialPos = -1;
		byte _partialValue = 0;
		public void PartialWriteByte(long index, byte value)
		{
			//Wait for a full byte to be written
			_partialPos = index;
			_partialValue = value;
		}

		public void CommitWriteByte()
		{
			if(_partialPos >= 0) {
				WriteByte(_partialPos, _partialValue);
				_partialPos = -1;
			}
		}

		public void DeleteBytes(long index, long length)
		{
		}

		public void InsertBytes(long index, byte[] bs)
		{
		}

		/// <summary>
		/// Gets the length of the bytes in the byte collection.
		/// </summary>
		public long Length
		{
			get
			{
				return _bytes.Length;
			}
		}

		/// <summary>
		/// Returns true
		/// </summary>
		public virtual bool SupportsWriteByte()
		{
			return true;
		}

		/// <summary>
		/// Returns true
		/// </summary>
		public virtual bool SupportsInsertBytes()
		{
			return false;
		}

		/// <summary>
		/// Returns true
		/// </summary>
		public virtual bool SupportsDeleteBytes()
		{
			return false;
		}
		#endregion

	}
}
