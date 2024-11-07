using Avalonia.Controls;
using Avalonia.Interactivity;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Utilities;

public class MesenUserControl : UserControl, IDisposable
{
	private HashSet<IDisposable> _disposables = new();
	public bool Disposed { get; private set; } = false;

	protected override void OnUnloaded(RoutedEventArgs e)
	{
		base.OnUnloaded(e);
		Dispose();
	}

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
	}

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
