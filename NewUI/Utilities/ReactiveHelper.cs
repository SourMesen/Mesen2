using ReactiveUI;
using ReactiveUI.Fody.Helpers;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;

namespace Mesen.Utilities
{
	public static class ReactiveHelper
	{
		public static void RegisterRecursiveObserver(ReactiveObject target, PropertyChangedEventHandler handler)
		{
			foreach(PropertyInfo prop in target.GetType().GetProperties(BindingFlags.Instance | BindingFlags.Public)) {
				if(prop.GetCustomAttribute<ReactiveAttribute>() != null) {
					if(prop.GetValue(target) is ReactiveObject propValue) {
						ReactiveHelper.RegisterRecursiveObserver(propValue, handler);
					}
				}
			}

			target.PropertyChanged += handler;
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
}
