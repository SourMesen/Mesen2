using Avalonia.Controls.Selection;
using AvaloniaEdit.Editing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public static class SelectionModelExtensions
	{
		public static void SelectIndexes<T>(this SelectionModel<T> selection, IEnumerable<int> indexes, int elementCount)
		{
			selection.BeginBatchUpdate();
			foreach(int index in indexes) {
				if(index < elementCount) {
					selection.Select(index);
				} else {
					selection.SelectedIndex = elementCount - 1;
				}
			}
			selection.EndBatchUpdate();
		}
	}
}
