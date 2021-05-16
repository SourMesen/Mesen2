using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Mesen.Config
{
	public class AviRecordConfig
	{
		public VideoCodec Codec = VideoCodec.CSCD;
		public UInt32 CompressionLevel = 6;
	}

	public enum VideoCodec
	{
		None = 0,
		ZMBV = 1,
		CSCD = 2,
		GIF = 3
	}
}
