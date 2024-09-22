using Avalonia.Data;
using Dock.Avalonia.Controls;
using Dock.Model;
using Dock.Model.Core;
using Mesen.Debugger.ViewModels;
using Mesen.Debugger.ViewModels.DebuggerDock;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Dock.Model.Controls;
using Mesen.Debugger.StatusViews;
using Mesen.Debugger.Controls;
using Dock.Model.Mvvm;
using Dock.Model.Mvvm.Controls;

namespace Mesen.Debugger
{
	public class DebuggerDockFactory : Factory
	{
		public ToolContainerViewModel<DisassemblyViewModel> DisassemblyTool { get; private set; }
		public ToolContainerViewModel<SourceViewViewModel> SourceViewTool { get; private set; }
		public ToolContainerViewModel<BaseConsoleStatusViewModel> StatusTool { get; private set; }
		public ToolContainerViewModel<BreakpointListViewModel> BreakpointListTool { get; private set; }
		public ToolContainerViewModel<WatchListViewModel> WatchListTool { get; private set; }
		public ToolContainerViewModel<CallStackViewModel> CallStackTool { get; private set; }
		public ToolContainerViewModel<LabelListViewModel> LabelListTool { get; private set; }
		public ToolContainerViewModel<FunctionListViewModel> FunctionListTool { get; private set; }
		public ToolContainerViewModel<FindResultListViewModel> FindResultListTool { get; private set; }
		public ToolContainerViewModel<ControllerListViewModel> ControllerListTool { get; private set; }

		private DockEntryDefinition? _savedRootDef;

		public DebuggerDockFactory(DockEntryDefinition? savedRootDef)
		{
			DisassemblyTool = new("Disassembly");
			DisassemblyTool.CanClose = false;
			SourceViewTool = new("Source View");
			SourceViewTool.CanClose = false;

			StatusTool = new("Status");
			BreakpointListTool = new("Breakpoints");
			WatchListTool = new("Watch");
			CallStackTool = new("Call Stack");
			LabelListTool = new("Labels");
			FunctionListTool = new("Functions");
			FindResultListTool = new("Find Results");
			ControllerListTool = new("Controllers");

			_savedRootDef = savedRootDef;
		}

		public override IRootDock CreateLayout()
		{
			if(_savedRootDef != null) {
				//Restore previous layout
				try {
					if(FromDockDefinition(_savedRootDef) is IRootDock savedRootLayout) {
						return savedRootLayout;
					}
				} catch {
					//Reset layout if any error occurs
				}
			}

			return GetDefaultLayout();
		}

		public IRootDock GetDefaultLayout()
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
								VisibleDockables = CreateList<IDockable>(DisassemblyTool, SourceViewTool)
							},
							new ProportionalDockSplitter(),
							new ProportionalDock {
								Proportion = 0.40,
								Orientation = Orientation.Vertical,
								VisibleDockables = CreateList<IDockable>(
									new ToolDock {
										Proportion = 0.5,
										VisibleDockables = CreateList<IDockable>(StatusTool)
									},
									new ProportionalDockSplitter(),
									new ToolDock {
										Proportion = 0.5,
										VisibleDockables = CreateList<IDockable>(LabelListTool, FunctionListTool, FindResultListTool, ControllerListTool)
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
								VisibleDockables = CreateList<IDockable>(WatchListTool)
							},
							new ProportionalDockSplitter(),
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(BreakpointListTool)
							},
							new ProportionalDockSplitter(),
							new ToolDock {
								Proportion = 0.33,
								VisibleDockables = CreateList<IDockable>(CallStackTool)
							}
						)
					}
				)
			};

			var root = CreateRootDock();
			root.ActiveDockable = mainLayout;
			root.DefaultDockable = mainLayout;
			root.VisibleDockables = CreateList<IDockable>(mainLayout);
			return root;
		}

		public override IProportionalDockSplitter CreateProportionalDockSplitter()
		{
			return new ProportionalDockSplitter();
		}

		public override void InitLayout(IDockable layout)
		{
			this.ContextLocator = new Dictionary<string, Func<object?>> {
			};

			this.HostWindowLocator = new Dictionary<string, Func<IHostWindow?>> {
				[nameof(IDockWindow)] = () => new HostWindow()
			};

			this.DockableLocator = new Dictionary<string, Func<IDockable?>> { };

			base.InitLayout(layout);
		}

		public DockEntryDefinition ToDockDefinition(IDockable dockable)
		{
			DockEntryDefinition entry = new();
			if(dockable is ProportionalDockSplitter) {
				entry.Type = DockEntryType.Splitter;
			} else if(dockable is IDock dock) {
				if(dock is IRootDock) {
					entry.Type = DockEntryType.Root;
				} else if(dock is IProportionalDock propDock) {
					entry.Type = DockEntryType.ProportionalDock;
					entry.Orientation = propDock.Orientation;
				} else {
					entry.Type = DockEntryType.ToolDock;
				}
				entry.Name = dock.Title;
				entry.Proportion = double.IsNaN(dock.Proportion) ? 0 : dock.Proportion;
				entry.Children = new();
				if(dock.VisibleDockables != null) {
					if(dock is IProportionalDock propDock && dock.VisibleDockables.Count == 1) {
						//Remove empty proportional docks (these seem to get created when moving things around)
						DockEntryDefinition innerEntry = ToDockDefinition(dock.VisibleDockables[0]);

						//Keep the outer layer's proportion (inner will be set to 1, which will cause issues)
						innerEntry.Proportion = entry.Proportion;
						return innerEntry;
					}

					if(dock.ActiveDockable != null) {
						int index = dock.VisibleDockables.IndexOf(dock.ActiveDockable);
						if(index >= 0) {
							entry.SelectedIndex = index;
						}
					}
					foreach(IDockable child in dock.VisibleDockables) {
						entry.Children.Add(ToDockDefinition(child));
					}
				}
			} else if(dockable is ITool tool) {
				entry.Type = DockEntryType.Tool;
				entry.Name = tool.Title;
				entry.ToolTypeName = tool.GetType().GetGenericArguments()[0].Name;
			}

			return entry;
		}

		public IDockable? FromDockDefinition(DockEntryDefinition def)
		{
			IDockable? dockable = null;
			switch(def.Type) {
				case DockEntryType.Splitter: return CreateProportionalDockSplitter();
				case DockEntryType.Root: dockable = CreateRootDock(); break;
				case DockEntryType.ToolDock: dockable = CreateToolDock(); break;
				
				case DockEntryType.ProportionalDock: {
					IProportionalDock propDock = CreateProportionalDock();
					propDock.Orientation = def.Orientation;
					dockable = propDock;
					break;
				}

				case DockEntryType.Tool:
					switch(def.ToolTypeName) {
						case nameof(DisassemblyViewModel): return DisassemblyTool;
						case nameof(SourceViewViewModel): return SourceViewTool;
						case nameof(BaseConsoleStatusViewModel): return StatusTool;
						case nameof(BreakpointListViewModel): return BreakpointListTool;
						case nameof(WatchListViewModel): return WatchListTool;
						case nameof(CallStackViewModel): return CallStackTool;
						case nameof(LabelListViewModel): return LabelListTool;
						case nameof(FunctionListViewModel): return FunctionListTool;
						case nameof(FindResultListViewModel): return FindResultListTool;
						case nameof(ControllerListViewModel): return ControllerListTool;
					}
					break;
			}

			IDock? dock = dockable as IDock;
			if(dock != null && def.Proportion != 0) {
				dock.Proportion = def.Proportion;
			}

			if(dock != null && def.Children != null) {
				dock.VisibleDockables = CreateList<IDockable>();
				foreach(DockEntryDefinition childDef in def.Children) {
					IDockable? child = FromDockDefinition(childDef);
					if(child != null) {
						dock.VisibleDockables.Add(child);
					}
				}
				dock.ActiveDockable = def.SelectedIndex < dock.VisibleDockables.Count ? dock.VisibleDockables[def.SelectedIndex] : dock.VisibleDockables[0];
				dock.DefaultDockable = dock.VisibleDockables[0];
			}

			return dockable;
		}
	}

	public enum DockEntryType
	{
		Root,
		ProportionalDock,
		ToolDock,
		Splitter,
		Tool
	}

	public class DockEntryDefinition
	{
		public DockEntryType Type { get; set; }
		public double Proportion { get; set; } = 0;
		public Orientation Orientation { get; set; } = Orientation.Horizontal;
		public string Name { get; set; } = "";
		public string ToolTypeName { get; set; } = "";
		public int SelectedIndex { get; set; } = 0;
		public List<DockEntryDefinition>? Children { get; set; }
	}
}
