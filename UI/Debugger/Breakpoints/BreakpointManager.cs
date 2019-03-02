using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Forms;

namespace Mesen.GUI.Debugger
{
	public class BreakpointManager
	{
		public static event EventHandler BreakpointsChanged;

		private static List<Breakpoint> _breakpoints = new List<Breakpoint>();

		public static ReadOnlyCollection<Breakpoint> Breakpoints
		{
			get { return _breakpoints.ToList().AsReadOnly(); }
		}

		public static void RefreshBreakpoints(Breakpoint bp = null)
		{
			if(BreakpointsChanged != null) {
				BreakpointsChanged(bp, null);
			}

			SetBreakpoints();
		}

		public static void SetBreakpoints(List<Breakpoint> breakpoints)
		{
			_breakpoints = breakpoints.ToList();
			RefreshBreakpoints();
		}

		public static void EditBreakpoint(Breakpoint bp)
		{
			if(new frmBreakpoint(bp).ShowDialog() == DialogResult.OK) {
				if(!_breakpoints.Contains(bp)) {
					_breakpoints.Add(bp);
				}
				RefreshBreakpoints(bp);
			}
		}

		public static void RemoveBreakpoint(Breakpoint bp)
		{
			_breakpoints.Remove(bp);
			RefreshBreakpoints(bp);
		}

		public static void AddBreakpoint(Breakpoint bp)
		{
			_breakpoints.Add(bp);
			RefreshBreakpoints(bp);
		}

		public static Breakpoint GetMatchingBreakpoint(AddressInfo info)
		{
			return Breakpoints.Where((bp) => bp.Matches((UInt32)info.Address, info.Type)).FirstOrDefault();
		}

		public static Breakpoint GetMatchingBreakpoint(UInt32 startAddress, UInt32 endAddress, SnesMemoryType memoryType)
		{
			bool isAddressRange = startAddress != endAddress;
			return Breakpoints.Where((bp) =>
					bp.MemoryType == memoryType &&
					((!isAddressRange && bp.Address == startAddress) || (isAddressRange && bp.StartAddress == startAddress && bp.EndAddress == endAddress))
				).FirstOrDefault();
		}

		public static void EnableDisableBreakpoint(AddressInfo info)
		{
			Breakpoint breakpoint = BreakpointManager.GetMatchingBreakpoint(info);
			if(breakpoint != null) {
				breakpoint.SetEnabled(!breakpoint.Enabled);
			}
		}

		public static void ToggleBreakpoint(AddressInfo info)
		{
			if(info.Address < 0) {
				return;
			}

			Breakpoint breakpoint = BreakpointManager.GetMatchingBreakpoint(info);
			if(breakpoint != null) {
				BreakpointManager.RemoveBreakpoint(breakpoint);
			} else {
				breakpoint = new Breakpoint() {
					Enabled = true,
					BreakOnExec = true,
					Address = (UInt32)info.Address
				};

				if(info.Type != SnesMemoryType.PrgRom) {
					breakpoint.BreakOnRead = true;
					breakpoint.BreakOnWrite = true;
				}

				breakpoint.MemoryType = info.Type;
				BreakpointManager.AddBreakpoint(breakpoint);
			}
		}

		public static void SetBreakpoints()
		{
			List<InteropBreakpoint> breakpoints = new List<InteropBreakpoint>();
			for(int i = 0; i < Breakpoints.Count; i++) {
				breakpoints.Add(Breakpoints[i].ToInteropBreakpoint(i));
			}
			DebugApi.SetBreakpoints(breakpoints.ToArray(), (UInt32)breakpoints.Count);
		}
	}
}
