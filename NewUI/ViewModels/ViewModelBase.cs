using ReactiveUI;
using System;
using System.Collections;
using System.Collections.Generic;

namespace Mesen.ViewModels
{
	public class ViewModelBase : ReactiveObject
	{
	}

	public class DisposableViewModel : ViewModelBase, IDisposable
	{
		private HashSet<IDisposable> _disposables = new();
		private bool _disposed = false;

		public void Dispose()
		{
			if(_disposed) {
				return;
			}

			_disposed = true;

			foreach(IDisposable obj in _disposables) {
				obj.Dispose();
			}
			_disposables.Clear();
			_disposables = new();

			DisposeView();
		}

		protected virtual void DisposeView() { }

		public void AddDisposable(IDisposable obj)
		{
			_disposables.Add(obj);
		}

		public T AddDisposables<T>(T disposables) where T : IEnumerable
		{
			foreach(object obj in disposables) {
				if(obj is IDisposable disposable) {
					_disposables.Add(disposable);
				}
			}
			return disposables;
		}
	}
}
