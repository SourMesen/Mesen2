using ReactiveUI;

namespace Mesen.ViewModels
{
	public class DisassemblyViewerViewModel : ViewModelBase
	{
		public string _content = "";
		public string Content
		{
			get => _content;
			set => this.RaiseAndSetIfChanged(ref _content, value);
		}

		public int _location = 0;
		public int Location
		{
			get => _location;
			set => this.RaiseAndSetIfChanged(ref _location, value);
		}
	}
}
