using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class DbgIntegrationConfig
	{
		public bool AutoImport = true;
		public bool ResetLabelsOnImport = true;

		public bool ImportCpuPrgRomLabels = true;
		public bool ImportCpuWorkRamLabels = true;
		public bool ImportCpuSaveRamLabels = true;
		public bool ImportCpuComments = true;

		public bool ImportSpcRamLabels = true;
		public bool ImportSpcComments = true;
	}
}
