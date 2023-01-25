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
		public bool Disposed { get; private set; } = false;

		public void Dispose()
		{
			if(Disposed) {
				return;
			}

			Disposed = true;

			foreach(IDisposable obj in _disposables) {
				obj.Dispose();
			}
			_disposables.Clear();
			_disposables = new();

			DisposeView();
		}

		protected virtual void DisposeView() { }

		public T AddDisposable<T>(T obj) where T : IDisposable
		{
			_disposables.Add(obj);
			return obj;
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
