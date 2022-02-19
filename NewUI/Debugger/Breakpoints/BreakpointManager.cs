using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;

namespace Mesen.Debugger
{
	public static class BreakpointManager
	{
		public static event EventHandler? BreakpointsChanged;

		private static List<Breakpoint> _breakpoints = new List<Breakpoint>();
		private static HashSet<CpuType> _activeCpuTypes = new HashSet<CpuType>();

		public static ReadOnlyCollection<Breakpoint> Breakpoints
		{
			get { return _breakpoints.ToList().AsReadOnly(); }
		}

		public static List<Breakpoint> Asserts { internal get; set; } = new List<Breakpoint>();

		public static List<Breakpoint> GetBreakpoints(CpuType cpuType)
		{
			List<Breakpoint> breakpoints = new List<Breakpoint>();
			foreach(Breakpoint bp in _breakpoints) {
				if(bp.CpuType == cpuType) {
					breakpoints.Add(bp);
				}
			}
			return breakpoints;
		}

		public static void AddCpuType(CpuType cpuType)
		{
			_activeCpuTypes.Add(cpuType);
			SetBreakpoints();
		}

		public static void RemoveCpuType(CpuType cpuType)
		{
			_activeCpuTypes.Remove(cpuType);
			SetBreakpoints();
		}

		public static void RefreshBreakpoints(Breakpoint? bp = null)
		{
			BreakpointsChanged?.Invoke(bp, EventArgs.Empty);
			SetBreakpoints();
		}

		public static void ClearBreakpoints()
		{
			_breakpoints = new();
			RefreshBreakpoints();
		}

		public static void AddBreakpoints(List<Breakpoint> breakpoints)
		{
			_breakpoints.AddRange(breakpoints);
			RefreshBreakpoints();
		}

		public static void RemoveBreakpoint(Breakpoint bp)
		{
			_breakpoints.Remove(bp);
			RefreshBreakpoints(bp);
		}

		public static void AddBreakpoint(Breakpoint bp)
		{
			if(!_breakpoints.Contains(bp)) {
				_breakpoints.Add(bp);
			}
			RefreshBreakpoints(bp);
		}

		public static void AddBreakpoint(AddressInfo addr, CpuType cpuType)
		{
			if(BreakpointManager.GetMatchingBreakpoint(addr, cpuType) == null) {
				Breakpoint bp = new Breakpoint() {
					StartAddress = (uint)addr.Address,
					EndAddress = (uint)addr.Address,
					MemoryType = addr.Type,
					BreakOnExec = true,
					BreakOnWrite = true,
					BreakOnRead = true
				};

				BreakpointManager.AddBreakpoint(bp);
			}
		}

		public static Breakpoint? GetMatchingBreakpoint(AddressInfo info, CpuType cpuType)
		{
			Breakpoint? bp = Breakpoints.Where((bp) => bp.Matches((UInt32)info.Address, info.Type, cpuType)).FirstOrDefault();

			if(bp == null) {
				AddressInfo altAddr;
				if(info.Type.IsRelativeMemory()) {
					altAddr = DebugApi.GetAbsoluteAddress(info);
				} else {
					altAddr = DebugApi.GetRelativeAddress(info, cpuType);
				}

				if(altAddr.Address >= 0) {
					bp = Breakpoints.Where((bp) => bp.Matches((UInt32)altAddr.Address, altAddr.Type, cpuType)).FirstOrDefault();
				}
			}

			return bp;
		}

		public static Breakpoint? GetMatchingBreakpoint(UInt32 startAddress, UInt32 endAddress, MemoryType memoryType)
		{
			bool isAddressRange = startAddress != endAddress;
			return Breakpoints.Where((bp) =>
					bp.MemoryType == memoryType &&
					bp.StartAddress == startAddress && bp.EndAddress == endAddress
				).FirstOrDefault();
		}

		public static bool EnableDisableBreakpoint(AddressInfo info, CpuType cpuType)
		{
			Breakpoint? breakpoint = BreakpointManager.GetMatchingBreakpoint(info, cpuType);
			if(breakpoint != null) {
				breakpoint.Enabled = !breakpoint.Enabled;
				RefreshBreakpoints();
				return true;
			}
			return false;
		}

		public static void ToggleBreakpoint(AddressInfo info, CpuType cpuType)
		{
			if(info.Address < 0) {
				return;
			}

			Breakpoint? breakpoint = BreakpointManager.GetMatchingBreakpoint(info, cpuType);
			if(breakpoint != null) {
				BreakpointManager.RemoveBreakpoint(breakpoint);
			} else {
				breakpoint = new Breakpoint() {
					CpuType = cpuType,
					Enabled = true,
					BreakOnExec = true,
					StartAddress = (UInt32)info.Address,
					EndAddress = (UInt32)info.Address
				};

				breakpoint.MemoryType = info.Type;
				BreakpointManager.AddBreakpoint(breakpoint);
			}
		}

		public static void SetBreakpoints()
		{
			List<InteropBreakpoint> breakpoints = new List<InteropBreakpoint>();

			ReadOnlyCollection<Breakpoint> userBreakpoints = BreakpointManager.Breakpoints;
			for(int i = 0; i < userBreakpoints.Count; i++) {
				if(_activeCpuTypes.Contains(userBreakpoints[i].CpuType)) {
					breakpoints.Add(userBreakpoints[i].ToInteropBreakpoint(breakpoints.Count));
				}
			}

			List<Breakpoint> assertBreakpoints = BreakpointManager.Asserts;
			for(int i = 0; i < assertBreakpoints.Count; i++) {
				if(_activeCpuTypes.Contains(assertBreakpoints[i].CpuType)) {
					breakpoints.Add(assertBreakpoints[i].ToInteropBreakpoint(breakpoints.Count));
				}
			}

			DebugApi.SetBreakpoints(breakpoints.ToArray(), (UInt32)breakpoints.Count);
		}

		public static Breakpoint? GetBreakpointById(int breakpointId)
		{
			if(breakpointId < 0) {
				return null;
			} else if(breakpointId < _breakpoints.Count) {
				return _breakpoints[breakpointId];
			} else if(breakpointId < _breakpoints.Count + Asserts.Count) {
				return Asserts[breakpointId - _breakpoints.Count];
			}
			return null;
		}
	}
}
