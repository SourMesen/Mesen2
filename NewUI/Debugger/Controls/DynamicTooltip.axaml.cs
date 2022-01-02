using Avalonia;
using Avalonia.Controls;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Utilities;
using Mesen.ViewModels;
using Mesen.Windows;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System.Collections.Generic;

namespace Mesen.Debugger.Controls
{
	public class DynamicTooltip : UserControl
	{
		public static readonly StyledProperty<TooltipEntries> ItemsProperty = AvaloniaProperty.Register<DynamicTooltip, TooltipEntries>(nameof(Items));

		public TooltipEntries Items
		{
			get { return GetValue(ItemsProperty); }
			set { SetValue(ItemsProperty, value); }
		}

		public DynamicTooltip()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}
	}

	public class TooltipEntry : ReactiveObject
	{
		[Reactive] public string Name { get; set; } = "";
		[Reactive] public object Value { get; set; } = "";

		public TooltipEntry(string name, object value)
		{
			Name = name;
			Value = value;
		}
	}

	public class TooltipEntries : List<TooltipEntry>
	{
		private Dictionary<string, TooltipEntry> _entries = new();

		public void AddPicture(string name, IImage source, double zoom, PixelRect? cropRect = null)
		{
			if(_entries.TryGetValue(name, out TooltipEntry? entry)) {
				if(entry.Value is TooltipPictureEntry picEntry) {
					picEntry.Zoom = zoom;

					if(picEntry.CropRect != null) {
						if(picEntry.CropRect != cropRect || picEntry.OriginalSource != source) {
							entry.Value = new TooltipPictureEntry(source, zoom, cropRect);
						}
					} else {
						picEntry.Source = source;
						picEntry.CropRect = cropRect;
					}
				} else {
					entry.Value = new TooltipPictureEntry(source, zoom, cropRect);
				}
			} else {
				entry = new TooltipEntry(name, new TooltipPictureEntry(source, zoom, cropRect));
				_entries[entry.Name] = entry;
				Add(entry);
			}
		}

		public void AddEntry(string name, object value)
		{
			if(_entries.TryGetValue(name, out TooltipEntry? entry)) {
				entry.Value = value;
			} else {
				entry = new TooltipEntry(name, value);
				_entries[entry.Name] = entry;
				Add(entry);
			}
		}
	}

	public class TooltipPictureEntry : ReactiveObject
	{
		[Reactive] public IImage Source { get; set; }
		[Reactive] public double Zoom { get; set; }
		[Reactive] public PixelRect? CropRect { get; set; }
		public IImage OriginalSource { get; }

		public TooltipPictureEntry(IImage src, double zoom, PixelRect? cropRect)
		{
			OriginalSource = src;
			if(cropRect != null) {
				Source = new DynamicCroppedBitmap(src, cropRect.Value);
			} else {
				Source = src;
			}
			Zoom = zoom;
			CropRect = cropRect;
		}
	}
}
