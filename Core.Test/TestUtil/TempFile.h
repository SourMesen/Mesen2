#pragma once

#include <string>
#include <filesystem>

namespace fs = std::filesystem;

// Helper function - make it easy to create and clean up temp files on disk.
struct TempFile
{
	fs::path FilePath;

	/**
	 * Create a temporary file in the OS temp directory.
	 * 
	 * TempFiles are automatically deleted once the object goes out of scope,
	 * so you can easily create TempFiles for testing and don't need to worry about leaving files around.
	 * e.g.
	 * 
	 * {
	 *		TempFile yourfile("name.txt");
	 *    // your code here...
	 * }
	 * // TempFile automatically deleted when scope exits the {} brackets.
	 *
	 * @param filename Filename to use, including extension if desired.
	 */
	TempFile(std::string filename)
	{
		FilePath = fs::temp_directory_path() / filename;
	}

	~TempFile()
	{
		std::error_code ec;
		fs::remove(FilePath, ec);
	}

	TempFile(const TempFile&) = delete;
	TempFile& operator=(const TempFile&) = delete;
	TempFile(TempFile&& other) noexcept : FilePath(std::move(other.FilePath))
	{
		other.FilePath.clear(); // Ensure the old object doesn't delete the file
	}
	TempFile& operator=(TempFile&& other) noexcept
	{
		if(this != &other) {
			FilePath = std::move(other.FilePath);
			other.FilePath.clear();
		}
		return *this;
	}

	std::string str() const { return FilePath.string(); }
	operator std::string() const { return FilePath.string(); }
	operator const fs::path& () const { return FilePath; }
};
