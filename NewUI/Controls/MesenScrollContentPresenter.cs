using Avalonia;
using Avalonia.Controls;
using Avalonia.Controls.Presenters;
using Avalonia.Input;
using Avalonia.LogicalTree;
using Avalonia.Styling;
using DataBoxControl;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Controls
{
	public class MesenScrollContentPresenter : ScrollContentPresenter, IStyleable
	{
		Type IStyleable.StyleKey => typeof(ScrollContentPresenter);
		
		protected override Size MeasureOverride(Size availableSize)
		{
			if(this.FindLogicalAncestorOfType<DataBox>() != null) {
				//When inside a DataBox, use availableSize directly without
				//calling MeasureOverride on the ScrollContentPresenter
				//Otherwise, the DataBox tries to resize with the height set
				//to the screen's height, causing a large number of rows to
				//be created, and then removed again once the measure is
				//re-calculated with the correct sizes.
				Child.Measure(availableSize);
				return Child.DesiredSize.Constrain(availableSize);
			}
			return base.MeasureOverride(availableSize);
		}

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			if(e.KeyModifiers == KeyModifiers.Shift) {
				//Allow shift+wheel for horizontal scrolling
				e.Delta = new Avalonia.Vector(e.Delta.Y, e.Delta.X);
			}

			if(!e.KeyModifiers.HasFlag(KeyModifiers.Control)) {
				//Skip event if control is pressed, because this is used to zoom in/out
				base.OnPointerWheelChanged(e);
			}
		}
	}
}
