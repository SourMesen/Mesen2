using Mesen.GUI;
using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reactive.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.ViewModels
{
	public class MainWindowViewModel : ViewModelBase
	{
		[Reactive] public RomInfo RomInfo { get; set; }

		[ObservableAsProperty] public bool ShowAudioPlayer { get; set; }

		public AudioPlayerViewModel AudioPlayer { get; set; }

		public MainWindowViewModel()
		{
			this.AudioPlayer = new AudioPlayerViewModel();
			this.RomInfo = new RomInfo();

			this.WhenAnyValue(x => x.RomInfo).Select(x => x.Format == RomFormat.Nsf || x.Format == RomFormat.Spc || x.Format == RomFormat.Gbs).ToPropertyEx(this, x => x.ShowAudioPlayer);
		}
	}
}
