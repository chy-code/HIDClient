#pragma once


// COutputBox

class COutputBox : public CRichEditCtrl
{
	DECLARE_DYNAMIC(COutputBox)

public:
	COutputBox();
	virtual ~COutputBox();

	virtual void PreSubclassWindow();

	void AppendText(LPCTSTR pszText);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnClear();
	afx_msg LRESULT OnAppendText(WPARAM wParam, LPARAM lParam);

private:
	void ShowContextMenu(CPoint &point);

private:
	CMenu m_contextMenu;
	CCriticalSection m_cs;
	DWORD m_dwMainThdID;
};


