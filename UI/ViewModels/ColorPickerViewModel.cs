using Avalonia.Media;
using ReactiveUI.Fody.Helpers;

namespace Mesen.ViewModels
{
	public class ColorPickerViewModel : ViewModelBase
	{
		[Reactive] public Color Color { get; set; }
	}
}
