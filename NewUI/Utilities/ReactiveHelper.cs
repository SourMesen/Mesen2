using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.ComponentModel;
using System.Reflection;

namespace Mesen.Utilities
{
	public static class ReactiveHelper
	{
		public static IDisposable RegisterRecursiveObserver(ReactiveObject target, PropertyChangedEventHandler handler)
		{
			foreach(PropertyInfo prop in target.GetType().GetProperties(BindingFlags.Instance | BindingFlags.Public)) {
				if(prop.GetCustomAttribute<ReactiveAttribute>() != null) {
					if(prop.GetValue(target) is ReactiveObject propValue) {
						ReactiveHelper.RegisterRecursiveObserver(propValue, handler);
					}
				}
			}

			target.PropertyChanged += handler;

			return new RecursiveObserver(target, handler);
		}

		public static void UnregisterRecursiveObserver(ReactiveObject target, PropertyChangedEventHandler handler)
		{
			foreach(PropertyInfo prop in target.GetType().GetProperties(BindingFlags.Instance | BindingFlags.Public)) {
				if(prop.GetCustomAttribute<ReactiveAttribute>() != null) {
					if(prop.GetValue(target) is ReactiveObject propValue) {
						ReactiveHelper.UnregisterRecursiveObserver(propValue, handler);
					}
				}
			}

			target.PropertyChanged -= handler;
		}
	}

	public class RecursiveObserver : IDisposable
	{
		private ReactiveObject _target;
		private PropertyChangedEventHandler _handler;

		public RecursiveObserver(ReactiveObject target, PropertyChangedEventHandler handler)
		{
			_target = target;
			_handler = handler;
		}

		public void Dispose()
		{
			ReactiveHelper.UnregisterRecursiveObserver(_target, _handler);
		}
	}
}
