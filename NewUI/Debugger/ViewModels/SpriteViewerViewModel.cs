using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Platform;
using Mesen.Config;
using Mesen.Debugger.Controls;
using Mesen.Interop;
using Mesen.Utilities;
using Mesen.ViewModels;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.ViewModels
{
	public class SpriteViewerViewModel : ViewModelBase
	{
		public SpriteViewerConfig Config { get; }
		public RefreshTimingViewModel RefreshTiming { get; }
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		[Reactive] public SpritePreviewModel? SelectedSprite { get; set; }
		[Reactive] public DynamicTooltip? PreviewPanel { get; set; }

		public List<object> FileMenuActions { get; } = new();
		public List<object> ViewMenuActions { get; } = new();


		[Obsolete("For designer only")]
		public SpriteViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public SpriteViewerViewModel(CpuType cpuType, ConsoleType consoleType)
		{
			Config = ConfigManager.Config.Debug.SpriteViewer;
			RefreshTiming = new RefreshTimingViewModel(Config.RefreshTiming);
			CpuType = cpuType;
			ConsoleType = consoleType;

			this.WhenAnyValue(x => x.SelectedSprite).Subscribe(x => UpdateSelectionPreview());
		}

		public DynamicTooltip? GetPreviewPanel(SpritePreviewModel sprite, DynamicTooltip? existingTooltip)
		{
			TooltipEntries entries = existingTooltip?.Items ?? new();
			entries.AddPicture("Sprite", sprite.SpritePreview, (int)sprite.SpritePreviewZoom);
			entries.AddEntry("X, Y", sprite.X + ", " + sprite.Y);
			entries.AddEntry("Size", sprite.Width + "x" + sprite.Height);

			entries.AddEntry("Tile index", "$" + sprite.TileIndex.ToString("X2"));
			entries.AddEntry("Sprite index", sprite.SpriteIndex.ToString());
			entries.AddEntry("Visible", sprite.Visible);
			entries.AddEntry("Horizontal mirror", sprite.HorizontalMirror);
			entries.AddEntry("Vertical mirror", sprite.VerticalMirror);
			entries.AddEntry("Priority", sprite.Priority);

			if(existingTooltip != null) {
				return existingTooltip;
			} else {
				return new DynamicTooltip() { Items = entries };
			}
		}

		public void UpdateSelectionPreview()
		{
			if(SelectedSprite != null) {
				PreviewPanel = GetPreviewPanel(SelectedSprite, PreviewPanel);
			} else {
				PreviewPanel = null;
			}
		}
	}

	public class SpritePreviewModel : ViewModelBase
	{
		[Reactive] public int SpriteIndex { get; set; }
		[Reactive] public int X { get; set; }
		[Reactive] public int Y { get; set; }
		[Reactive] public int Width { get; set; }
		[Reactive] public int Height { get; set; }
		[Reactive] public int TileIndex { get; set; }
		[Reactive] public int Priority { get; set; }
		[Reactive] public int Palette { get; set; }
		[Reactive] public bool Visible { get; set; }
		[Reactive] public string Size { get; set; } = "";
		
		[Reactive] public bool HorizontalMirror { get; set; }
		[Reactive] public bool VerticalMirror { get; set; }

		[Reactive] public DynamicBitmap SpritePreview { get; set; } = new DynamicBitmap(new PixelSize(1, 1), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
		[Reactive] public double SpritePreviewZoom { get; set; }

		public unsafe void Init(ref DebugSpriteInfo sprite)
		{
			SpriteIndex = sprite.SpriteIndex;
			X = sprite.X;
			Y = sprite.Y;
			Width = sprite.Width;
			Height = sprite.Height;
			TileIndex = sprite.TileIndex;
			Priority = sprite.Priority;
			Palette = sprite.Palette;
			
			Size = sprite.Width + "x" + sprite.Height;

			fixed(UInt32* p = sprite.SpritePreview) {
				if(SpritePreview.PixelSize.Width != sprite.Width || SpritePreview.PixelSize.Height != sprite.Height) {
					SpritePreview = new DynamicBitmap(new PixelSize(Width, Height), new Vector(96, 96), PixelFormat.Bgra8888, AlphaFormat.Premul);
				}

				int spriteSize = Width * Height * sizeof(UInt32);
				using(var bitmapLock = SpritePreview.Lock()) {
					Buffer.MemoryCopy(p, (void*)bitmapLock.FrameBuffer.Address, spriteSize, spriteSize);
				}
			}
			
			SpritePreviewZoom = 32 / Math.Max(Width, Height);

			Visible = sprite.Visible;

			HorizontalMirror = sprite.HorizontalMirror;
			VerticalMirror = sprite.VerticalMirror;
			//flags += sprite.UseSecondTable ? "N" : "";
		}
	}

}
