using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities
{
	public class SwappableList<T> : List<T>, INotifyCollectionChanged
	{
		public event NotifyCollectionChangedEventHandler? CollectionChanged;

		public void Swap(IEnumerable<T> newList)
		{
			Clear();
			AddRange(newList);
			CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
		}
	}
}
