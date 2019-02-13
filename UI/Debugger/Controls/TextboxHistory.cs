using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger
{
	class TextboxHistory
	{
		private List<int> _historyList = new List<int>() { 0 };
		private int _historyPosition = 0;

		private void ClearForwardHistory()
		{
			_historyList.RemoveRange(_historyPosition + 1, _historyList.Count - _historyPosition - 1);
		}

		public void AddHistory(int lineIndex)
		{
			if(_historyList.Count - 1 > _historyPosition) {
				ClearForwardHistory();
			}

			if(_historyList[_historyList.Count-1] != lineIndex) {
				_historyList.Add(lineIndex);
				_historyPosition++;
			}
		}

		public int GoBack()
		{
			if(_historyPosition > 0) {
				_historyPosition--;
			}
			return _historyList[_historyPosition];
		}

		public int GoForward()
		{
			if(_historyPosition < _historyList.Count - 1) {
				_historyPosition++;
			}
			return _historyList[_historyPosition];
		}
	}
}
