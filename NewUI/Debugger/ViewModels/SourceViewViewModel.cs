using Avalonia;
using Avalonia.Threading;
using Dock.Model.ReactiveUI.Controls;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Debugger.Integration;
using Mesen.Debugger.Views;
using Mesen.Interop;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Linq;
using System.Collections.Generic;
using System.Threading.Tasks;
using Mesen.ViewModels;

namespace Mesen.Debugger.ViewModels;

public class SourceViewViewModel : ViewModelBase
{
	public ISymbolProvider SymbolProvider { get; set; }
	
	public List<SourceFileInfo> SourceFiles { get; }
	[Reactive] public SourceFileInfo? SelectedFile { get; set; }
	public FontConfig Font { get; }

	public CpuType CpuType { get; }

	private double _scrollPosition = 0;
	private int? _activeAddress = null;
	private MesenTextEditor? _editor = null;
	private ActiveLineBackgroundRenderer _activeLineRenderer;

	public SourceViewViewModel(ISymbolProvider symbolProvider, CpuType cpuType)
	{
		CpuType = cpuType;
		SymbolProvider = symbolProvider;
		Font = ConfigManager.Config.Debug.Font;

		SourceFiles = SymbolProvider.SourceFiles.Where(f => f.Data.Length > 0 && !f.Name.EndsWith(".chr", StringComparison.OrdinalIgnoreCase)).ToList();
		SourceFiles.Sort((a, b) => a.ToString().CompareTo(b.ToString()));
		if(SourceFiles.Count > 0) {
			SelectedFile = SourceFiles[0];
		}

		_activeLineRenderer = new ActiveLineBackgroundRenderer(this);

		this.WhenAnyValue(x => x.SelectedFile).Subscribe(x => {
			_scrollPosition = 0;
			UpdateEditor();
		});
	}

	public void Refresh(int? activeAddress)
	{
		_activeAddress = activeAddress;

		if(activeAddress != null) {
			AddressInfo absAddress = DebugApi.GetAbsoluteAddress(new AddressInfo() { Address = activeAddress.Value, Type = CpuType.ToMemoryType() });
			SourceCodeLocation? location = SymbolProvider.GetSourceCodeLineInfo(absAddress);
			if(location != null) {
				SelectedFile = location.Value.File;
				ScrollToLocation(location.Value);
			}
		}
		_activeLineRenderer.SetActiveAddress(activeAddress);

		_editor?.TextArea.LeftMargins[0].InvalidateVisual();
		_editor?.TextArea.TextView.InvalidateVisual();
	}

	private void ScrollToLocation(SourceCodeLocation location)
	{
		Task.Run(() => {
			System.Threading.Thread.Sleep(30);
			Dispatcher.UIThread.Post(() => {
				_editor?.ScrollLineToMiddle(location.LineNumber);
			});
		});
	}

	public void SetEditor(MesenTextEditor? editor)
	{
		_editor = editor;
		if(_editor != null) {
			_editor.TextEditorReady += _editor_TextEditorReady;
			UpdateEditor();
		}
	}

	private void _editor_TextEditorReady(object? sender, EventArgs e)
	{
		if(_editor != null) {
			_editor.VerticalScrollBarValue = _scrollPosition;
			Refresh(_activeAddress);
		}
	}

	public void UpdateEditor()
	{
		if(_editor == null) {
			return;
		}

		_editor.SyntaxHighlighting = SelectedFile?.IsAssembly == true ? MesenTextEditor.Asm6502Highlighting : null;
		_editor.TextArea.TextView.BackgroundRenderers.Clear();
		_editor.TextArea.TextView.BackgroundRenderers.Add(_activeLineRenderer);
		_editor.TextArea.LeftMargins.Clear();
		_editor.TextArea.LeftMargins.Add(new LineNumberMargin(this));
		_editor.HorizontalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Visible;
		_editor.VerticalScrollBarVisibility = Avalonia.Controls.Primitives.ScrollBarVisibility.Visible;
	}

	public void SaveScrollPosition()
	{
		_scrollPosition = _editor?.VerticalScrollBarValue ?? 0;
	}
}
