//          Copyright Tao Klerks, 2010-2012, tao@klerks.biz         
//          Licensed under the modified BSD license.


using System;
using System.IO;
using System.Text;
using System.Text.RegularExpressions;

namespace FastColoredTextBoxNS
{
    public static class EncodingDetector
    {
        const long _defaultHeuristicSampleSize = 0x10000; //completely arbitrary - inappropriate for high numbers of files / high speed requirements

        public static Encoding DetectTextFileEncoding(string InputFilename)
        {
            using (FileStream textfileStream = File.OpenRead(InputFilename))
            {
                return DetectTextFileEncoding(textfileStream, _defaultHeuristicSampleSize);
            }
        }

        public static Encoding DetectTextFileEncoding(FileStream InputFileStream, long HeuristicSampleSize)
        {
            bool uselessBool = false;
            return DetectTextFileEncoding(InputFileStream, _defaultHeuristicSampleSize, out uselessBool);
        }

        public static Encoding DetectTextFileEncoding(FileStream InputFileStream, long HeuristicSampleSize, out bool HasBOM)
        {
            Encoding encodingFound = null;

            long originalPos = InputFileStream.Position;

            InputFileStream.Position = 0;


            //First read only what we need for BOM detection
            byte[] bomBytes = new byte[InputFileStream.Length > 4 ? 4 : InputFileStream.Length];
            InputFileStream.Read(bomBytes, 0, bomBytes.Length);

            encodingFound = DetectBOMBytes(bomBytes);

            if (encodingFound != null)
            {
                InputFileStream.Position = originalPos;
                HasBOM = true;
                return encodingFound;
            }


            //BOM Detection failed, going for heuristics now.
            //  create sample byte array and populate it
            byte[] sampleBytes = new byte[HeuristicSampleSize > InputFileStream.Length ? InputFileStream.Length : HeuristicSampleSize];
            Array.Copy(bomBytes, sampleBytes, bomBytes.Length);
            if (InputFileStream.Length > bomBytes.Length)
                InputFileStream.Read(sampleBytes, bomBytes.Length, sampleBytes.Length - bomBytes.Length);
            InputFileStream.Position = originalPos;

            //test byte array content
            encodingFound = DetectUnicodeInByteSampleByHeuristics(sampleBytes);

            HasBOM = false;
            return encodingFound;
        }

        public static Encoding DetectBOMBytes(byte[] BOMBytes)
        {
            if (BOMBytes.Length < 2)
                return null;

            if (BOMBytes[0] == 0xff
                && BOMBytes[1] == 0xfe
                && (BOMBytes.Length < 4
                    || BOMBytes[2] != 0
                    || BOMBytes[3] != 0
                    )
                )
                return Encoding.Unicode;

            if (BOMBytes[0] == 0xfe
                && BOMBytes[1] == 0xff
                )
                return Encoding.BigEndianUnicode;

            if (BOMBytes.Length < 3)
                return null;

            if (BOMBytes[0] == 0xef && BOMBytes[1] == 0xbb && BOMBytes[2] == 0xbf)
                return Encoding.UTF8;

            if (BOMBytes[0] == 0x2b && BOMBytes[1] == 0x2f && BOMBytes[2] == 0x76)
                return Encoding.UTF7;

            if (BOMBytes.Length < 4)
                return null;

            if (BOMBytes[0] == 0xff && BOMBytes[1] == 0xfe && BOMBytes[2] == 0 && BOMBytes[3] == 0)
                return Encoding.UTF32;

            if (BOMBytes[0] == 0 && BOMBytes[1] == 0 && BOMBytes[2] == 0xfe && BOMBytes[3] == 0xff)
                return Encoding.GetEncoding(12001);

            return null;
        }

        public static Encoding DetectUnicodeInByteSampleByHeuristics(byte[] SampleBytes)
        {
            long oddBinaryNullsInSample = 0;
            long evenBinaryNullsInSample = 0;
            long suspiciousUTF8SequenceCount = 0;
            long suspiciousUTF8BytesTotal = 0;
            long likelyUSASCIIBytesInSample = 0;

            //Cycle through, keeping count of binary null positions, possible UTF-8 
            //  sequences from upper ranges of Windows-1252, and probable US-ASCII 
            //  character counts.

            long currentPos = 0;
            int skipUTF8Bytes = 0;

            while (currentPos < SampleBytes.Length)
            {
                //binary null distribution
                if (SampleBytes[currentPos] == 0)
                {
                    if (currentPos % 2 == 0)
                        evenBinaryNullsInSample++;
                    else
                        oddBinaryNullsInSample++;
                }

                //likely US-ASCII characters
                if (IsCommonUSASCIIByte(SampleBytes[currentPos]))
                    likelyUSASCIIBytesInSample++;

                //suspicious sequences (look like UTF-8)
                if (skipUTF8Bytes == 0)
                {
                    int lengthFound = DetectSuspiciousUTF8SequenceLength(SampleBytes, currentPos);

                    if (lengthFound > 0)
                    {
                        suspiciousUTF8SequenceCount++;
                        suspiciousUTF8BytesTotal += lengthFound;
                        skipUTF8Bytes = lengthFound - 1;
                    }
                }
                else
                {
                    skipUTF8Bytes--;
                }

                currentPos++;
            }

            //1: UTF-16 LE - in english / european environments, this is usually characterized by a 
            //  high proportion of odd binary nulls (starting at 0), with (as this is text) a low 
            //  proportion of even binary nulls.
            //  The thresholds here used (less than 20% nulls where you expect non-nulls, and more than
            //  60% nulls where you do expect nulls) are completely arbitrary.

            if (((evenBinaryNullsInSample * 2.0) / SampleBytes.Length) < 0.2
                && ((oddBinaryNullsInSample * 2.0) / SampleBytes.Length) > 0.6
                )
                return Encoding.Unicode;


            //2: UTF-16 BE - in english / european environments, this is usually characterized by a 
            //  high proportion of even binary nulls (starting at 0), with (as this is text) a low 
            //  proportion of odd binary nulls.
            //  The thresholds here used (less than 20% nulls where you expect non-nulls, and more than
            //  60% nulls where you do expect nulls) are completely arbitrary.

            if (((oddBinaryNullsInSample * 2.0) / SampleBytes.Length) < 0.2
                && ((evenBinaryNullsInSample * 2.0) / SampleBytes.Length) > 0.6
                )
                return Encoding.BigEndianUnicode;


            //3: UTF-8 - Martin Dürst outlines a method for detecting whether something CAN be UTF-8 content 
            //  using regexp, in his w3c.org unicode FAQ entry: 
            //  http://www.w3.org/International/questions/qa-forms-utf-8
            //  adapted here for C#.
            string potentiallyMangledString = Encoding.ASCII.GetString(SampleBytes);
            Regex UTF8Validator = new Regex(@"\A("
                + @"[\x09\x0A\x0D\x20-\x7E]"
                + @"|[\xC2-\xDF][\x80-\xBF]"
                + @"|\xE0[\xA0-\xBF][\x80-\xBF]"
                + @"|[\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}"
                + @"|\xED[\x80-\x9F][\x80-\xBF]"
                + @"|\xF0[\x90-\xBF][\x80-\xBF]{2}"
                + @"|[\xF1-\xF3][\x80-\xBF]{3}"
                + @"|\xF4[\x80-\x8F][\x80-\xBF]{2}"
                + @")*\z");
            if (UTF8Validator.IsMatch(potentiallyMangledString))
            {
                //Unfortunately, just the fact that it CAN be UTF-8 doesn't tell you much about probabilities.
                //If all the characters are in the 0-127 range, no harm done, most western charsets are same as UTF-8 in these ranges.
                //If some of the characters were in the upper range (western accented characters), however, they would likely be mangled to 2-byte by the UTF-8 encoding process.
                // So, we need to play stats.

                // The "Random" likelihood of any pair of randomly generated characters being one 
                //   of these "suspicious" character sequences is:
                //     128 / (256 * 256) = 0.2%.
                //
                // In western text data, that is SIGNIFICANTLY reduced - most text data stays in the <127 
                //   character range, so we assume that more than 1 in 500,000 of these character 
                //   sequences indicates UTF-8. The number 500,000 is completely arbitrary - so sue me.
                //
                // We can only assume these character sequences will be rare if we ALSO assume that this
                //   IS in fact western text - in which case the bulk of the UTF-8 encoded data (that is 
                //   not already suspicious sequences) should be plain US-ASCII bytes. This, I 
                //   arbitrarily decided, should be 80% (a random distribution, eg binary data, would yield 
                //   approx 40%, so the chances of hitting this threshold by accident in random data are 
                //   VERY low). 

                if ((suspiciousUTF8SequenceCount * 500000.0 / SampleBytes.Length >= 1) //suspicious sequences
                    && (
                    //all suspicious, so cannot evaluate proportion of US-Ascii
                           SampleBytes.Length - suspiciousUTF8BytesTotal == 0
                           ||
                           likelyUSASCIIBytesInSample * 1.0 / (SampleBytes.Length - suspiciousUTF8BytesTotal) >= 0.8
                       )
                    )
                    return Encoding.UTF8;
            }

            return null;
        }

        private static bool IsCommonUSASCIIByte(byte testByte)
        {
            if (testByte == 0x0A //lf
                || testByte == 0x0D //cr
                || testByte == 0x09 //tab
                || (testByte >= 0x20 && testByte <= 0x2F) //common punctuation
                || (testByte >= 0x30 && testByte <= 0x39) //digits
                || (testByte >= 0x3A && testByte <= 0x40) //common punctuation
                || (testByte >= 0x41 && testByte <= 0x5A) //capital letters
                || (testByte >= 0x5B && testByte <= 0x60) //common punctuation
                || (testByte >= 0x61 && testByte <= 0x7A) //lowercase letters
                || (testByte >= 0x7B && testByte <= 0x7E) //common punctuation
                )
                return true;
            else
                return false;
        }

        private static int DetectSuspiciousUTF8SequenceLength(byte[] SampleBytes, long currentPos)
        {
            int lengthFound = 0;

            if (SampleBytes.Length >= currentPos + 1
                && SampleBytes[currentPos] == 0xC2
                )
            {
                if (SampleBytes[currentPos + 1] == 0x81
                    || SampleBytes[currentPos + 1] == 0x8D
                    || SampleBytes[currentPos + 1] == 0x8F
                    )
                    lengthFound = 2;
                else if (SampleBytes[currentPos + 1] == 0x90
                    || SampleBytes[currentPos + 1] == 0x9D
                    )
                    lengthFound = 2;
                else if (SampleBytes[currentPos + 1] >= 0xA0
                    && SampleBytes[currentPos + 1] <= 0xBF
                    )
                    lengthFound = 2;
            }
            else if (SampleBytes.Length >= currentPos + 1
                && SampleBytes[currentPos] == 0xC3
                )
            {
                if (SampleBytes[currentPos + 1] >= 0x80
                    && SampleBytes[currentPos + 1] <= 0xBF
                    )
                    lengthFound = 2;
            }
            else if (SampleBytes.Length >= currentPos + 1
                && SampleBytes[currentPos] == 0xC5
                )
            {
                if (SampleBytes[currentPos + 1] == 0x92
                    || SampleBytes[currentPos + 1] == 0x93
                    )
                    lengthFound = 2;
                else if (SampleBytes[currentPos + 1] == 0xA0
                    || SampleBytes[currentPos + 1] == 0xA1
                    )
                    lengthFound = 2;
                else if (SampleBytes[currentPos + 1] == 0xB8
                    || SampleBytes[currentPos + 1] == 0xBD
                    || SampleBytes[currentPos + 1] == 0xBE
                    )
                    lengthFound = 2;
            }
            else if (SampleBytes.Length >= currentPos + 1
                && SampleBytes[currentPos] == 0xC6
                )
            {
                if (SampleBytes[currentPos + 1] == 0x92)
                    lengthFound = 2;
            }
            else if (SampleBytes.Length >= currentPos + 1
                && SampleBytes[currentPos] == 0xCB
                )
            {
                if (SampleBytes[currentPos + 1] == 0x86
                    || SampleBytes[currentPos + 1] == 0x9C
                    )
                    lengthFound = 2;
            }
            else if (SampleBytes.Length >= currentPos + 2
                && SampleBytes[currentPos] == 0xE2
                )
            {
                if (SampleBytes[currentPos + 1] == 0x80)
                {
                    if (SampleBytes[currentPos + 2] == 0x93
                        || SampleBytes[currentPos + 2] == 0x94
                        )
                        lengthFound = 3;
                    if (SampleBytes[currentPos + 2] == 0x98
                        || SampleBytes[currentPos + 2] == 0x99
                        || SampleBytes[currentPos + 2] == 0x9A
                        )
                        lengthFound = 3;
                    if (SampleBytes[currentPos + 2] == 0x9C
                        || SampleBytes[currentPos + 2] == 0x9D
                        || SampleBytes[currentPos + 2] == 0x9E
                        )
                        lengthFound = 3;
                    if (SampleBytes[currentPos + 2] == 0xA0
                        || SampleBytes[currentPos + 2] == 0xA1
                        || SampleBytes[currentPos + 2] == 0xA2
                        )
                        lengthFound = 3;
                    if (SampleBytes[currentPos + 2] == 0xA6)
                        lengthFound = 3;
                    if (SampleBytes[currentPos + 2] == 0xB0)
                        lengthFound = 3;
                    if (SampleBytes[currentPos + 2] == 0xB9
                        || SampleBytes[currentPos + 2] == 0xBA
                        )
                        lengthFound = 3;
                }
                else if (SampleBytes[currentPos + 1] == 0x82
                    && SampleBytes[currentPos + 2] == 0xAC
                    )
                    lengthFound = 3;
                else if (SampleBytes[currentPos + 1] == 0x84
                    && SampleBytes[currentPos + 2] == 0xA2
                    )
                    lengthFound = 3;
            }

            return lengthFound;
        }
    }
}
