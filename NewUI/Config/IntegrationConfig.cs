using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class IntegrationConfig : BaseConfig<IntegrationConfig>
	{
		[Reactive] public bool AutoLoadDbgFiles { get; set; } = true;
		[Reactive] public bool AutoLoadMlbFiles { get; set; } = true;
		[Reactive] public bool AutoLoadCdlFiles { get; set; } = true;
		[Reactive] public bool AutoLoadSymFiles { get; set; } = true;
		[Reactive] public bool AutoLoadFnsFiles { get; set; } = true;

		[Reactive] public bool ResetLabelsOnImport { get; set; } = true;

		//TODO
		[Reactive] public bool ImportCpuPrgRomLabels { get; set; } = true;
		[Reactive] public bool ImportCpuWorkRamLabels { get; set; } = true;
		[Reactive] public bool ImportCpuSaveRamLabels { get; set; } = true;
		[Reactive] public bool ImportCpuComments { get; set; } = true;
		[Reactive] public bool ImportSpcRamLabels { get; set; } = true;
		[Reactive] public bool ImportSpcComments { get; set; } = true;
	}
}
