using Avalonia.Data;
using Dock.Avalonia.Controls;
using Dock.Model;
using Dock.Model.ReactiveUI.Controls;
using Dock.Model.Core;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.ViewModels.DebuggerDock;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Dock.Model.Controls;
using Dock.Model.ReactiveUI;

namespace Mesen.Debugger
{
	public class DebuggerDockFactory : Factory
	{
		private DebuggerWindowViewModel _context;

		public BreakpointToolViewModel BreakpointListTool { get; private set; }
		public DisassemblyToolViewModel DisassemblyTool { get; private set; }
		public CpuStatusToolViewModel CpuStatusTool { get; private set; }
		public PpuStatusToolViewModel PpuStatusTool { get; private set; }

		public DebuggerDockFactory(DebuggerWindowViewModel context)
		{
			_context = context;
			BreakpointListTool = new BreakpointToolViewModel(_context.BreakpointList);
			DisassemblyTool = new DisassemblyToolViewModel(_context.Disassembly);
			CpuStatusTool = new CpuStatusToolViewModel(_context.SnesCpu);
			PpuStatusTool = new PpuStatusToolViewModel(_context.SnesPpu);
		}

		public override IRootDock CreateLayout()
		{
			var mainLayout = new ProportionalDock {
				Orientation = Orientation.Vertical,
				VisibleDockables = CreateList<IDockable>(
					new ProportionalDock {
						Proportion = 0.75,
						Orientation = Orientation.Horizontal,
						ActiveDockable = null,
						VisibleDockables = CreateList<IDockable>(
							new ToolDock {
								Proportion = 0.60,
								VisibleDockables = CreateList<IDockable>(DisassemblyTool)
							},
							new SplitterDockable(),
							new ProportionalDock {
								Proportion = 0.40,
								Orientation = Orientation.Vertical,
								VisibleDockables = CreateList<IDockable>(
									new ToolDock {
										Proportion = double.NaN,
										VisibleDockables = CreateList<IDockable>(CpuStatusTool)
									},
									new SplitterDockable(),
									new ToolDock {
										Proportion = double.NaN,
										VisibleDockables = CreateList<IDockable>(PpuStatusTool)
									},
									new SplitterDockable(),
									new ToolDock {
										Proportion = 0.33,
										VisibleDockables = CreateList<IDockable>(new DummyTool() { Id = "Labels", Title = "Labels" })
									}
								)
							}
						)
					},
					new SplitterDockable(),
					new ProportionalDock {
						Proportion = 0.25,
						Orientation = Orientation.Horizontal,
						VisibleDockables = CreateList<IDockable>(
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(new DummyTool() { Id = "Watch", Title = "Watch" })
							},
							new SplitterDockable(),
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(BreakpointListTool)
							},
							new SplitterDockable(),
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(new DummyTool() { Id = "CallStack", Title = "Call Stack" })
							}
						)
					}
				)
			};

			var mainView = new DebuggerDockViewModel {
				Id = "Main",
				Title = "Main",
				ActiveDockable = mainLayout,
				VisibleDockables = CreateList<IDockable>(mainLayout)
			};

			var root = CreateRootDock();

			root.Id = "Root";
			root.Title = "Root";
			root.ActiveDockable = mainView;
			root.DefaultDockable = mainView;
			root.VisibleDockables = CreateList<IDockable>(mainView);

			return root;
		}

		public override void InitLayout(IDockable layout)
		{
			this.ContextLocator = new Dictionary<string, Func<object>> {
			};

			this.HostWindowLocator = new Dictionary<string, Func<IHostWindow>> {
				[nameof(IDockWindow)] = () => new HostWindow()
			};

			this.DockableLocator = new Dictionary<string, Func<IDockable>> { };

			base.InitLayout(layout);
		}
	}

	public class DummyTool : Tool
	{
	}
}
