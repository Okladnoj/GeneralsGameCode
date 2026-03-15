/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

////////////////////////////////////////////////////////////////////////////////
//																																						//
//  (c) 2001-2003 Electronic Arts Inc.																				//
//																																						//
////////////////////////////////////////////////////////////////////////////////

///////// StdLocalFileSystem.cpp /////////////////////////
// Stephan Vedder, April 2025
////////////////////////////////////////////////////////////

#include "Common/AsciiString.h"
#include "Common/GameMemory.h"
#include "Common/PerfTimer.h"
#include "StdDevice/Common/StdLocalFileSystem.h"
#include "StdDevice/Common/StdLocalFile.h"

#include <algorithm>
#include <filesystem>
#include <strings.h>

StdLocalFileSystem::StdLocalFileSystem() : LocalFileSystem()
{
}

StdLocalFileSystem::~StdLocalFileSystem() {
}

void StdLocalFileSystem::addSearchPath(const AsciiString& path) {
	if (path.isEmpty()) {
		return;
	}

	std::string normalized = path.str();
	if (normalized.back() != '/' && normalized.back() != '\\') {
		normalized += '/';
	}

	for (const auto& existing : m_searchPaths) {
		if (existing == normalized) {
			return;
		}
	}

	printf("StdLocalFileSystem::addSearchPath - '%s'\n", normalized.c_str());
	fflush(stdout);
	m_searchPaths.push_back(std::move(normalized));
}

//DECLARE_PERF_TIMER(StdLocalFileSystem_openFile)
static std::filesystem::path fixFilenameFromWindowsPath(const Char *filename, Int access)
{
	std::string fixedFilename(filename);

#ifndef _WIN32
	std::replace(fixedFilename.begin(), fixedFilename.end(), '\\', '/');
#endif

	std::error_code ec;
	std::filesystem::path p(fixedFilename);

	if (std::filesystem::exists(p, ec)) {
		return p;
	}

	std::filesystem::path currentPath = ".";
	bool isAbsolute = p.is_absolute();
	if (isAbsolute) {
		currentPath = p.root_path();
	}

	for (auto const &component : p) {
		if (component.empty() || component == "/" || component == "." ||
			component == "..")
			continue;

		bool found = false;
		if (std::filesystem::exists(currentPath, ec) &&
			std::filesystem::is_directory(currentPath, ec)) {
			for (auto const &entry :
				 std::filesystem::directory_iterator(currentPath, ec)) {
				std::string filenameStr = entry.path().filename().string();
				if (strcasecmp(filenameStr.c_str(), component.string().c_str()) == 0) {
					currentPath /= entry.path().filename();
					found = true;
					break;
				}
			}
		}

		if (!found) {
			if (access & File::WRITE) {
				currentPath /= component;
			} else {
				return std::filesystem::path();
			}
		}
	}

	return currentPath;
}

static std::filesystem::path resolveWithSearchPaths(
	const Char *filename,
	Int access,
	const std::vector<std::string>& searchPaths) {

	std::filesystem::path path = fixFilenameFromWindowsPath(filename, access);
	if (!path.empty()) {
		return path;
	}

	if (access & File::WRITE) {
		return path;
	}

	std::string fixedRelative(filename);
#ifndef _WIN32
	std::replace(fixedRelative.begin(), fixedRelative.end(), '\\', '/');
#endif

	for (const auto& searchPath : searchPaths) {
		std::string fullPath = searchPath + fixedRelative;
		std::filesystem::path resolved = fixFilenameFromWindowsPath(fullPath.c_str(), access);
		if (!resolved.empty()) {
			return resolved;
		}
	}

	return std::filesystem::path();
}

File * StdLocalFileSystem::openFile(const Char *filename, Int access, size_t bufferSize)
{
	if (strlen(filename) <= 0) {
		return nullptr;
	}

	std::filesystem::path path = resolveWithSearchPaths(filename, access, m_searchPaths);

	if (path.empty()) {
		return nullptr;
	}

	if (access & File::WRITE) {
		std::filesystem::path dir = path.parent_path();
		if (!dir.empty()) {
			std::error_code ec;
			if (!std::filesystem::exists(dir, ec) || ec) {
				if(!std::filesystem::create_directories(dir, ec) || ec) {
					DEBUG_LOG(("StdLocalFileSystem::openFile - Error creating directory %s", dir.string().c_str()));
					return nullptr;
				}
			}
		}
	}

	StdLocalFile *file = newInstance( StdLocalFile );

	if (file->open(path.string().c_str(), access, bufferSize) == FALSE) {
		deleteInstance(file);
		file = nullptr;
	} else {
		file->deleteOnClose();
	}

	return file;
}

void StdLocalFileSystem::update()
{
}

void StdLocalFileSystem::init()
{
}

void StdLocalFileSystem::reset()
{
}

//DECLARE_PERF_TIMER(StdLocalFileSystem_doesFileExist)
Bool StdLocalFileSystem::doesFileExist(const Char *filename) const
{
	std::filesystem::path path = resolveWithSearchPaths(filename, 0, m_searchPaths);
	if(path.empty()) {
		return FALSE;
	}

	std::error_code ec;
	return std::filesystem::exists(path, ec);
}

void StdLocalFileSystem::getFileListInDirectory(const AsciiString& currentDirectory, const AsciiString& originalDirectory, const AsciiString& searchName, FilenameList & filenameList, Bool searchSubdirectories) const
{

	AsciiString asciisearch;
	asciisearch = originalDirectory;
	asciisearch.concat(currentDirectory);
	auto searchExt = std::filesystem::path(searchName.str()).extension();
	if (asciisearch.isEmpty()) {
		asciisearch = ".";
	}

	std::string fixedDirectory(asciisearch.str());

#ifndef _WIN32
	// Replace backslashes with forward slashes on unix
	std::replace(fixedDirectory.begin(), fixedDirectory.end(), '\\', '/');
#endif

	Bool done = FALSE;
	std::error_code ec;

	auto iter = std::filesystem::directory_iterator(fixedDirectory.c_str(), ec);
	// The default iterator constructor creates an end iterator
	done = iter == std::filesystem::directory_iterator();

#ifdef __APPLE__
	// TheSuperHackers @info If directory not found in CWD, try search paths
	std::string resolvedDir;
	if (ec && !std::filesystem::path(fixedDirectory).is_absolute()) {
		for (const auto& searchPath : m_searchPaths) {
			resolvedDir = searchPath + fixedDirectory;
			std::error_code search_ec;
			iter = std::filesystem::directory_iterator(resolvedDir.c_str(), search_ec);
			if (!search_ec) {
				ec = search_ec;
				done = iter == std::filesystem::directory_iterator();
				break;
			}
		}
	}
#endif

	if (ec) {
		return;
	}

	while (!done)	{
		std::string filenameStr = iter->path().filename().string();
		if (!iter->is_directory() && iter->path().extension() == searchExt &&
			(strcmp(filenameStr.c_str(), ".") != 0 && strcmp(filenameStr.c_str(), "..") != 0)) {
			AsciiString newFilename = asciisearch;
			if (newFilename.str()[newFilename.getLength()-1] != '/' &&
				newFilename.str()[newFilename.getLength()-1] != '\\') {
				newFilename.concat('\\');
			}
			newFilename.concat(filenameStr.c_str());
			if (filenameList.find(newFilename) == filenameList.end()) {
				filenameList.insert(newFilename);
			}
		}

		iter++;
		done = iter == std::filesystem::directory_iterator();
	}

	if (searchSubdirectories) {
		std::string subScanDir = fixedDirectory;
#ifdef __APPLE__
		if (!resolvedDir.empty()) {
			subScanDir = resolvedDir;
		}
#endif
		auto subIter = std::filesystem::directory_iterator(subScanDir, ec);

		if (ec) {
			return;
		}

		done = subIter == std::filesystem::directory_iterator();

		while (!done) {
			std::string filenameStr = subIter->path().filename().string();
			if(subIter->is_directory() &&
				(strcmp(filenameStr.c_str(), ".") != 0 && strcmp(filenameStr.c_str(), "..") != 0)) {
				AsciiString tempsearchstr(filenameStr.c_str());

				getFileListInDirectory(tempsearchstr, asciisearch, searchName, filenameList, searchSubdirectories);
			}

			subIter++;
			done = subIter == std::filesystem::directory_iterator();
		}
	}
}

Bool StdLocalFileSystem::getFileInfo(const AsciiString& filename, FileInfo *fileInfo) const
{
	std::filesystem::path path = resolveWithSearchPaths(filename.str(), 0, m_searchPaths);

	if(path.empty()) {
		return FALSE;
	}

	std::error_code ec;
	auto file_size = std::filesystem::file_size(path, ec);
	if (ec)
	{
		return FALSE;
	}

	auto write_time = std::filesystem::last_write_time(path, ec);
	if (ec)
	{
		return FALSE;
	}

	// TODO: fix this to be win compatible (time since 1601)
	auto time = write_time.time_since_epoch().count();
	fileInfo->timestampHigh = time >> 32;
	fileInfo->timestampLow = time & UINT32_MAX;
	fileInfo->sizeHigh      = file_size >> 32;
	fileInfo->sizeLow  = file_size & UINT32_MAX;

	return TRUE;
}

Bool StdLocalFileSystem::createDirectory(AsciiString directory)
{
	bool result = FALSE;

	std::string fixedDirectory(directory.str());

#ifndef _WIN32
	// Replace backslashes with forward slashes on unix
	std::replace(fixedDirectory.begin(), fixedDirectory.end(), '\\', '/');
#endif

	if ((!fixedDirectory.empty()) && (fixedDirectory.length() < _MAX_DIR)) {
		// Convert to host path
		std::filesystem::path path(std::move(fixedDirectory));

		std::error_code ec;
		result = std::filesystem::create_directory(path, ec);
		if (ec) {
			result = FALSE;
		}
	}
	return result;
}

AsciiString StdLocalFileSystem::normalizePath(const AsciiString& filePath) const
{
	std::string nonNormalized(filePath.str());
#ifndef _WIN32
	// Replace backslashes with forward slashes on non-Windows platforms
	std::replace(nonNormalized.begin(), nonNormalized.end(), '\\', '/');
#endif
	std::filesystem::path pathNonNormalized(nonNormalized);
	return AsciiString(pathNonNormalized.lexically_normal().string().c_str());
}
