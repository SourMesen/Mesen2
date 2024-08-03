using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Layout;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Mesen.Config;
using Mesen.Interop;
using Mesen.Localization;
using Mesen.Utilities;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.Globalization;

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
			if(Application.Current?.Resources["MesenFont"] is FontFamily mesenFont && Application.Current?.Resources["MesenFontSize"] is double fontSize) {
				int maxWidth = 0;
				Typeface typeface = new Typeface(mesenFont, FontStyle.Normal, FontWeight.Bold);
				foreach(var item in Items) {
					if(item is TooltipSeparator || item is CustomTooltipEntry) {
						continue;
					}

					var text = new FormattedText(item.Name, CultureInfo.CurrentCulture, FlowDirection.LeftToRight, typeface, fontSize, null);
					maxWidth = Math.Max(maxWidth, (int)text.Width + 5);
				}
				FirstColumnWidth = maxWidth;
			}
		}

		private void TextBox_Tapped(object? sender, TappedEventArgs e)
		{
			if(sender is TextBox txt) {
				txt.SelectAll();
			}
		}

		private void TextBox_ContextRequested(object? sender, ContextRequestedEventArgs e)
		{
			if(sender is TextBox txt) {
				txt.SelectAll();
			}
		}

		private void TextBox_LostFocus(object? sender, RoutedEventArgs e)
		{
			if(sender is TextBox txt) {
				txt.ClearSelection();
			}
		}
	}

	public class TooltipEntry : ReactiveObject
	{
		[Reactive] public string Name { get; set; } = "";
		[Reactive] public object Value { get; set; } = "";
		[Reactive] public bool UseMonoFont { get; set; } = false;

		public virtual VerticalAlignment VerticalAlignment => Value is bool ? VerticalAlignment.Center : VerticalAlignment.Top;

		public TooltipEntry(string name, object value, bool useMonoFont = false)
		{
			Name = name;
			Value = value;
			UseMonoFont = useMonoFont;
		}
	}

	public class CustomTooltipEntry : TooltipEntry
	{
		public CustomTooltipEntry(string name, object value, bool useMonoFont = false) : base(name, value, useMonoFont)
		{
		}
	}

	public class TooltipSeparator : TooltipEntry
	{
		[Reactive] public bool Hidden { get; set; } = false;

		public TooltipSeparator(string name) : base(name, false, false)
		{
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

		public void AddCustomEntry(string name, Control value)
		{
			_updatedKeys.Add(name);
			if(_entries.TryGetValue(name, out TooltipEntry? entry)) {
				entry.Value = value;
			} else {
				entry = new CustomTooltipEntry(name, value);
				_entries[entry.Name] = entry;
				base.Insert(_updatedKeys.Count - 1, entry);
				CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
			}
		}

		public void AddSeparator(string name)
		{
			_updatedKeys.Add(name);
			if(!_entries.TryGetValue(name, out _)) {
				TooltipEntry? entry = new TooltipSeparator(name);
				_entries[name] = entry;
				base.Insert(_updatedKeys.Count - 1, new TooltipSeparator(name));
				CollectionChanged?.Invoke(this, new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
			}
		}

		public void AddEntry(string name, NullableBoolean value)
		{
			if(value != NullableBoolean.Undefined) {
				AddEntry(name, value == NullableBoolean.True);
			}
		}
		
		public void AddEntry(string name, object value, bool useMonoFont = false)
		{
			_updatedKeys.Add(name);

			if(value is Enum) {
				value = ResourceHelper.GetEnumText((Enum)value);
			}

			if(_entries.TryGetValue(name, out TooltipEntry? entry)) {
				entry.Value = value;
			} else {
				entry = new TooltipEntry(name, value, useMonoFont);
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

			for(int i = 0; i < Count; i++) {
				if(this[i] is TooltipSeparator sep) {
					bool hideSeparator = (i < Count - 1 && this[i + 1] is TooltipSeparator) || i == Count - 1;
					if(sep.Hidden != hideSeparator) {
						sep.Hidden = hideSeparator;
					}
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

	public class TooltipPaletteEntry : ReactiveObject
	{
		[Reactive] public UInt32[] RgbPalette { get; set; }
		[Reactive] public UInt32[] RawPalette { get; set; }
		[Reactive] public RawPaletteFormat RawFormat { get; set; }

		public TooltipPaletteEntry(UInt32[] rgbPalette, UInt32[] rawPalette, RawPaletteFormat rawFormat)
		{
			RgbPalette = rgbPalette;
			RawPalette = rawPalette;
			RawFormat = rawFormat;
		}

		public TooltipPaletteEntry(int paletteIndex, int paletteSize, UInt32[] rgbPalette, UInt32[] rawPalette, RawPaletteFormat rawFormat)
		{
			RgbPalette = new UInt32[paletteSize];
			if(rgbPalette.Length >= paletteIndex * paletteSize + paletteSize) {
				Array.Copy(rgbPalette, paletteIndex * paletteSize, RgbPalette, 0, paletteSize);
			}

			RawPalette = new UInt32[paletteSize];
			if(rawPalette.Length >= paletteIndex * paletteSize + paletteSize) {
				Array.Copy(rawPalette, paletteIndex * paletteSize, RawPalette, 0, paletteSize);
			}

			RawFormat = rawFormat;
		}
	}
}
