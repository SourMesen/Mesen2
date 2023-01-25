using Dock.Model.ReactiveUI.Controls;
using Dock.Model.Core;
using System;
using System.Collections.Generic;
using System.Windows.Input;

namespace Mesen.Debugger.Controls
{
	//TODO: This is a patch to prevent performance issues when opening debugger caused by exceptions
	//triggered in Dock because it expects children of DockControl to implement IDock
	public class MesenProportionalDockSplitter : ProportionalDockSplitter, IDock
	{
		public IList<IDockable>? VisibleDockables { get => null; set => throw new NotImplementedException(); }
		public IList<IDockable>? HiddenDockables { get => null; set => throw new NotImplementedException(); }
		public IList<IDockable>? PinnedDockables { get => null; set => throw new NotImplementedException(); }
		public IDockable? ActiveDockable { get => null; set => throw new NotImplementedException(); }
		public IDockable? DefaultDockable { get => null; set => throw new NotImplementedException(); }
		public IDockable? FocusedDockable { get => null; set => throw new NotImplementedException(); }
		public double Proportion { get => 1.0; set => throw new NotImplementedException(); }
		public DockMode Dock { get => DockMode.Top; set => throw new NotImplementedException(); }
		public bool IsActive { get => false; set => throw new NotImplementedException(); }
		public bool IsCollapsable { get => false; set => throw new NotImplementedException(); }

		public bool CanGoBack => false;
		public bool CanGoForward => false;
		public ICommand GoBack => null!;
		public ICommand GoForward => null!;
		public ICommand Navigate => null!;
		public ICommand Close => null!;
	}
}
