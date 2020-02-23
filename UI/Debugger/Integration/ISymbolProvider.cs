using System;
using System.Collections.Generic;
using System.IO;

namespace Mesen.GUI.Debugger.Integration
{
	public interface ISymbolProvider
	{
		DateTime SymbolFileStamp { get; }
		string SymbolPath { get; }

		List<SourceFileInfo> SourceFiles { get; }

		AddressInfo? GetLineAddress(SourceFileInfo file, int lineIndex);
		string GetSourceCodeLine(int prgRomAddress);
		SourceCodeLocation GetSourceCodeLineInfo(AddressInfo address);
		AddressInfo? GetSymbolAddressInfo(SourceSymbol symbol);
		SourceCodeLocation GetSymbolDefinition(SourceSymbol symbol);
		SourceSymbol GetSymbol(string word, int prgStartAddress, int prgEndAddress);
		List<SourceSymbol> GetSymbols();
		int GetSymbolSize(SourceSymbol srcSymbol);
		//List<DbgImporter.ReferenceInfo> GetSymbolReferences(SourceSymbol symbol);
		//int GetSymbolSize(SourceSymbol symbol);
	}

	public class SourceFileInfo
	{
		public string Name;
		public string[] Data;
		
		public object InternalFile;

		public override string ToString()
		{
			string folderName = Path.GetDirectoryName(Name);
			string fileName = Path.GetFileName(Name);
			if(string.IsNullOrWhiteSpace(folderName)) {
				return fileName;
			} else {
				return $"{fileName} ({folderName})";
			}
		}
	}

	public class SourceSymbol
	{
		public string Name;
		public int? Address;
		public object InternalSymbol;
	}

	public class SourceCodeLocation
	{
		public SourceFileInfo File;
		public int LineNumber;
		public object InternalLine;
	}
}