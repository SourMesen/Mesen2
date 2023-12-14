using ReactiveUI.Fody.Helpers;
using System;
using System.Reactive.Linq;
using System.Linq;
using Mesen.Interop;
using Mesen.Config;
using ReactiveUI;
using System.Text;
using Avalonia.Controls;
using System.Collections.Generic;
using Mesen.Utilities;
using Mesen.Windows;

namespace Mesen.ViewModels
{
	public class CheatEditWindowViewModel : DisposableViewModel
	{
		public CheatCode Cheat { get; }

		[ObservableAsProperty] public string ConvertedCodes { get; } = "";
		[Reactive] public bool ShowInvalidCodeHint { get; private set; } = false;
		[Reactive] public bool OkButtonEnabled { get; private set; } = false;

		[Reactive] public Enum[] AvailableCheatTypes { get; private set; } = Array.Empty<Enum>();

		private MainWindowViewModel MainWndModel { get; }

		[Obsolete("For designer only")]
		public CheatEditWindowViewModel() : this(new CheatCode()) { }

		public CheatEditWindowViewModel(CheatCode cheat)
		{
			Cheat = cheat;
			MainWndModel = MainWindowViewModel.Instance;

			AddDisposable(this.WhenAnyValue(x => x.Cheat.Codes, x => x.Cheat.Type).Select(x => {
				string[] codes = cheat.Codes.Split(Environment.NewLine);
				StringBuilder sb = new StringBuilder();
				bool hasInvalidCode = false;
				bool hasValidCode = false;
				foreach(string codeString in codes) {
					if(sb.Length > 0) {
						sb.Append(Environment.NewLine);
					}

					InteropInternalCheatCode code = new();
					if(EmuApi.GetConvertedCheat(new InteropCheatCode(cheat.Type, codeString.Trim()), ref code)) {
						if(code.IsAbsoluteAddress) {
							sb.Append(code.MemType.GetShortName() + " ");
						}

						if(code.Compare >= 0) {
							sb.Append($"{code.Address:X2}:{code.Value:X2}:{code.Compare:X2}");
						} else {
							sb.Append($"{code.Address:X2}:{code.Value:X2}");
						}
						hasValidCode = true;
					} else {
						if(!string.IsNullOrWhiteSpace(codeString)) {
							hasInvalidCode = true;
							sb.Append("[invalid code]");
						}
					}
				}

				ShowInvalidCodeHint = hasInvalidCode;
				OkButtonEnabled = hasValidCode && !hasInvalidCode;

				return sb.ToString();
			}).ToPropertyEx(this, x => x.ConvertedCodes));

			if(Design.IsDesignMode) {
				return;
			}

			AddDisposable(this.WhenAnyValue(x => x.MainWndModel.RomInfo).Subscribe(romInfo => {
				AvailableCheatTypes = Enum.GetValues<CheatType>().Where(e => romInfo.CpuTypes.Contains(e.ToCpuType())).Cast<Enum>().ToArray();
				if(!AvailableCheatTypes.Contains(Cheat.Type)) {
					Cheat.Type = (CheatType)AvailableCheatTypes[0];
				}
			}));
		}
	}
}
