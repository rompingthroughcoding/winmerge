// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file  GitBar.h
 *
 * @brief Declaration of the dialog bar to start a git comparison and switch its commit.
 */
#pragma once

#include <vector>
#include "TrDialogs.h"

class CGitBar : public CTrDialogBar
{
public:
	BOOL Create(CWnd* pParentWnd);

	/** @brief Fill the commit list; an empty list shows a hint and disables the list. */
	void SetVersions(const std::vector<String>& labels, int selected);
	int GetSelectedVersion() const;

	enum { IDD = IDD_GIT_BAR };
};
