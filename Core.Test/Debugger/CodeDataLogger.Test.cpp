#include "pch.h"
#include "Debugger/CodeDataLogger.h"
#include "Debugger/Debugger.h"
#include "TestUtil/TempFile.h"

#include "CppUnitTest.h"

#include <algorithm>
#include <fstream>
#include <vector>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test_Debugger
{
	class TestLogger : public CodeDataLogger
	{
	public:
		TestLogger(uint32_t memSize = 0x100, uint32_t romCrc = 0)
			: CodeDataLogger(nullptr, MemoryType::SnesPrgRom, memSize, CpuType::Snes, romCrc)
		{
		}

		static const uint32_t PublicHeaderSize = HeaderSize;
		static constexpr std::string_view PublicHeader = CdlHeader;
	};

	TEST_CLASS(Test_CodeDataLogger)
	{
		void AssertRange(CodeDataLogger& logger, uint32_t start, uint32_t end, uint8_t expectedFlag, const wchar_t* message)
		{
			auto data = logger.GetRawData();
			const bool match = std::all_of(data + start, data + end + 1,
											[expectedFlag](uint8_t b) { return b == expectedFlag; });

			Assert::IsTrue(match, message);
		}

	public:

		TEST_METHOD(Constructor_Memsize_Large_Does_Not_Crash)
		{
			constexpr uint32_t largeSize = 0xFFFFFFFF;
			TestLogger logger(largeSize);
			Assert::IsNotNull(logger.GetRawData(), L"Data not set");
		}

		TEST_METHOD(Constructor_Memsize_Zero_Does_Not_Crash)
		{
			constexpr uint32_t zeroSize = 0;
			TestLogger logger(zeroSize);
			Assert::IsNotNull(logger.GetRawData(), L"Data not set");
		}

		TEST_METHOD(Constructor_Null_Debugger_Does_Not_Crash)
		{
			TestLogger logger(0x100);
			Assert::IsNotNull(logger.GetRawData(), L"Raw data buffer should be allocated even with null debugger");
		}

		TEST_METHOD(GetCdlData_Destination_Null_Pointer_Does_Not_Crash)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			logger.GetCdlData(0, memSize / 2, nullptr);
			Assert::IsTrue(true, L"If we reached here, GetCdlData didn't crash");
		}
		TEST_METHOD(GetCdlData_Valid_Offset_Valid_Length_Reads_Data)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t copySize = memSize * 2;

			uint8_t output[copySize];
			std::fill(std::begin(output), std::end(output), 0xEE);

			constexpr uint32_t offset = 10;
			constexpr uint32_t length = 10;

			logger.GetCdlData(offset, length, output);

			const bool copied = std::all_of(output, output + length,
				[](uint8_t b) { return b == 0x00; });
			Assert::IsTrue(copied, L"data not copied");
			const bool unchanged = std::all_of(output + length, output + copySize,
				[](uint8_t b) { return b == 0xEE; });
			Assert::IsTrue(unchanged, L"data copied too far!");
		}
		TEST_METHOD(GetCdlData_Valid_Offset_Invalid_Length_Stops)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t copySize = memSize * 2;

			uint8_t output[copySize];
			std::fill(std::begin(output), std::end(output), 0xEE);

			logger.GetCdlData(10, memSize + 10, output);
			const bool unchanged = std::all_of(std::begin(output), std::end(output),
				[](uint8_t b) { return b == 0xEE; });
			Assert::IsTrue(unchanged, L"memcpy ran off the end");
		}
		TEST_METHOD(GetCdlData_Invalid_Offset_Valid_Length_Stops)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t copySize = memSize * 2;

			uint8_t output[copySize];
			std::fill(std::begin(output), std::end(output), 0xEE);

			logger.GetCdlData(memSize + 10, 10, output);
			const bool unchanged = std::all_of(std::begin(output), std::end(output),
				[](uint8_t b) { return b == 0xEE; });
			Assert::IsTrue(unchanged, L"memcpy ran off the end");
		}
		TEST_METHOD(GetCdlData_Invalid_Offset_Invalid_Length_Stops)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t copySize = memSize * 2;

			uint8_t output[copySize];
			std::fill(std::begin(output), std::end(output), 0xEE);

			logger.GetCdlData(memSize + 10, memSize + 10, output);
			const bool unchanged = std::all_of(std::begin(output), std::end(output),
				[](uint8_t b) { return b == 0xEE; });
			Assert::IsTrue(unchanged, L"memcpy ran off the end");
		}

		TEST_METHOD(GetCdlData_Can_Read_Last_Byte)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			uint8_t out = 0xEE;

			logger.GetCdlData(memSize - 1, 1, &out);
			Assert::AreNotEqual((uint8_t)0xEE, out, L"Failed to read the last valid byte of the buffer!");
		}
		TEST_METHOD(GetFlags_Addr_Outside_Memsize_Returns)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t outOfBoundsAddr = 0xFFFFFFFF;
			Assert::IsTrue(outOfBoundsAddr > memSize);
			Assert::AreEqual((uint8_t)0, logger.GetFlags(outOfBoundsAddr), L"GetFlags should return 0 for OOB");
		}

		TEST_METHOD(GetFunctions_Returns_Data)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			logger.MarkBytesAs(0, 49, CdlFlags::SubEntryPoint);

			struct
			{
				uint32_t list[10];
			} data;

			const uint32_t count = logger.GetFunctions(data.list, 10);
			Assert::AreEqual((uint32_t)10, count, L"Count should be capped at maxSize");
			Assert::AreEqual((uint32_t)0, data.list[0], L"First entry should be address 0");
			Assert::AreEqual((uint32_t)9, data.list[9], L"Last entry should be address 9");
		}

		TEST_METHOD(GetFunctions_NullBuffer_Does_Not_Crash)
		{
			TestLogger logger(0x100);
			logger.MarkBytesAs(0, 0, CdlFlags::SubEntryPoint);
			logger.GetFunctions(nullptr, 10);
		}

		TEST_METHOD(GetStatistics_Returns_Correct_Numbers)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);

			constexpr uint32_t codeStart = 0;
			constexpr uint32_t codeEnd = 10;
			logger.MarkBytesAs(codeStart, codeEnd, CdlFlags::Code);

			constexpr uint32_t dataStart = 20;
			constexpr uint32_t dataEnd = 32;
			logger.MarkBytesAs(dataStart, dataEnd, CdlFlags::Data);

			const CdlStatistics stats = logger.GetStatistics();

			Assert::AreEqual(codeEnd - codeStart + 1, stats.CodeBytes, L"Code stats incorrect");
			Assert::AreEqual(dataEnd - dataStart + 1, stats.DataBytes, L"Data stats incorrect");
			Assert::AreEqual(memSize, stats.TotalBytes, L"Total bytes incorrect");
		}

		TEST_METHOD(Integration_MarkBytes_Then_SaveFile_Then_LoadFile_Works)
		{
			constexpr uint32_t memSize = 0x10;
			constexpr uint32_t romCrc = 0x11111111;
			TempFile testFile("integration.cdl");
			TestLogger logger(memSize, romCrc);

			logger.MarkBytesAs(0, 0, CdlFlags::Code);
			logger.MarkBytesAs(1, 1, CdlFlags::Data);
			logger.MarkBytesAs(2, 2, CdlFlags::JumpTarget);
			logger.MarkBytesAs(3, 3, CdlFlags::SubEntryPoint);
			logger.MarkBytesAs(4, 4, CdlFlags::Code | CdlFlags::JumpTarget);

			Assert::IsTrue(logger.SaveCdlFile(testFile), L"Could not save file");

			TestLogger reader(memSize, romCrc);
			Assert::IsTrue(reader.LoadCdlFile(testFile, true), L"Could not read file");

			Assert::IsTrue(reader.IsCode(0), L"Flag lost: Code");
			Assert::IsTrue(reader.IsData(1), L"Flag lost: Data");
			Assert::IsTrue(reader.IsJumpTarget(2), L"Flag lost: JumpTarget");
			Assert::IsTrue(reader.IsSubEntryPoint(3), L"Flag lost: SubEntryPoint");
			Assert::AreEqual((uint8_t)(CdlFlags::Code | CdlFlags::JumpTarget), reader.GetFlags(4), L"Combined flags mismatch");
		}

		TEST_METHOD(IsCode_Addr_Outside_Memsize_Clamps)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t dangerousAddr = 0xFFFFFFFF;
			Assert::IsFalse(logger.IsCode(dangerousAddr), L"crashed or returned garbage for high address");
		}
		TEST_METHOD(IsData_Addr_Outside_Memsize_Clamps)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t dangerousAddr = 0xFFFFFFFF;
			Assert::IsFalse(logger.IsData(dangerousAddr), L"crashed or returned garbage for high address");
		}

		TEST_METHOD(IsJumpTarget_Addr_Outside_Memsize_Clamps)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t dangerousAddr = 0xFFFFFFFF;
			Assert::IsFalse(logger.IsJumpTarget(dangerousAddr), L"crashed or returned garbage for high address");
		}
		TEST_METHOD(IsSubEntryPoint_Addr_Outside_Memsize_Clamps)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);
			constexpr uint32_t dangerousAddr = 0xFFFFFFFF;
			Assert::IsFalse(logger.IsSubEntryPoint(dangerousAddr), L"crashed or returned garbage for high address");
		}

		TEST_METHOD(LoadCdlFile_NonExistentFile_ShouldReturnFalse)
		{
			TestLogger logger(0x100, 0);
			Assert::IsFalse(logger.LoadCdlFile("does_not_exist.cdl", true), L"Should fail for non-existent file");
		}

		TEST_METHOD(LoadCdlFile_File_Too_Small_Has_Empty_Data)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize, 0x11111111);
			TempFile testFile("too_small.cdl");

			{
				ofstream outFile(testFile, std::ios::binary | std::ios::trunc);
				outFile.write("123", 3);
				outFile.close();
			}

			Assert::IsFalse(logger.LoadCdlFile(testFile, true), L"Should reject truncated file");
			AssertRange(logger, 0, memSize - 1, 0, L"File too small: All cdl file data should be set back to zeros.");
		}

		TEST_METHOD(LoadCdlFile_AutoResetCdl_False_CRC_Match_Loads_Data)
		{
			constexpr uint32_t memSize = 0x100;
			constexpr uint32_t romCrc = 0x11111111;
			TempFile testFile("load_crc_match_no_reset.cdl");
			constexpr bool AutoResetCdl = false;

			TestLogger logger(memSize, romCrc);
			logger.MarkBytesAs(0, 10, CdlFlags::Code);
			logger.MarkBytesAs(11, 20, CdlFlags::Data);
			Assert::IsTrue(logger.SaveCdlFile(testFile), L"Failed to save CDL file");

			TestLogger reader(memSize, romCrc);
			Assert::IsTrue(reader.LoadCdlFile(testFile, AutoResetCdl), L"Failed to load CDL file");
			AssertRange(reader, 0, 10, CdlFlags::Code, L"Range 0-10 should be Code");
			AssertRange(reader, 11, 20, CdlFlags::Data, L"Range 11-20 should be Data");
		}

		TEST_METHOD(LoadCdlFile_AutoResetCdl_True_CRC_Match_Loads_Data)
		{
			constexpr uint32_t memSize = 0x100;
			constexpr uint32_t romCrc = 0x11111111;
			TempFile testFile("load_crc_match_with_reset.cdl");
			constexpr bool AutoResetCdl = true;

			TestLogger logger(memSize, romCrc);
			logger.MarkBytesAs(0, 10, CdlFlags::Code);
			logger.MarkBytesAs(11, 20, CdlFlags::Data);
			Assert::IsTrue(logger.SaveCdlFile(testFile), L"Failed to save CDL file");

			TestLogger reader(memSize, romCrc);
			Assert::IsTrue(reader.LoadCdlFile(testFile, AutoResetCdl), L"Failed to load CDL file");
			AssertRange(reader, 0, 10, CdlFlags::Code, L"Range 0-10 should be Code");
			AssertRange(reader, 11, 20, CdlFlags::Data, L"Range 11-20 should be Data");
		}

		TEST_METHOD(LoadCdlFile_AutoResetCdl_False_CRC_Mismatch_Loads_Data)
		{
			constexpr uint32_t memSize = 0x100;
			constexpr uint32_t romCrc = 0x11111111;
			TempFile testFile("load_crc_bad_no_reset.cdl");
			constexpr bool AutoResetCdl = false;

			TestLogger logger(memSize, romCrc);
			logger.MarkBytesAs(0, 10, CdlFlags::Code);
			logger.MarkBytesAs(11, 20, CdlFlags::Data);
			Assert::IsTrue(logger.SaveCdlFile(testFile), L"Failed to save CDL file");

			constexpr uint32_t badCrc = 0x0BAD0BAD;
			TestLogger reader(memSize, badCrc);
			Assert::IsTrue(reader.LoadCdlFile(testFile, AutoResetCdl), L"Failed to load CDL file");
			AssertRange(reader, 0, 10, CdlFlags::Code, L"Range 0-10 should be Code");
			AssertRange(reader, 11, 20, CdlFlags::Data, L"Range 11-20 should be Data");
		}

		TEST_METHOD(LoadCdlFile_AutoResetCdl_True_CRC_Mismatch_Zeros_Data)
		{
			constexpr uint32_t memSize = 0x100;
			constexpr uint32_t romCrc = 0x11111111;
			TempFile testFile("load_crc_bad_with_reset.cdl");
			constexpr bool AutoResetCdl = true;

			TestLogger logger(memSize, romCrc);
			logger.MarkBytesAs(0, 10, CdlFlags::Code);
			logger.MarkBytesAs(11, 20, CdlFlags::Data);
			Assert::IsTrue(logger.SaveCdlFile(testFile), L"Failed to save CDL file");

			constexpr uint32_t badCrc = 0x0BAD0BAD;
			TestLogger reader(memSize, badCrc);
			Assert::IsFalse(reader.LoadCdlFile(testFile, AutoResetCdl), L"LoadCdlFile() should return false");

			AssertRange(reader, 0, memSize - 1, 0, L"CRC Mismatch and AudoReset True: All cdl file data should be set back to zeros.");
		}

		TEST_METHOD(LoadCdlFile_Legacy_Format_Loads)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize, 0x11111111);
			TempFile testFile("legacy.cdl");

			// Create a file with no header, just raw data
			{
				ofstream outFile(testFile, std::ios::binary);
				vector<uint8_t> rawData(memSize, 0xCC);
				outFile.write((char*)rawData.data(), memSize);
				outFile.close();
			}

			Assert::IsTrue(logger.LoadCdlFile(testFile, true), L"Could not load legacy cdl file");
			Assert::AreEqual((uint8_t)0xCC, logger.GetRawData()[0], L"Legacy data mismatch");
		}

		TEST_METHOD(MarkBytesAs_End_Greater_Than_Memsize_Clamps)
		{
			constexpr uint32_t memSize = 0x100;
			constexpr uint32_t endTooFar = memSize + 0x100;
			constexpr uint32_t start = 90;
			Assert::IsTrue(start + endTooFar > memSize, L"Incorrect test setup: MarkBytes outside of memSize.");

			TestLogger logger(memSize);
			logger.MarkBytesAs(start, endTooFar, CdlFlags::Code);

			Assert::AreEqual((uint8_t)CdlFlags::Code, logger.GetFlags(start), L"Start of buffer should be marked safely");
			Assert::AreEqual((uint8_t)CdlFlags::Code, logger.GetFlags(memSize - 1), L"End of buffer should be marked safely");
			Assert::AreEqual((uint8_t)0, logger.GetFlags(memSize), L"Data past end should not be modified");
		}

		TEST_METHOD(SaveCdlFile_Writes_Expected_Format)
		{
			constexpr uint32_t memSize = 0x10;
			constexpr uint32_t romCrc = 0x11111111;
			TempFile testFile("save_test.cdl");

			TestLogger logger(memSize, romCrc);
			logger.MarkBytesAs(0, 0x07, CdlFlags::Code);
			logger.MarkBytesAs(0x08, 0x0F, CdlFlags::Data);
			Assert::IsTrue(logger.SaveCdlFile(testFile), L"SaveCdlFile returned false");

			std::ifstream inFile(testFile, std::ios::binary);
			std::vector<uint8_t> readData((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
			inFile.close();

			constexpr size_t expectedSize = TestLogger::PublicHeaderSize + memSize;
			Assert::AreEqual(expectedSize, readData.size(), L"Saved file size mismatch");

			Assert::AreEqual(0, memcmp(readData.data(), TestLogger::PublicHeader.data(), TestLogger::PublicHeader.length()), L"Header mismatch");

			const uint32_t savedCrc = readData[5] |
				(readData[6] << 8) |
				(readData[7] << 16) |
				(readData[8] << 24);

			Assert::AreEqual(romCrc, savedCrc, L"CRC32 in file mismatch");

			const uint8_t* actualData = logger.GetRawData();
			for(uint32_t i = 0; i < memSize; i++) {
				Assert::AreEqual(actualData[i], readData[TestLogger::PublicHeaderSize + i],
					L"Payload data mismatch at index");
			}
		}

		TEST_METHOD(SetCdlData_Length_Greater_Than_Memsize_Does_Not_Crash)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);

			uint8_t sourceData[] = { 0xAA, 0xBB, 0xCC, 0xDD };

			logger.SetCdlData(sourceData, 0xFFFFFFFF);
			const uint8_t* internalData = logger.GetRawData();
			for(uint32_t i = 0; i < memSize; i++) {
				Assert::AreEqual((uint8_t)0, internalData[i], L"Data was modified despite invalid length");
			}
		}

		TEST_METHOD(SetCdlData_Null_Pointer_Does_Not_Crash)
		{
			constexpr uint32_t memSize = 0x100;
			TestLogger logger(memSize);

			logger.SetCdlData(nullptr, 1);
			Assert::IsTrue(true, L"SetCdlData did not crash.");
		}
		TEST_METHOD(StripData_Null_Rombuffer_Does_Not_Crash)
		{
			TestLogger logger(0x100);
			logger.StripData(nullptr, CdlStripOption::StripUnused);
			Assert::IsTrue(true, L"StripData did not crash.");
		}
	};
}
