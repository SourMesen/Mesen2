using Avalonia.Markup.Xaml;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Localization
{
	public class TranslateExtension : MarkupExtension
	{
		public TranslateExtension(string key)
		{
			this.Key = key;
		}

		public string Key { get; set; }

		public override object ProvideValue(IServiceProvider serviceProvider)
		{
			Type? contextType = null;
			if(serviceProvider.GetType().GenericTypeArguments.Length > 0) {
				contextType = serviceProvider.GetType().GenericTypeArguments[0];
			}

			return ResourceHelper.GetViewLabel(contextType?.Name ?? "unknown", this.Key);
		}
	}
}	
