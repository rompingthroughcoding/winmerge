// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file  GitCompareDlg.cpp
 *
 * @brief Implementation of the dialog to compare a file with a git version.
 */

#include "StdAfx.h"
#include "GitCompareDlg.h"
#include <algorithm>
#include "paths.h"
#include "FileOrFolderSelect.h"
#include "MergeApp.h"
#include "OptionsMgr.h"
#include "OptionsDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CGitCompareDlg, CTrDialog)
	ON_BN_CLICKED(IDC_GITCMP_BROWSE, OnBrowse)
	ON_BN_CLICKED(IDC_GITCMP_BASEBROWSE, OnBrowseBase)
	ON_CBN_KILLFOCUS(IDC_GITCMP_FILE, OnKillFocusFile)
	ON_CBN_SELCHANGE(IDC_GITCMP_FILE, OnSelChangeFile)
	ON_EN_KILLFOCUS(IDC_GITCMP_BASE, OnKillFocusBase)
END_MESSAGE_MAP()

CGitCompareDlg::CGitCompareDlg(CWnd* pParent /*= nullptr*/)
	: CTrDialog(IDD_GIT_COMPARE, pParent)
{
}

BOOL CGitCompareDlg::OnInitDialog()
{
	CTrDialog::OnInitDialog();
	SetDlgItemText(IDC_GITCMP_BASE, GetOptionsMgr()->GetString(OPT_GIT_BASE_FOLDER));
	LoadRecentFiles();
	CComboBox* pFileCombo = static_cast<CComboBox*>(GetDlgItem(IDC_GITCMP_FILE));
	if (m_strFile.empty() && pFileCombo->GetCount() > 0)
	{
		CString newest;
		pFileCombo->GetLBText(0, newest);
		m_strFile = newest.GetString();
	}
	SetDlgItemText(IDC_GITCMP_FILE, m_strFile);
	LoadVersions();
	return TRUE;
}

String CGitCompareDlg::GetFileText()
{
	String file;
	GetDlgItemText(IDC_GITCMP_FILE, file);
	return strutils::trim_ws(file);
}

String CGitCompareDlg::GetBaseText()
{
	String base;
	GetDlgItemText(IDC_GITCMP_BASE, base);
	return strutils::trim_ws(base);
}

/**
 * @brief Fill the file list with the most recent files below the base folder.
 */
void CGitCompareDlg::LoadRecentFiles()
{
	const String base = GetBaseText();
	if (base == m_strLoadedBase)
		return;
	m_strLoadedBase = base;

	const String file = GetFileText();
	CComboBox* pCombo = static_cast<CComboBox*>(GetDlgItem(IDC_GITCMP_FILE));
	pCombo->ResetContent();
	int widest = 0;
	if (!base.empty() && paths::DoesPathExist(base) == paths::IS_EXISTING_DIR)
	{
		CDC* pDC = pCombo->GetDC();
		CFont* pOldFont = pDC->SelectObject(pCombo->GetFont());
		for (const String& recent : git::GetRecentFiles(base, 30))
		{
			pCombo->AddString(recent.c_str());
			widest = (std::max)(widest, static_cast<int>(pDC->GetTextExtent(recent.c_str(), static_cast<int>(recent.length())).cx));
		}
		pDC->SelectObject(pOldFont);
		pCombo->ReleaseDC(pDC);
	}
	// Widen the dropped list so full paths are readable, capped at the screen width.
	const int maxWidth = GetSystemMetrics(SM_CXSCREEN) * 9 / 10;
	pCombo->SetDroppedWidth((std::min)(widest + GetSystemMetrics(SM_CXVSCROLL) + 8, maxWidth));
	SetDlgItemText(IDC_GITCMP_FILE, file);
}

/**
 * @brief Fill the version list with HEAD followed by the commits that changed the file.
 */
void CGitCompareDlg::LoadVersions()
{
	const String file = GetFileText();
	if (file == m_strLoadedFile && !file.empty())
		return;
	m_strLoadedFile = file;

	CComboBox* pCombo = static_cast<CComboBox*>(GetDlgItem(IDC_GITCMP_VERSION));
	pCombo->ResetContent();
	m_commits.clear();

	String error;
	if (file.empty())
		error = _("Select a file.");
	else if (paths::DoesPathExist(file) != paths::IS_EXISTING_FILE)
		error = _("The file does not exist.");
	else
		error = git::GetFileHistory(file, m_commits);

	if (error.empty())
	{
		pCombo->AddString(_("HEAD (latest commit)").c_str());
		for (const git::Commit& commit : m_commits)
			pCombo->AddString(commit.label.c_str());
		pCombo->SetCurSel(0);
	}
	SetDlgItemText(IDC_GITCMP_STATUS, error);
	EnableDlgItem(IDC_GITCMP_VERSION, error.empty());
	EnableDlgItem(IDOK, error.empty());
}

void CGitCompareDlg::OnBrowse()
{
	String path = GetFileText();
	String folder = path.empty() ? GetBaseText() : paths::GetParentPath(path);
	if (!SelectFile(m_hWnd, path, true, folder.empty() ? nullptr : folder.c_str()))
		return;
	SetDlgItemText(IDC_GITCMP_FILE, path);
	LoadVersions();
}

void CGitCompareDlg::OnBrowseBase()
{
	String path = GetBaseText();
	if (!SelectFolder(path, path.empty() ? nullptr : path.c_str(), _("Select the base folder"), m_hWnd))
		return;
	SetDlgItemText(IDC_GITCMP_BASE, path);
	LoadRecentFiles();
}

void CGitCompareDlg::OnKillFocusBase()
{
	LoadRecentFiles();
}

void CGitCompareDlg::OnKillFocusFile()
{
	LoadVersions();
}

void CGitCompareDlg::OnSelChangeFile()
{
	CComboBox* pCombo = static_cast<CComboBox*>(GetDlgItem(IDC_GITCMP_FILE));
	const int sel = pCombo->GetCurSel();
	if (sel == CB_ERR)
		return;
	CString selected;
	pCombo->GetLBText(sel, selected);
	SetDlgItemText(IDC_GITCMP_FILE, selected.GetString());
	LoadVersions();
}

void CGitCompareDlg::OnOK()
{
	LoadVersions();
	CComboBox* pCombo = static_cast<CComboBox*>(GetDlgItem(IDC_GITCMP_VERSION));
	const int sel = pCombo->GetCurSel();
	if (sel == CB_ERR)
		return;
	m_strFile = m_strLoadedFile;
	m_strRev = (sel == 0) ? "HEAD" : m_commits[sel - 1].hash;
	GetOptionsMgr()->SaveOption(OPT_GIT_BASE_FOLDER, GetBaseText());
	CTrDialog::OnOK();
}
