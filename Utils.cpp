#include "pch.h"
#include "Utils.h"


void FormatWin32Error(DWORD dwError, CString& strOut)
{
	LPTSTR pszMessage = nullptr;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwError,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&pszMessage,
		0, NULL);

	if (pszMessage != nullptr)
	{
		strOut = pszMessage;
		LocalFree(pszMessage);
	}
}


void ReportLastWIN32Error()
{
	DWORD dwError = GetLastError();
	if (dwError == 0)
		return;

	CString strError;
	FormatWin32Error(dwError, strError);
	AfxMessageBox(strError, MB_OK | MB_ICONERROR);
}
