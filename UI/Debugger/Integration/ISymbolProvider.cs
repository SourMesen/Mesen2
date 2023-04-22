using Mesen.Interop;
using System;
using System.Collections.Generic;
using System.IO;

namespace Mesen.Debugger.Integration
{
	public interface ISymbolProvider
	{
		DateTime SymbolFileStamp { get; }
		string SymbolPath { get; }

		List<SourceFileInfo> SourceFiles { get; }

		AddressInfo? GetLineAddress(SourceFileInfo file, int lineIndex);
		AddressInfo? GetLineEndAddress(SourceFileInfo file, int lineIndex);
		string? GetSourceCodeLine(int prgRomAddress);
		SourceCodeLocation? GetSourceCodeLineInfo(AddressInfo address);
		AddressInfo? GetSymbolAddressInfo(SourceSymbol symbol);
		SourceCodeLocation? GetSymbolDefinition(SourceSymbol symbol);
		SourceSymbol? GetSymbol(string word, int scopeStart, int scopeEnd);
		List<SourceSymbol> GetSymbols();
		int GetSymbolSize(SourceSymbol srcSymbol);
		//List<DbgImporter.ReferenceInfo> GetSymbolReferences(SourceSymbol symbol);
		//int GetSymbolSize(SourceSymbol symbol);
	}

	public interface IFileDataProvider
	{
		string[] Data { get; }
	}

	public class SourceFileInfo
	{
		public string Name { get; }
		public IFileDataProvider InternalFile { get; }
		public bool IsAssembly { get; }
		public string[] Data => InternalFile.Data;
		public string FileData => string.Join(Environment.NewLine, InternalFile.Data);

		public SourceFileInfo(string name, bool isAssembly, IFileDataProvider internalFile)
		{
			Name = name;
			IsAssembly = isAssembly;
			InternalFile = internalFile;
		}

		public override string ToString()
		{
			string? folderName = Path.GetDirectoryName(Name);
			string fileName = Path.GetFileName(Name);
			if(string.IsNullOrWhiteSpace(folderName)) {
				return fileName;
			} else {
				return $"{folderName}/{fileName}";
			}
		}
	}

	public readonly struct SourceSymbol
	{
		public string Name { get; }
		public int? Address { get; }
		public object InternalSymbol { get; }

		public SourceSymbol(string name, int? address, object internalSymbol)
		{
			Name = name;
			Address = address;
			InternalSymbol = internalSymbol;
		}
	}

	public readonly struct SourceCodeLocation : IEquatable<SourceCodeLocation>
	{
		public SourceFileInfo File { get; }
		public int LineNumber { get; }
		public object? InternalLine { get; }

		public SourceCodeLocation(SourceFileInfo file, int lineNumber, object? internalLine = null)
		{
			File = file;
			LineNumber = lineNumber;
			InternalLine = internalLine;
		}

		public bool Equals(SourceCodeLocation other)
		{
			return File == other.File && LineNumber == other.LineNumber;
		}
	}
}