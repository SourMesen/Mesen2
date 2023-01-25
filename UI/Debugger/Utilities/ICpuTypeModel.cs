using Mesen.Interop;

namespace Mesen.Debugger.Utilities
{
	public interface ICpuTypeModel
	{
		CpuType CpuType { get; set; }
		void OnGameLoaded();
	}
}