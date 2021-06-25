using Avalonia;
using Avalonia.Controls;
using Avalonia.Media;
using Avalonia.Media.Imaging;
using Avalonia.Platform;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;

namespace Mesen.Debugger.ViewModels
{
	public class SpriteViewerViewModel : ViewModelBase
	{
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }

		[Reactive] public SpritePreviewModel? SelectedSprite { get; set; }

		//For designer
		public SpriteViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public SpriteViewerViewModel(CpuType cpuType, ConsoleType consoleType)
		{
			CpuType = cpuType;
			ConsoleType = consoleType;
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

		[Reactive] public Bitmap? SpritePreview { get; set; }

		public void Init(ref DebugSpriteInfo sprite)
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

			GCHandle handle = GCHandle.Alloc(sprite.SpritePreview, GCHandleType.Pinned);
			IntPtr previewPtr = handle.AddrOfPinnedObject();
			Bitmap originalPreview = new Bitmap(PixelFormat.Bgra8888, AlphaFormat.Premul, previewPtr, new PixelSize(Width, Height), new Vector(96, 96), Width * 4);
			
			int scale = 64 / Math.Max(Width, Height);
			SpritePreview = originalPreview.CreateScaledBitmap(new PixelSize(Width*scale, Height*scale), Avalonia.Visuals.Media.Imaging.BitmapInterpolationMode.Default);
			handle.Free();

			Visible = sprite.Visible;

			HorizontalMirror = sprite.HorizontalMirror;
			VerticalMirror = sprite.VerticalMirror;
			//flags += sprite.UseSecondTable ? "N" : "";
		}
	}

}
