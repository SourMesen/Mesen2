using System.Collections.Generic;

namespace Mesen.Debugger.Utilities
{
	public class NavigationHistory<T>
	{
		private List<T> _historyList = new List<T>();
		private int _historyPosition = -1;

		private void ClearForwardHistory()
		{
			_historyList.RemoveRange(_historyPosition + 1, _historyList.Count - _historyPosition - 1);
		}

		public void AddHistory(T location)
		{
			if(_historyList.Count - 1 > _historyPosition) {
				ClearForwardHistory();
			}

			if(_historyList.Count == 0 || !object.Equals(_historyList[^1], location)) {
				_historyList.Add(location);
				_historyPosition++;
			}
		}

		public bool CanGoBack()
		{
			return _historyPosition > 0;
		}

		public bool CanGoForward()
		{
			return _historyPosition < _historyList.Count - 1;
		}

		public T GoBack()
		{
			if(_historyPosition > 0) {
				_historyPosition--;
			}
			return _historyList[_historyPosition];
		}

		public T GoForward()
		{
			if(_historyPosition < _historyList.Count - 1) {
				_historyPosition++;
			}
			return _historyList[_historyPosition];
		}
	}
}
