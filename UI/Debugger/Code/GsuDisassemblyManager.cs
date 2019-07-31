using Mesen.GUI.Debugger.Controls;
using Mesen.GUI.Debugger.Integration;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.GUI.Debugger.Code
{
	public class GsuDisassemblyManager : CpuDisassemblyManager
	{
		public override CpuType CpuType { get { return CpuType.Gsu; } }
		public override SnesMemoryType RelativeMemoryType { get { return SnesMemoryType.GsuMemory; } }
		public override int AddressSize { get { return 6; } }
		public override int ByteCodeSize { get { return 3; } }
		public override bool AllowSourceView { get { return false; } }

		public override void RefreshCode(DbgImporter symbolProvider, DbgImporter.FileInfo file)
		{
			this._provider = new CodeDataProvider(CpuType.Gsu);
		}

		protected override int GetFullAddress(int address, int length)
		{
			return address;
		}
	}
}
