using Mesen.Debugger.Utilities;
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
		private static List<Breakpoint> _temporaryBreakpoints = new List<Breakpoint>();
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
			if(_breakpoints.Remove(bp)) {
				DebugWorkspaceManager.AutoSave();
			}
			RefreshBreakpoints(bp);
		}

		public static void RemoveBreakpoints(IEnumerable<Breakpoint> breakpoints)
		{
			foreach(Breakpoint bp in breakpoints) {
				_breakpoints.Remove(bp);
			}
			RefreshBreakpoints(null);
		}

		public static void AddBreakpoint(Breakpoint bp)
		{
			if(!_breakpoints.Contains(bp)) {
				_breakpoints.Add(bp);
				DebugWorkspaceManager.AutoSave();
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
					CpuType = cpuType,
					BreakOnExec = true,
					BreakOnWrite = true,
					BreakOnRead = true
				};

				BreakpointManager.AddBreakpoint(bp);
			}
		}

		public static void AddTemporaryBreakpoint(Breakpoint bp)
		{
			_temporaryBreakpoints.Add(bp);
			SetBreakpoints();
		}

		public static void ClearTemporaryBreakpoints()
		{
			if(_temporaryBreakpoints.Count > 0) {
				_temporaryBreakpoints.Clear();
				SetBreakpoints();
			}
		}

		private static Breakpoint? GetMatchingBreakpoint(AddressInfo info, CpuType cpuType, Func<Breakpoint, bool> predicate)
		{
			Breakpoint? bp = Breakpoints.Where((bp) => predicate(bp) && bp.Matches((UInt32)info.Address, info.Type, cpuType)).FirstOrDefault();

			if(bp == null) {
				AddressInfo altAddr;
				if(info.Type.IsRelativeMemory()) {
					altAddr = DebugApi.GetAbsoluteAddress(info);
				} else {
					altAddr = DebugApi.GetRelativeAddress(info, cpuType);
				}

				if(altAddr.Address >= 0) {
					bp = Breakpoints.Where((bp) => predicate(bp) && bp.Matches((UInt32)altAddr.Address, altAddr.Type, cpuType)).FirstOrDefault();
				}
			}

			return bp;
		}

		public static Breakpoint? GetMatchingBreakpoint(AddressInfo info, CpuType cpuType, bool ignoreRangedRwBp = false)
		{
			return GetMatchingBreakpoint(info, cpuType, (bp) => !ignoreRangedRwBp || bp.IsSingleAddress || bp.BreakOnExec);
		}

		public static Breakpoint? GetMatchingForbidBreakpoint(AddressInfo info, CpuType cpuType)
		{
			return GetMatchingBreakpoint(info, cpuType, (bp) => bp.Forbid);
		}

		public static Breakpoint? GetMatchingBreakpoint(UInt32 startAddress, UInt32 endAddress, MemoryType memoryType)
		{
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
				DebugWorkspaceManager.AutoSave();
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

			Breakpoint? breakpoint = BreakpointManager.GetMatchingForbidBreakpoint(info, cpuType) ?? BreakpointManager.GetMatchingBreakpoint(info, cpuType, true);
			if(breakpoint != null) {
				BreakpointManager.RemoveBreakpoint(breakpoint);
			} else {
				bool execBreakpoint = true;
				bool readWriteBreakpoint = !info.Type.IsRomMemory() || info.Type.IsRelativeMemory();
				if(info.Type.SupportsCdl()) {
					CdlFlags cdlData = DebugApi.GetCdlData((uint)info.Address, 1, info.Type)[0];
					bool isCode = cdlData.HasFlag(CdlFlags.Code);
					bool isData = cdlData.HasFlag(CdlFlags.Data);
					if(isCode || isData) {
						readWriteBreakpoint = !isCode;
						execBreakpoint = isCode;
					}
				}

				breakpoint = new Breakpoint() {
					CpuType = cpuType,
					Enabled = true,
					BreakOnExec = execBreakpoint,
					BreakOnRead = readWriteBreakpoint,
					BreakOnWrite = readWriteBreakpoint,
					StartAddress = (UInt32)info.Address,
					EndAddress = (UInt32)info.Address
				};

				breakpoint.MemoryType = info.Type;
				BreakpointManager.AddBreakpoint(breakpoint);
			}
		}

		public static void ToggleForbidBreakpoint(AddressInfo addr, CpuType cpuType)
		{
			if(addr.Address < 0) {
				return;
			}

			Breakpoint? breakpoint = GetMatchingForbidBreakpoint(addr, cpuType);
			if(breakpoint != null) {
				BreakpointManager.RemoveBreakpoint(breakpoint);
			} else {
				breakpoint = new Breakpoint() {
					CpuType = cpuType,
					Enabled = true,
					Forbid = true,
					StartAddress = (UInt32)addr.Address,
					EndAddress = (UInt32)addr.Address
				};
				breakpoint.MemoryType = addr.Type;
				BreakpointManager.AddBreakpoint(breakpoint);
			}
		}

		public static void SetBreakpoints()
		{
			List<InteropBreakpoint> breakpoints = new List<InteropBreakpoint>();

			int id = 0;
			void toInteropBreakpoints(IEnumerable<Breakpoint> bpList)
			{
				foreach(Breakpoint bp in bpList) {
					if(_activeCpuTypes.Contains(bp.CpuType)) {
						breakpoints.Add(bp.ToInteropBreakpoint(id));
					}
					id++;
				}
			}

			toInteropBreakpoints(BreakpointManager.Breakpoints);
			toInteropBreakpoints(BreakpointManager.Asserts);
			toInteropBreakpoints(BreakpointManager._temporaryBreakpoints);

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
			} else if(breakpointId < _breakpoints.Count + Asserts.Count + _temporaryBreakpoints.Count) {
				return _temporaryBreakpoints[breakpointId - _breakpoints.Count - Asserts.Count];
			}
			return null;
		}
	}
}
