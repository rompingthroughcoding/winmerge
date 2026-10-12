// SPDX-License-Identifier: GPL-2.0-or-later
/**
 * @file  GitBar.cpp
 *
 * @brief Implementation of the dialog bar to start a git comparison and switch its commit.
 */

#include "StdAfx.h"
#include "GitBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BOOL CGitBar::Create(CWnd* pParentWnd)
{
	return CTrDialogBar::Create(pParentWnd, IDD, CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY, IDD);
}

void CGitBar::SetVersions(const std::vector<String>& labels, int selected)
{
	CComboBox* pCombo = static_cast<CComboBox*>(GetDlgItem(IDC_GITBAR_VERSION));
	pCombo->ResetContent();
	if (labels.empty())
	{
		pCombo->AddString(_("Not comparing with git. Click \"Compare with Git\" to start.").c_str());
		pCombo->SetCurSel(0);
		pCombo->EnableWindow(FALSE);
		return;
	}
	for (const String& label : labels)
		pCombo->AddString(label.c_str());
	pCombo->SetCurSel(selected);
	pCombo->EnableWindow(TRUE);
}

int CGitBar::GetSelectedVersion() const
{
	const CComboBox* pCombo = static_cast<const CComboBox*>(GetDlgItem(IDC_GITBAR_VERSION));
	return pCombo->IsWindowEnabled() ? pCombo->GetCurSel() : CB_ERR;
}
