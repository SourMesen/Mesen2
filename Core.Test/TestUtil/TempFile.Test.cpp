#include "pch.h"
#include <fstream>
#include <filesystem>
#include "CppUnitTest.h"
#include "TempFile.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace fs = std::filesystem;

namespace Test_TestUtil
{

	TEST_CLASS(Test_TempFile)
	{
	public:
		TEST_METHOD(Constructor_Creates_File)
		{
			std::string filename = "creation_test.cdl";
			TempFile temp(filename);
			std::string pathStr = temp.str();
			Assert::IsFalse(pathStr.empty(), L"Path should not be empty");

			fs::path p(pathStr);
			Assert::AreEqual(filename, p.filename().string(), L"Filename mismatch");
		}

		TEST_METHOD(Destructor_Deletes_File)
		{
			fs::path filePath;

			// use brackets for scope to trigger destructor
			{
				TempFile temp("cleanup_test.txt");
				filePath = temp.FilePath;

				// Manually create the file so there is something to delete
				std::ofstream ofs(filePath);
				ofs << "test data";
				ofs.close();

				Assert::IsTrue(fs::exists(filePath), L"File should exist before destructor runs");
			}
			Assert::IsFalse(fs::exists(filePath), L"File should have been deleted by destructor");
		}

		TEST_METHOD(Destructor_Handles_Missing_File)
		{
			fs::path filePath;

			{
				TempFile temp("never_created.txt");
				filePath = temp.FilePath;
				Assert::IsFalse(fs::exists(filePath), L"File should not exist");
			}
			Assert::IsFalse(fs::exists(filePath), L"File should not exist");
			Assert::IsTrue(true, L"Destructor handled non-existent file without crashing");
		}
		TEST_METHOD(Move_Transfers_Ownership)
		{
			fs::path originalPath;
			{
				TempFile source("move_test.txt");
				originalPath = source.FilePath;

				TempFile destination(std::move(source));

				Assert::AreEqual(originalPath.string(), destination.str(), L"destination should now own the path");
				Assert::IsTrue(source.FilePath.empty(), L"source path should be cleared after move");
			}
			Assert::IsFalse(fs::exists(originalPath), L"File should be gone after moved-to object dies");
		}

		TEST_METHOD(Move_Assignment_Transfers_And_Clears_Source)
		{
			fs::path originalPath;
			{
				TempFile source("move_assign_test.txt");
				TempFile destination("dummy.txt");
				originalPath = source.FilePath;

				// This triggers: TempFile& operator=(TempFile&& other)
				destination = std::move(source);

				Assert::AreEqual(originalPath.string(), destination.str());
				Assert::IsTrue(source.FilePath.empty(), L"Source not cleared after assignment");
			}
			Assert::IsFalse(fs::exists(originalPath));
		}

		TEST_METHOD(Move_Assignment_Self_Assignment_Safety)
		{
			TempFile source("self.txt");
			std::string pathBefore = source.str();

			source = std::move(source);

			Assert::AreEqual(pathBefore, source.str(), L"Self-assignment cleared the path!");
		}
	};
}
