using ReactiveUI;
using System;

namespace Mesen.Utilities
{
	public class AllPropertiesObserver : IObserver<IReactivePropertyChangedEventArgs<IReactiveObject>>
	{
		private Action _callback;

		public AllPropertiesObserver(Action callback)
		{
			_callback = callback;
		}

		public void OnCompleted()
		{
		}

		public void OnError(Exception error)
		{
		}

		public void OnNext(IReactivePropertyChangedEventArgs<IReactiveObject> value)
		{
			_callback();
		}
	}
}
