
// HIDClientDlg.h: 头文件
//

#pragma once


#include "OutputBox.h"


// CHIDClientDlg 对话框
class CHIDClientDlg : public CDialogEx, HIDPortCallback
{
// 构造
public:
	CHIDClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HIDCLIENT_DIALOG };
#endif

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	void OnReadError(DWORD dwError) override;

// 实现
protected:
	HICON m_hIcon;
	CString m_strVID;
	CString m_strPID;
	CEdit m_wndInput;
	COutputBox m_wndOutput;

	HIDPort m_port;

	// 生成的消息映射函数
	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedClose();
	afx_msg void OnBnClickedTransmit();

	void UpdateUIState(bool fOpened);
};
