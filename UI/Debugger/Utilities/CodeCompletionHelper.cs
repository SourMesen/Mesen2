using Mesen.Interop;
using Mesen.Localization;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Text.Json;
using System.Text.RegularExpressions;

namespace Mesen.Debugger.Utilities
{
	public class CodeCompletionHelper
	{
		private static Dictionary<string, DocEntryViewModel> _documentation;
		
		private static Regex _linkRegex = new Regex("emu[.](([a-zA-Z0-9]+)(\\([a-zA-Z0-9 ,]*\\)){0,1})", RegexOptions.Compiled);

		static CodeCompletionHelper()
		{
			using StreamReader reader = new StreamReader(Assembly.GetExecutingAssembly().GetManifestResourceStream("Mesen.Debugger.Documentation.LuaDocumentation.json")!);
			DocEntryViewModel[] documentation = (DocEntryViewModel[]?)JsonSerializer.Deserialize(reader.ReadToEnd(), typeof(DocEntryViewModel[]), MesenCamelCaseSerializerContext.Default) ?? Array.Empty<DocEntryViewModel>();

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

		public static string GenerateHtmlDocumentation()
		{
			string processText(string text)
			{
				return _linkRegex.Replace(text, (match) => "<a href=\"#" + match.Groups[2].Value + "\">emu." + match.Groups[1].Value + "</a>").Replace("\n", "<br/>");
			}

			StringBuilder sb = new();
			sb.AppendLine(@$"
				<html>
					<head>
						<title>Mesen Lua API Reference</title>
						<style>
							* {{
								margin: 0;
								font: inherit;
							}}

							body {{
								font-family: ""Calibri"", ""Segoe UI"", ""Microsoft Sans Serif"", ""FreeSans"", ""DejaVu Sans"", ""Noto Sans"";
								font-size: 17px;
								height: 100%;
							}}

							em {{
								font-style: italic;
							}}

							a {{
								text-decoration: none;
								color: #599a3e;
								font-weight: bold;
							}}
							
							a:hover {{
								color: #3f6d2c;
							}}

							strong {{
								font-weight: bold;
							}}

							h1 {{
								font-size: 32px;
								margin-top: 40px;
								margin-bottom: -20px;
							}}

							.mainheader {{
								font-size: 36px;
								margin-top: 10px;
								margin-bottom: 20px;
							}}

							h2 {{
								font-size: 26px;
								border-bottom: 1px solid gray;
								margin-top: 20px;
								margin-bottom: 5px;
							}}

							h3 {{
								font-size: 22px;
								border-bottom: 1px solid gray;
								margin-top: 30px;
								margin-bottom: 5px;
							}}

							p {{
								margin-top: 10px;
								margin-bottom: 5px;
							}}

							.monofont {{
								border: 1px solid gray;
								padding: 10px;
								margin-left: 10px;
								background: #F0F0F0;
								font-size: 15px;
								font-family: ""Consolas"", ""Courier New"", ""FreeMono"", ""DejaVu Sans Mono"", ""Noto Sans Mono"", ""PT Mono"";
							}}

							.fieldData {{
								margin-left: 10px;
							}}

							.fieldDesc {{
								margin-left: 10px;
								margin-top: 1px;
								margin-bottom: 10px;
							}}

							.pageLayout {{
								height: 100%;
							}}
							
							.menu {{
								width: 200px;
								position: fixed;
								padding: 10px;
								height: calc(100vh - 20px);
								overflow: auto;
								font-size: 16px;
							}}

							.maincontent {{
								overflow: auto;
								padding-left: 230px;
								padding-right: 25px;
								padding-bottom: 25px;
							}}

							.category {{
								margin-left: 10px;
								margin-bottom: 10px;
							}}

							.subcategory {{
								margin-left: 10px;
								margin-bottom: 5px;
							}}

							.apilink {{
								font-size: 14px;
							}}

							.apilink > a {{ 
								color: #07D;
								font-weight: normal;
							}}

							.apilink > a:hover {{ 
								color: #04A;
								font-weight: normal;
							}}

							.defaultvalue {{
								color: #555;
								font-style: italic;
							}}
						</style>
					</head>
				<body>
					<div class=""pageLayout"">
");


			DocEntryCategory? category = null;
			DocEntrySubcategory? subcategory = null;

			sb.AppendLine("<div class=\"menu\">");
			foreach(DocEntryViewModel entry in _documentation.Values.OrderBy(x => (int)x.Category).ThenBy(x => (int?)x.Subcategory)) {
				if(entry.Category != category) {
					if(category != null) {
						sb.AppendLine("</div>");
					}
					if(subcategory != null) {
						sb.AppendLine("</div>");
					}

					subcategory = null;
					sb.AppendLine($"<a href=\"#{entry.Category}\">{ResourceHelper.GetEnumText(entry.Category)}</a><div class=\"category\">");
					category = entry.Category;
				}

				if(entry.Subcategory != subcategory && entry.Subcategory != null) {
					if(subcategory != null) {
						sb.AppendLine("</div>");
					}
					sb.AppendLine($"<a href=\"#{entry.Category}_{entry.Subcategory}\">{ResourceHelper.GetEnumText(entry.Subcategory)}</a><div class=\"subcategory\">");
					subcategory = entry.Subcategory;
				}

				sb.AppendLine($"<div class=\"apilink\"><a href=\"#{entry.Name}\">{entry.Name}</a></div>");
			}
			if(category != null) {
				sb.AppendLine("</div>");
			}
			if(subcategory != null) {
				sb.AppendLine("</div>");
			}
			sb.AppendLine("</div>");

			category = null;
			subcategory = null;

			sb.AppendLine("<div class=\"maincontent\">");

			sb.AppendLine("<h1 class=\"mainheader\">Mesen Lua API reference</h1>");
			sb.AppendLine("<p><strong>Important: </strong>This API is similar but not completely compatible with the old Mesen 0.9.x (or Mesen-S) Lua APIs.</p>");
			sb.AppendLine("<p>Generated on " + EmuApi.GetMesenBuildDate() + " for Mesen " + EmuApi.GetMesenVersion().ToString() + ".</p>");

			foreach(DocEntryViewModel entry in _documentation.Values.OrderBy(x => (int)x.Category).ThenBy(x => (int?)x.Subcategory)) {
				if(entry.Category != category) {
					sb.AppendLine($"<h1 id=\"{entry.Category}\">{ResourceHelper.GetEnumText(entry.Category)}</h1>");
					category = entry.Category;
					subcategory = null;
				}

				if(entry.Subcategory != subcategory && entry.Subcategory != null) {
					sb.AppendLine($"<h2 id=\"{entry.Category}_{entry.Subcategory}\">{ResourceHelper.GetEnumText(entry.Subcategory)}</h2>");
					subcategory = entry.Subcategory;
				}

				sb.AppendLine($"<h3 id=\"{entry.Name}\">{entry.Name}</h3>");
				if(entry.EnumValues.Count > 0) {
					sb.AppendLine($"<p><strong>Syntax</strong></p>");
					sb.AppendLine($"<div class=\"monofont\">emu.{entry.Name}.[value]</div>");

					sb.AppendLine($"<p><strong>Description</strong></p>");
					sb.AppendLine($"<div class=\"fieldData\">{processText(entry.Description)}</div>");

					sb.AppendLine($"<p><strong>Values</strong></p>");
					sb.AppendLine($"<div class=\"monofont\">");
					foreach(DocEnumValue value in entry.EnumValues) {
						sb.AppendLine($"<div>{value.Name} - {processText(value.Description)}</div>");
					}
					sb.AppendLine($"</div>");
				} else {
					sb.AppendLine($"<p><strong>Syntax</strong></p>");
					sb.AppendLine($"<div class=\"monofont\">emu.{entry.Name}({string.Join(", ", entry.Parameters.Select(x => x.Name))})</div>");
					sb.AppendLine($"<p><strong>Parameters</strong></p>");

					if(entry.Parameters.Count == 0) {
						sb.AppendLine($"<div class=\"fieldData\"><em>&lt;none&gt;</em></div>");
					} else {
						foreach(DocParam param in entry.Parameters) {
							sb.AppendLine($"<div class=\"fieldData\"><strong>{param.Name}</strong> - <em>");
							if(param.Type.ToLowerInvariant() == "enum") {
								sb.AppendLine($"<a href=\"#{param.EnumName}\">{param.CalculatedType}</a>");
							} else {
								sb.AppendLine($"{param.CalculatedType}");
							}
							sb.AppendLine($"</em>{(param.DefaultValue.Length > 0 ? $" <span class=\"defaultvalue\">(default: {param.DefaultValue})</span>" : "")}");
							sb.AppendLine($"<div class=\"fieldDesc\">{processText(param.Description)}</div>");
							sb.AppendLine($"</div>");
						}
					}

					sb.AppendLine($"<p><strong>Return value</strong></p>");

					sb.AppendLine($"<div class=\"fieldData\">");
					if(string.IsNullOrEmpty(entry.ReturnValue.Type)) {
						sb.AppendLine("<em>&lt;none&gt;</em>");
					} else {
						sb.AppendLine($"<em>{entry.ReturnValue.Type}</em> - {processText(entry.ReturnValue.Description)}");
					}
					sb.AppendLine($"</div>");
					sb.AppendLine($"<p><strong>Description</strong></p>");
					sb.AppendLine($"<div class=\"fieldData\">{processText(entry.Description)}</div>");
				}
			}
			sb.AppendLine("</div>");
			sb.AppendLine("</body></html>");

			return sb.ToString();
		}
	}

	public enum DocEntryCategory
	{
		Callbacks,
		Drawing,
		Emulation,
		Input,
		Logging,
		MemoryAccess,
		Miscellaneous,
		Enums
	}

	public enum DocEntrySubcategory
	{
		AccessCounters,
		Cdl,
		Cheats,
		SaveStates,
		Others
	}

	public class DocEntryViewModel
	{
		public string Name { get; set; } = "";
		public string Description { get; set; } = "";
		public DocEntryCategory Category { get; set; } = DocEntryCategory.Callbacks;
		public DocEntrySubcategory? Subcategory { get; set; } = null;
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
