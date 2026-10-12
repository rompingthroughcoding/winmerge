// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file  GitHelper.h
 *
 * @brief Declaration of helpers to read file versions from git.
 */
#pragma once

#include <string>
#include <vector>
#include "UnicodeString.h"

namespace git
{

struct Commit
{
	std::string hash;
	String label; /**< "<short hash> <date> <author>: <subject>" */
};

/**
 * @brief Get the most recently modified files below a folder, newest first.
 */
std::vector<String> GetRecentFiles(const String& folder, size_t maxCount);

/**
 * @brief Get the latest commits that changed a file.
 * @return Empty string on success, otherwise an error message.
 */
String GetFileHistory(const String& file, std::vector<Commit>& commits);

/**
 * @brief Save the version of a file in a git revision to destPath.
 * @return Empty string on success, otherwise an error message.
 */
String SaveFileVersion(const String& file, const std::string& rev, const String& destPath);

}
