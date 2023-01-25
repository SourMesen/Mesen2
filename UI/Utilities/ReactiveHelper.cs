using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections;
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
					object? value = prop.GetValue(target);
					if(value is ReactiveObject propValue) {
						ReactiveHelper.RegisterRecursiveObserver(propValue, handler);
					} else if(value is IList list) {
						foreach(object listValue in list) {
							if(listValue is ReactiveObject) {
								ReactiveHelper.RegisterRecursiveObserver((ReactiveObject)listValue, handler);
							}
						}
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
					object? value = prop.GetValue(target);
					if(value is ReactiveObject propValue) {
						ReactiveHelper.UnregisterRecursiveObserver(propValue, handler);
					} else if(value is IList list) {
						foreach(object listValue in list) {
							if(listValue is ReactiveObject) {
								ReactiveHelper.UnregisterRecursiveObserver((ReactiveObject)listValue, handler);
							}
						}
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
