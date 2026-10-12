// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file  GitCompareDlg.h
 *
 * @brief Declaration of the dialog to compare a file with a git version.
 */
#pragma once

#include <string>
#include <vector>
#include "TrDialogs.h"
#include "GitHelper.h"

class CGitCompareDlg : public CTrDialog
{
public:
	explicit CGitCompareDlg(CWnd* pParent = nullptr);

	String m_strFile;     /**< File to compare. */
	std::string m_strRev; /**< Selected git revision. */

protected:
	virtual BOOL OnInitDialog() override;
	virtual void OnOK() override;

	afx_msg void OnBrowse();
	afx_msg void OnBrowseBase();
	afx_msg void OnKillFocusFile();
	afx_msg void OnSelChangeFile();
	afx_msg void OnKillFocusBase();
	DECLARE_MESSAGE_MAP()

private:
	void LoadVersions();
	void LoadRecentFiles();
	String GetFileText();
	String GetBaseText();

	std::vector<git::Commit> m_commits;
	String m_strLoadedFile;
	String m_strLoadedBase;
};
