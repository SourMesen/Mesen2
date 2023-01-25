using Mesen.Interop;
using ReactiveUI.Fody.Helpers;

namespace Mesen.Config
{
	public class MovieRecordConfig : BaseConfig<MovieRecordConfig>
	{
		[Reactive] public RecordMovieFrom RecordFrom { get; set; } = RecordMovieFrom.CurrentState;
		[Reactive] public string Author { get; set; } = "";
		[Reactive] public string Description { get; set; } = "";
	}
}
