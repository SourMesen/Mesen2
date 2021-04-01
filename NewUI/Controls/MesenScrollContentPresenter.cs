using Avalonia.Controls;
using Avalonia.Controls.Presenters;
using Avalonia.Input;
using Avalonia.Styling;
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

		protected override void OnPointerWheelChanged(PointerWheelEventArgs e)
		{
			if(e.KeyModifiers == KeyModifiers.Shift) {
				//Allow shift+wheel for horizontal scrolling
				e.Delta = new Avalonia.Vector(e.Delta.Y, e.Delta.X);
			}
			base.OnPointerWheelChanged(e);
		}
	}
}
