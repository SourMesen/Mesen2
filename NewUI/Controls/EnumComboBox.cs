using Avalonia;
using Avalonia.Controls;
using System;
using System.Linq;
using Avalonia.Styling;
using Avalonia.Data;

namespace Mesen.Controls
{
	public class EnumComboBox : ComboBox, IStyleable
	{
		Type IStyleable.StyleKey => typeof(ComboBox);

		public static readonly StyledProperty<Enum[]> AvailableValuesProperty = AvaloniaProperty.Register<EnumComboBox, Enum[]>(nameof(AvailableValues));

		public Enum[] AvailableValues
		{
			get { return GetValue(AvailableValuesProperty); }
			set { SetValue(AvailableValuesProperty, value); }
		}

		private Type? _enumType = null;

		static EnumComboBox()
		{
			AvailableValuesProperty.Changed.AddClassHandler<EnumComboBox>((x, e) => x.InitComboBox());
		}

		public EnumComboBox()
		{
			HorizontalAlignment = Avalonia.Layout.HorizontalAlignment.Stretch;
		}

		protected override void UpdateDataValidation(AvaloniaProperty property, BindingValueType state, Exception? error)
		{
			if(_enumType == null && property == SelectedItemProperty && state == BindingValueType.Value) {
				if(SelectedItem is Enum selectedItem) {
					_enumType = selectedItem.GetType();
				} else {
					throw new Exception("Invalid selected item - not an enum");
				}
				InitComboBox();
			}
			base.UpdateDataValidation(property, state, error);
		}

		private void InitComboBox()
		{
			if(_enumType == null) {
				if(AvailableValues.Length > 0) {
					_enumType = AvailableValues[0].GetType();
				} else {
					throw new Exception("Enum type is not set");
				}
			}

			Enum? selectedItem = SelectedItem as Enum;
			if(AvailableValues == null || AvailableValues.Length == 0) {
				Items = Enum.GetValues(_enumType);
			} else {
				Items = AvailableValues;
			}

			if(AvailableValues?.Length > 0 && !AvailableValues.Contains(selectedItem)) {
				SelectedItem = AvailableValues[0];
			} else {
				SelectedItem = selectedItem;
			}
		}
	}
}
