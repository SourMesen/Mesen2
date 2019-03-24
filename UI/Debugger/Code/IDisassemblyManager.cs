using Mesen.GUI.Debugger.Controls;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public interface IDisassemblyManager
	{
		ICodeDataProvider Provider { get; }

		void RefreshCode();
		void ToggleBreakpoint(int lineIndex);
	}
}
