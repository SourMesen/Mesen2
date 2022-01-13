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

		public DisassemblyToolViewModel DisassemblyTool { get; private set; }
		public StatusToolViewModel CpuStatusTool { get; private set; }
		public StatusToolViewModel PpuStatusTool { get; private set; }

		public DebuggerDockFactory(DebuggerWindowViewModel context)
		{
			_context = context;
			DisassemblyTool = new DisassemblyToolViewModel(_context.Disassembly);
			CpuStatusTool = new StatusToolViewModel() { Id = "CpuStatusTool", Title = "CPU Status" };
			PpuStatusTool = new StatusToolViewModel() { Id = "PpuStatusTool", Title = "PPU Status" };
		}

		public override IRootDock CreateLayout()
		{
			var codeDockables = CreateList<IDockable>(DisassemblyTool);
			if(_context.SourceView != null) {
				codeDockables.Add(_context.SourceView);
			}

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
								VisibleDockables = codeDockables
							},
							new ProportionalDockSplitter(),
							new ProportionalDock {
								Proportion = 0.40,
								Orientation = Orientation.Vertical,
								VisibleDockables = CreateList<IDockable>(
									new ToolDock {
										Proportion = double.NaN,
										VisibleDockables = CreateList<IDockable>(CpuStatusTool)
									},
									new ProportionalDockSplitter(),
									new ToolDock {
										Proportion = double.NaN,
										VisibleDockables = CreateList<IDockable>(PpuStatusTool)
									},
									new ProportionalDockSplitter(),
									new ToolDock {
										Proportion = 0.33,
										VisibleDockables = CreateList<IDockable>(_context.LabelList)
									}
								)
							}
						)
					},
					new ProportionalDockSplitter(),
					new ProportionalDock {
						Proportion = 0.25,
						Orientation = Orientation.Horizontal,
						VisibleDockables = CreateList<IDockable>(
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(_context.WatchList)
							},
							new ProportionalDockSplitter(),
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(_context.BreakpointList)
							},
							new ProportionalDockSplitter(),
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(_context.CallStack)
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

			this.DockableLocator = new Dictionary<string, Func<IDockable?>> { };

			base.InitLayout(layout);
		}
	}
}
