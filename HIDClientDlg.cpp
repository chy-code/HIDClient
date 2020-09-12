
// HIDClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "HIDClient.h"
#include "HIDClientDlg.h"
#include "Utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define KEY_VID	_T("VendorID")
#define KEY_PID	_T("ProductID")

#define TIMERID_READ	1


// CHIDClientDlg 对话框



CHIDClientDlg::CHIDClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HIDCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHIDClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_VID, m_strVID);
	DDV_MaxChars(pDX, m_strVID, 4);
	DDX_Text(pDX, IDC_PID, m_strPID);
	DDV_MaxChars(pDX, m_strPID, 4);

	DDX_Control(pDX, IDC_INPUT, m_wndInput);
	DDX_Control(pDX, IDC_OUTPUT, m_wndOutput);
}

BEGIN_MESSAGE_MAP(CHIDClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_OPEN, &CHIDClientDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_CLOSE, &CHIDClientDlg::OnBnClickedClose)
	ON_BN_CLICKED(IDC_TRANSMIT, &CHIDClientDlg::OnBnClickedTransmit)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CHIDClientDlg 消息处理程序

BOOL CHIDClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	m_strVID = AfxGetApp()->GetProfileString(AfxGetApp()->m_pszAppName, KEY_VID, _T("0483"));
	m_strPID = AfxGetApp()->GetProfileString(AfxGetApp()->m_pszAppName, KEY_PID, _T("5750"));
	m_port.SetCallback(this);

	UpdateData(FALSE);
	UpdateUIState(false);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CHIDClientDlg::OnDestroy()
{
	OnBnClickedClose();

	__super::OnDestroy();
}


// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHIDClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHIDClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHIDClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent != TIMERID_READ)
		return;

	CStringA strA;
	CString strT;
	std::vector<BYTE> buf;
	DWORD nbytes = m_port.BytesCanRead();

	if (nbytes > 0)
	{
		buf.resize(nbytes);
		m_port.Read(buf.data(), nbytes);

		strA.Append((char*)buf.data(), nbytes);
		strT = CA2T(strA);

		m_wndOutput.AppendText(strT);
	}
}


void CHIDClientDlg::OnBnClickedOpen()
{
	if (!UpdateData())
		return;

	if (m_strVID.IsEmpty())
	{
		AfxMessageBox(_T("未输入VID"), MB_OK | MB_ICONERROR);
		return;
	}

	if (m_strPID.IsEmpty())
	{
		AfxMessageBox(_T("未输入PID"), MB_OK | MB_ICONERROR);
		return;
	}

	USHORT VID = (USHORT)strtol(CT2A(m_strVID), NULL, 16);
	USHORT PID = (USHORT)strtol(CT2A(m_strPID), NULL, 16);

	CString strDevicePath;
	if (!HIDPort::Find(VID, PID, strDevicePath))
	{
		AfxMessageBox(_T("设备未找到"), MB_OK | MB_ICONERROR);
		return;
	}

	if (!m_port.Open(strDevicePath))
	{
		ReportLastWIN32Error();
		return;
	}

	SetTimer(TIMERID_READ, 10, NULL);
	UpdateUIState(true);

	AfxGetApp()->WriteProfileString(AfxGetApp()->m_pszAppName, KEY_VID, m_strVID);
	AfxGetApp()->WriteProfileString(AfxGetApp()->m_pszAppName, KEY_PID, m_strPID);
}


void CHIDClientDlg::OnBnClickedClose()
{
	if (!m_port.Close())
	{
		ReportLastWIN32Error();
		return;
	}

	KillTimer(TIMERID_READ);
	UpdateUIState(false);
}


void CHIDClientDlg::OnBnClickedTransmit()
{
	CString strT;
	CStringA strA;

	m_wndInput.GetWindowText(strT);
	strA = CT2A(strT);

	if (!m_port.Write((BYTE*)strA.GetBuffer(), strA.GetLength()))
	{
		ReportLastWIN32Error();
		return;
	}

	if (!m_port.Flush())
		ReportLastWIN32Error();
}


void CHIDClientDlg::OnReadError(DWORD dwError)
{
	CString strError;
	FormatWin32Error(dwError, strError);
	m_wndOutput.AppendText(strError);
}


void CHIDClientDlg::UpdateUIState(bool fOpened)
{
	GetDlgItem(IDC_VID)->EnableWindow(!fOpened);
	GetDlgItem(IDC_PID)->EnableWindow(!fOpened);
	GetDlgItem(IDC_OPEN)->EnableWindow(!fOpened);
	GetDlgItem(IDC_CLOSE)->EnableWindow(fOpened);
	GetDlgItem(IDC_TRANSMIT)->EnableWindow(fOpened);
}
