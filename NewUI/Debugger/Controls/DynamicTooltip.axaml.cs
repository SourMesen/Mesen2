using Avalonia;
using Avalonia.Controls;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace Mesen.Debugger.Controls
{
	public class DynamicTooltip : UserControl
	{
		public static readonly StyledProperty<TooltipEntries> ItemsProperty = AvaloniaProperty.Register<DynamicTooltip, TooltipEntries>(nameof(Items));
		public static readonly StyledProperty<int> FirstColumnWidthProperty = AvaloniaProperty.Register<DynamicTooltip, int>(nameof(FirstColumnWidth));

		public TooltipEntries Items
		{
			get { return GetValue(ItemsProperty); }
			set { SetValue(ItemsProperty, value); }
		}

		public int FirstColumnWidth
		{
			get { return GetValue(FirstColumnWidthProperty); }
			set { SetValue(FirstColumnWidthProperty, value); }
		}

		static DynamicTooltip()
		{
			ItemsProperty.Changed.AddClassHandler<DynamicTooltip>((x, e) => {
				x.ComputeColumnWidth();
			});
		}

		public DynamicTooltip()
		{
			InitializeComponent();
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		private void ComputeColumnWidth()
		{
			//TODO, remove hardcoded font name+size+weight
			int maxWidth = 0;
			var text = new FormattedText("", new Typeface("Microsoft Sans Serif", FontStyle.Normal, FontWeight.Bold), 11, TextAlignment.Left, TextWrapping.NoWrap, Size.Empty);
			foreach(var item in Items) {
				text.Text = item.Name;
				maxWidth = Math.Max(maxWidth, (int)text.Bounds.Width + 5);
			}
			FirstColumnWidth = maxWidth;
		}
	}

	public class TooltipEntry : ReactiveObject
	{
		[Reactive] public string Name { get; set; } = "";
		[Reactive] public object Value { get; set; } = "";
		[Reactive] public FontFamily Font { get; set; }

		public TooltipEntry(string name, object value, FontFamily? font = null)
		{
			Name = name;
			Value = value;
			Font = font ?? FontFamily.Default;
		}
	}

	public class TooltipEntries : List<TooltipEntry>, INotifyCollectionChanged
	{
		private Dictionary<string, TooltipEntry> _entries = new();
		private HashSet<string> _updatedKeys = new();

		public event NotifyCollectionChangedEventHandler? CollectionChanged;

		[Obsolete("Do not use")]
		public new void Add(TooltipEntry entry)
		{
			throw new NotImplementedException();
		}

		public void AddPicture(string name, IImage source, double zoom, PixelRect? cropRect = null)
		{
			_updatedKeys.Add(name);

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
				base.Insert(_updatedKeys.Count - 1, entry);
				CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
			}
		}

		public void AddEntry(string name, object value, FontFamily? font = null)
		{
			_updatedKeys.Add(name);

			if(_entries.TryGetValue(name, out TooltipEntry? entry)) {
				entry.Value = value;
			} else {
				entry = new TooltipEntry(name, value, font);
				_entries[entry.Name] = entry;
				base.Insert(_updatedKeys.Count - 1, entry);
				CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
			}
		}

		public void StartUpdate()
		{
			_updatedKeys = new();
		}

		public void EndUpdate()
		{
			bool updated = false;
			for(int i = Count - 1; i >= 0; i--) {
				if(!_updatedKeys.Contains(this[i].Name)) {
					_entries.Remove(this[i].Name);
					RemoveAt(i);
					updated = true;
				}
			}

			if(updated) {
				CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
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

	public class TooltipColorEntry : ReactiveObject
	{
		[Reactive] public UInt32[] Color { get; set; }

		public TooltipColorEntry(UInt32 color)
		{
			Color = new UInt32[1];
			Color[0] = color;
		}
	}
}
