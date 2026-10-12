// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file  GitHelper.cpp
 *
 * @brief Implementation of helpers to read file versions from git.
 */

#include "StdAfx.h"
#include "GitHelper.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <Poco/Process.h>
#include <Poco/Pipe.h>
#include <Poco/Exception.h>
#include "paths.h"
#include "unicoder.h"

namespace git
{

/**
 * @brief Run git in a folder and capture its stdout.
 * @return Exit code of git, or -1 if git could not be launched.
 */
static int RunGit(const String& dir, const std::vector<std::string>& args, std::string& output)
{
	try
	{
		Poco::Pipe outPipe, errPipe;
		Poco::ProcessHandle ph = Poco::Process::launch("git", args, ucr::toUTF8(dir), nullptr, &outPipe, &errPipe);
		output.clear();
		char buf[8192];
		for (int n; (n = outPipe.readBytes(buf, sizeof(buf))) > 0; )
			output.append(buf, n);
		return ph.wait();
	}
	catch (Poco::Exception&)
	{
		return -1;
	}
}

static std::vector<std::string> SplitLines(const std::string& text)
{
	std::vector<std::string> lines;
	for (size_t pos = 0; pos < text.size();)
	{
		size_t end = text.find('\n', pos);
		if (end == std::string::npos)
			end = text.size();
		std::string line = text.substr(pos, end - pos);
		while (!line.empty() && (line.back() == '\r' || line.back() == ' '))
			line.pop_back();
		if (!line.empty())
			lines.push_back(line);
		pos = end + 1;
	}
	return lines;
}

std::vector<String> GetRecentFiles(const String& folder, size_t maxCount)
{
	namespace fs = std::filesystem;
	constexpr size_t maxVisited = 20000; // keeps huge trees from stalling the dialog
	std::vector<std::pair<fs::file_time_type, fs::path>> files;
	std::error_code ec;
	fs::recursive_directory_iterator it(fs::path(folder), fs::directory_options::skip_permission_denied, ec);
	for (size_t visited = 0; !ec && it != fs::recursive_directory_iterator() && visited < maxVisited; it.increment(ec), ++visited)
	{
		std::error_code entryEc;
		if (it->is_directory(entryEc))
		{
			if (it->path().filename() == ".git")
				it.disable_recursion_pending();
		}
		else if (it->is_regular_file(entryEc))
		{
			const fs::file_time_type time = it->last_write_time(entryEc);
			if (!entryEc)
				files.emplace_back(time, it->path());
		}
	}
	std::sort(files.begin(), files.end(), [](const auto& a, const auto& b) { return a.first > b.first; });
	if (files.size() > maxCount)
		files.resize(maxCount);
	std::vector<String> result;
	for (const auto& file : files)
		result.push_back(file.second.native());
	return result;
}

String GetFileHistory(const String& file, std::vector<Commit>& commits)
{
	commits.clear();
	std::string output;
	const String dir = paths::GetParentPath(file);
	const std::string name = ucr::toUTF8(paths::FindFileName(file));
	int rc = RunGit(dir, { "log", "-n", "50", "--date=short", "--format=%H%x09%h %ad %an: %s", "--", name }, output);
	if (rc == -1)
		return _("Cannot run git. Make sure git is installed and in the PATH.");
	if (rc != 0)
		return _("The file is not in a git repository.");
	for (const std::string& line : SplitLines(output))
	{
		size_t tab = line.find('\t');
		if (tab == std::string::npos)
			continue;
		commits.push_back({ line.substr(0, tab), ucr::toTString(line.substr(tab + 1)) });
	}
	if (commits.empty())
		return _("No version of the file was found in git.");
	return _T("");
}

String SaveFileVersion(const String& file, const std::string& rev, const String& destPath)
{
	const String dir = paths::GetParentPath(file);
	const std::string spec = rev + ":./" + ucr::toUTF8(paths::FindFileName(file));
	std::string output;
	// --filters converts line endings the same way as checkout does
	int rc = RunGit(dir, { "cat-file", "--filters", spec }, output);
	if (rc != 0)
		rc = RunGit(dir, { "show", spec }, output);
	if (rc == -1)
		return _("Cannot run git. Make sure git is installed and in the PATH.");
	if (rc != 0)
		return _("Cannot read the selected version of the file from git.");
	std::ofstream ofs(destPath, std::ios::binary);
	if (!ofs.write(output.data(), output.size()))
		return _("Cannot create a temporary file.");
	return _T("");
}

}
