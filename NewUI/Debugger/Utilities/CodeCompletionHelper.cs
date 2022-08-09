using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;

namespace Mesen.Debugger.Utilities
{
	public class CodeCompletionHelper
	{
		private static DocEntryViewModel[] _documentation;

		static CodeCompletionHelper()
		{
			using StreamReader reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.Utilities.LuaDocumentation.json")!);
			_documentation = JsonSerializer.Deserialize<DocEntryViewModel[]>(reader.ReadToEnd(), new JsonSerializerOptions() { PropertyNamingPolicy = JsonNamingPolicy.CamelCase }) ?? Array.Empty<DocEntryViewModel>();
		}

		public static IEnumerable<string> GetEntries()
		{
			return _documentation.Select(x => x.Name);
		}

		public static DocEntryViewModel GetEntry(string keyword)
		{
			foreach(var entry in _documentation) {
				if(entry.Name == keyword) {
					return entry;
				}
			}
			return _documentation[0];			
		}
	}

	public class DocEntryViewModel
	{
		public string Name { get; set; } = "";
		public string Description { get; set; } = "";
		public List<DocParam> Parameters { get; set; } = new();
		public DocReturnValue ReturnValue { get; set; } = new();

		public string Syntax
		{
			get
			{
				StringBuilder sb = new();
				sb.Append($"emu.{Name}(");
				bool isOptional = false;
				for(int i = 0; i < Parameters.Count; i++) {
					if(!string.IsNullOrEmpty(Parameters[i].DefaultValue)) {
						if(!isOptional) {
							isOptional = true;
							sb.Append("[");
						}
					} else {
						System.Diagnostics.Debug.Assert(!isOptional);
					}
					sb.Append(Parameters[i].Name);
					if(i < Parameters.Count - 1) {
						sb.Append(", ");
					} else {
						if(isOptional) {
							sb.Append("]");
						}
					}
				}
				sb.Append(")");
				return sb.ToString();
			}
		}
	}

	public class DocParam
	{
		public string Name { get; set; } = "";
		public string Type { get; set; } = "";
		public string Description { get; set; } = "";
		public string DefaultValue { get; set; } = "";
	}

	public class DocReturnValue
	{
		public string Type { get; set; } = "";
		public string Description { get; set; } = "";
	}
}
