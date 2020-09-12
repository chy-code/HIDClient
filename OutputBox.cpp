// OutputBox.cpp: 实现文件
//

#include "pch.h"
#include "OutputBox.h"

#define ID_RICH_CLEAR  105

#define WM_APPENDTEXT	(WM_USER+1)


struct OBAPPENDTEXTINFO
{
	CString mText;

	OBAPPENDTEXTINFO(LPCTSTR pszText)
		: mText(pszText) { }
};



// COutputBox

IMPLEMENT_DYNAMIC(COutputBox, CRichEditCtrl)

COutputBox::COutputBox()
	: m_dwMainThdID(0)
{
}

COutputBox::~COutputBox()
{

}


void COutputBox::PreSubclassWindow()
{
	COLORREF crBk = GetSysColor(COLOR_INFOBK);
	SetBackgroundColor(FALSE, crBk);

	m_dwMainThdID = GetCurrentThreadId();

	CRichEditCtrl::PreSubclassWindow();
}


void COutputBox::AppendText(LPCTSTR pszText)
{
	//if (!m_cs.Lock())
	//	return;

	if (GetCurrentThreadId() != m_dwMainThdID)
	{
		OBAPPENDTEXTINFO* lpATI = new OBAPPENDTEXTINFO(pszText);
		while (!::PostMessage(GetSafeHwnd(),
			WM_APPENDTEXT, (WPARAM)lpATI, NULL));
	}
	else
	{
		SetSel(-1, -1);
		ReplaceSel(pszText);
		SendMessage(WM_VSCROLL, SB_BOTTOM, 0);
	}

	//m_cs.Unlock();
}


BEGIN_MESSAGE_MAP(COutputBox, CRichEditCtrl)
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_RICH_CLEAR, OnClear)
	ON_MESSAGE(WM_APPENDTEXT, OnAppendText)
END_MESSAGE_MAP()



// COutputBox 消息处理程序

void COutputBox::OnRButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
	ShowContextMenu(point);
}


LRESULT COutputBox::OnAppendText(WPARAM wParam, LPARAM lParam)
{
	OBAPPENDTEXTINFO* lpATI = (OBAPPENDTEXTINFO*)wParam;

	SetSel(-1, -1);
	ReplaceSel(lpATI->mText);

	delete lpATI;

	PostMessage(WM_VSCROLL, SB_BOTTOM, 0);

	return 0;
}


void COutputBox::OnClear()
{
	SetWindowText(NULL);
}


void COutputBox::ShowContextMenu(CPoint& point)
{
	if (m_contextMenu.GetSafeHmenu() == NULL)
	{
		if (!m_contextMenu.CreatePopupMenu())
			return;

		m_contextMenu.AppendMenu(0, ID_RICH_CLEAR, _T("清空"));
	}

	CMenu popupMenu;
	if (!popupMenu.Attach(m_contextMenu.GetSafeHmenu()))
		return;

	ClientToScreen(&point);
	popupMenu.TrackPopupMenu(TPM_RIGHTBUTTON, point.x, point.y, this);
	popupMenu.Detach();
}

