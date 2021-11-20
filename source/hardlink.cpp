#include <set>
#include <deque>
#include <mutex>
#include <string>
#include <cassert>
#include <sstream>
#include <iostream>
#include <filesystem>


namespace fs = std::filesystem;


static std::wstring s_srcRawPath;
static std::wstring s_desRawPath;
static std::deque<fs::path> s_pendingPaths;
static std::set<fs::path> s_processingPaths;
static std::mutex s_lock;
static std::mutex s_printLock;


static fs::path ReplacePath(const fs::path& orig)
{
	std::wstring rawSrc = orig.native();
	size_t index = rawSrc.find(s_srcRawPath);
	if (index == 0)
	{
		std::wstring rawDes = rawSrc.replace(index, s_srcRawPath.length(), s_desRawPath);
		return fs::path(rawDes);
	}
	return fs::path();
}


static void PushPendingPath(const fs::path& path)
{
	s_pendingPaths.push_back(path);
}


static fs::path PopPendingPath()
{
	fs::path path;
	if (s_pendingPaths.size())
	{
		path = s_pendingPaths.front();
		s_pendingPaths.pop_front();
	}
	return path;
}


static void AddProcessingPath(const fs::path& path)
{
	auto it = s_processingPaths.find(path);
	assert(it == s_processingPaths.end());
	s_processingPaths.insert(path);
}


static void RemoveProcessingPath(const fs::path& path)
{
	auto it = s_processingPaths.find(path);
	assert(it != s_processingPaths.end());
	s_processingPaths.erase(it);
}


static void FindNewPath(const fs::path& path)
{
	std::lock_guard lock(s_lock);
	PushPendingPath(path);
}


static fs::path ExploreNewPath()
{
	std::lock_guard lock(s_lock);
	const fs::path path = PopPendingPath();
	if (!path.empty())
	{
		AddProcessingPath(path);
	}
	return path;
}


static void ExplorePathDone(const fs::path& path)
{
	std::lock_guard lock(s_lock);
	RemoveProcessingPath(path);
}


static bool HasWork()
{
	std::lock_guard lock(s_lock);
	return s_pendingPaths.size() != 0 || s_processingPaths.size() != 0;
}


void SetHardLinkPath(const std::wstring& srcPath, const std::wstring& desPath)
{
	s_srcRawPath = srcPath;
	s_desRawPath = desPath;

	PushPendingPath(s_srcRawPath);
}


static void PrintLog(bool isFile, bool isNew)
{
	std::lock_guard lock(s_printLock);

	static int numFile = 0;
	static int numDir = 0;
	static int numFileTotal = 0;
	static int numDirTotal = 1;

	if (isNew)
	{
		isFile ? numFile++ : numDir++;
	}
	else
	{
		isFile ? numFileTotal++ : numDirTotal++;
	}
	std::ostringstream oss;
	oss << "\rFile: " << numFile << "/" << numFileTotal << " Folder: " << numDir << "/" << numDirTotal;
	std::cout << oss.str();
}


void CreateHardLink(const fs::path& path, const fs::path& link)
{
	fs::create_hard_link(path, link);
}


bool Explore()
{
	const fs::path& srcPath = ExploreNewPath();
	if (fs::exists(status(srcPath)))
	{
		const fs::path& desPath = ReplacePath(srcPath);
		if (!fs::exists(status(desPath)))
		{
			bool success = fs::create_directories(desPath);
			assert(success);
			PrintLog(false, true);
		}

		for (auto& p : fs::directory_iterator(srcPath))
		{
			const fs::path& srcPath = p.path();
			const fs::path& desPath = ReplacePath(srcPath);
			bool isFile = !fs::is_directory(srcPath);
			PrintLog(isFile, false);
			if (isFile)
			{
				if (fs::exists(desPath))
				{
					continue;
				}
				CreateHardLink(srcPath, desPath);
				PrintLog(true, true);
			}
			else
			{
				FindNewPath(srcPath);
			}
		}
		ExplorePathDone(srcPath);
	}
	return HasWork();
}
