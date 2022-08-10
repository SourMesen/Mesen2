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
		private static Dictionary<string, DocEntryViewModel> _documentation;

		static CodeCompletionHelper()
		{
			using StreamReader reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.Utilities.LuaDocumentation.json")!);
			DocEntryViewModel[] documentation = JsonSerializer.Deserialize<DocEntryViewModel[]>(reader.ReadToEnd(), new JsonSerializerOptions() { PropertyNamingPolicy = JsonNamingPolicy.CamelCase }) ?? Array.Empty<DocEntryViewModel>();

			_documentation = new Dictionary<string, DocEntryViewModel>();
			foreach(DocEntryViewModel entry in documentation) {
				_documentation[entry.Name] = entry;
			}
		}

		public static IEnumerable<string> GetEntries()
		{
			return _documentation.Select(x => x.Key).OrderBy(x => x);
		}

		public static DocEntryViewModel? GetEntry(string keyword)
		{
			if(_documentation.TryGetValue(keyword, out DocEntryViewModel? entry)) {
				return entry;
			}
			return null;
		}
	}

	public class DocEntryViewModel
	{
		public string Name { get; set; } = "";
		public string Description { get; set; } = "";
		public List<DocParam> Parameters { get; set; } = new();
		public DocReturnValue ReturnValue { get; set; } = new();
		
		public List<DocEnumValue> EnumValues { get; set; } = new();

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
		public string EnumName { get; set; } = "";
		public string Description { get; set; } = "";
		public string DefaultValue { get; set; } = "";

		public string CalculatedType
		{
			get
			{
				if(Type.ToLowerInvariant() == "enum") {
					System.Diagnostics.Debug.Assert(EnumName.Length > 0);
					return $"Enum ({EnumName})";
				}
				return Type;
			}
		}
	}

	public class DocReturnValue
	{
		public string Type { get; set; } = "";
		public string Description { get; set; } = "";
	}
	
	public class DocEnumValue
	{
		public string Name { get; set; } = "";
		public string Description { get; set; } = "";
	}
}
