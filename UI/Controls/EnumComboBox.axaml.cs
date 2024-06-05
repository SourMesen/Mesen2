using Avalonia;
using Avalonia.Controls;
using Avalonia.Input;
using Avalonia.Interactivity;
using Avalonia.Markup.Xaml;
using Avalonia.Media;
using Avalonia.Threading;
using Mesen.Localization;
using System;
using System.Collections;
using System.Linq;

namespace Mesen.Controls
{
	public class EnumComboBox : UserControl
	{
		public static readonly StyledProperty<Enum[]?> AvailableValuesProperty = AvaloniaProperty.Register<EnumComboBox, Enum[]?>(nameof(AvailableValues), null);
		public static readonly StyledProperty<Enum?> SelectedItemProperty = AvaloniaProperty.Register<EnumComboBox, Enum?>(nameof(SelectedItem), defaultBindingMode: Avalonia.Data.BindingMode.TwoWay);
		
		public static readonly StyledProperty<IEnumerable?> InternalItemsProperty = AvaloniaProperty.Register<EnumComboBox, IEnumerable?>(nameof(InternalItems));
		public static readonly StyledProperty<Enum?> InternalSelectedItemProperty = AvaloniaProperty.Register<EnumComboBox, Enum?>(nameof(InternalSelectedItem));

		public Enum[]? AvailableValues
		{
			get { return GetValue(AvailableValuesProperty); }
			set { SetValue(AvailableValuesProperty, value); }
		}

		public Enum? SelectedItem
		{
			get { return GetValue(SelectedItemProperty); }
			set { SetValue(SelectedItemProperty, value); }
		}

		public IEnumerable? InternalItems
		{
			get { return GetValue(InternalItemsProperty); }
			set { SetValue(InternalItemsProperty, value); }
		}

		public Enum? InternalSelectedItem
		{
			get { return GetValue(InternalSelectedItemProperty); }
			set { SetValue(InternalSelectedItemProperty, value); }
		}

		private Type? _enumType = null;

		static EnumComboBox()
		{
			SelectedItemProperty.Changed.AddClassHandler<EnumComboBox>((x, e) => { x.InitComboBox(false); });
			AvailableValuesProperty.Changed.AddClassHandler<EnumComboBox>((x, e) => { x.InitComboBox(true); });
			InternalSelectedItemProperty.Changed.AddClassHandler<EnumComboBox>((x, e) => {
				if(x.IsLoaded && x.InternalSelectedItem != null) {
					x.SelectedItem = x.InternalSelectedItem;
				}
			});
		}

		public EnumComboBox()
		{
			InitializeComponent();

			ComboBox dropdown = this.GetControl<ComboBox>("Dropdown");
			dropdown.AddHandler(ComboBox.PointerPressedEvent, this.PointerPressedHandler, RoutingStrategies.Tunnel);
			dropdown.AddHandler(ComboBox.PointerReleasedEvent, this.PointerReleasedHandler, RoutingStrategies.Tunnel);
		}

		private void InitializeComponent()
		{
			AvaloniaXamlLoader.Load(this);
		}

		protected override void OnLoaded(RoutedEventArgs e)
		{
			base.OnLoaded(e);
			InitComboBox(true);
		}

		private void InitComboBox(bool updateAvailableValues)
		{
			if(!IsLoaded) {
				return;
			}
			
			if(_enumType == null || SelectedItem == null) {
				if(AvailableValues?.Length > 0) {
					_enumType = AvailableValues[0].GetType();
				} else if(SelectedItem != null) {
					_enumType = SelectedItem.GetType();
				} else {
					return;
				}
			}

			if(SelectedItem == null || (AvailableValues?.Length > 0 && !AvailableValues.Contains(SelectedItem))) {
				SelectedItem = AvailableValues?.Length > 0 ? AvailableValues[0] : null;
			}

			Dispatcher.UIThread.Post(() => {
				if(updateAvailableValues) {
					if(AvailableValues == null || AvailableValues.Length == 0) {
						InternalItems = ResourceHelper.GetEnumValues(_enumType);
					} else {
						InternalItems = AvailableValues;
					}
				}

				InternalSelectedItem = SelectedItem;
			});
		}

		private bool _isPressed = false;
		private void PointerPressedHandler(object? sender, RoutedEventArgs e)
		{
			_isPressed = true;
		}

		private void PointerReleasedHandler(object? sender, PointerReleasedEventArgs e)
		{
			if(!_isPressed) {
				//Prevent combobox from opening when it only receives a "pointer released" event without
				//a "pointer pressed" event. This can occur when a click event opens a window under the cursor.
				e.Handled = true;
			}
			_isPressed = false;
		}
	}
}
