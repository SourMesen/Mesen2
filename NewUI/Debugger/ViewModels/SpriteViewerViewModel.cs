using Avalonia.Controls;
using Avalonia.Media;
using Mesen.Interop;
using Mesen.ViewModels;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;

namespace Mesen.Debugger.ViewModels
{
	public class SpriteViewerViewModel : ViewModelBase
	{
		public CpuType CpuType { get; }
		public ConsoleType ConsoleType { get; }
	
		[Reactive] public SpriteRowModel[] Sprites { get; set; } = new SpriteRowModel[0];

		//For designer
		public SpriteViewerViewModel() : this(CpuType.Cpu, ConsoleType.Snes) { }

		public SpriteViewerViewModel(CpuType cpuType, ConsoleType consoleType)
		{
			CpuType = cpuType;
			ConsoleType = consoleType;
		}

		public void UpdateSprites(DebugSpriteInfo[] newSprites)
		{
			if(Sprites.Length != newSprites.Length) {
				SpriteRowModel[] sprites = new SpriteRowModel[newSprites.Length];
				for(int i = 0; i < newSprites.Length; i++) {
					sprites[i] = new SpriteRowModel();
				}
				Sprites = sprites;
			}

			for(int i = 0; i < newSprites.Length; i++) {
				Sprites[i].Init(ref newSprites[i]);
			}
		}

		public class SpriteRowModel : ViewModelBase
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
			[Reactive] public ISolidColorBrush? Foreground { get; set; }
			[Reactive] public FontStyle FontStyle { get; set; }
			[Reactive] public string Size { get; set; } = "";
			[Reactive] public string Flags { get; set; } = "";

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
				
				Visible = sprite.Visible;
				FontStyle = FontStyle.Normal;
				Foreground = sprite.Visible ? Brushes.Black : Brushes.Gray;

				string flags = sprite.HorizontalMirror ? "H" : "";
				flags += sprite.VerticalMirror ? "V" : "";
				flags += sprite.UseSecondTable ? "N" : "";
				Flags = flags;
			}
		}
	}
}
