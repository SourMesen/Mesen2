using Avalonia.Controls;
using System;

namespace Mesen.Controls;

public class MesenNumericUpDown : NumericUpDown
{
	protected override Type StyleKeyOverride => typeof(NumericUpDown);

	protected override void OnTextChanged(string? oldValue, string? newValue)
	{
		if(newValue == null || newValue == "") {
			//Prevent displaying invalid cast error/breaking the layout when user clears the content of the control
			Text = "0";
		}
		base.OnTextChanged(oldValue, newValue);
	}
}