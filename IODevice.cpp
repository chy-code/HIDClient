#include "pch.h"
#include "IODevice.h"


IODevice::IODevice()
	: m_fOpened(false)
{
}


IODevice::~IODevice()
{
	Close();
}


bool IODevice::Open(const TCHAR* pszDevicePath)
{
	if (m_fOpened)
		Close();

	m_hFile = CreateFile(pszDevicePath,
		GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (m_hFile == INVALID_HANDLE_VALUE)
		return false;

	if (!InitOverlappeds())
	{
		CloseHandle(m_hFile);
		return false;
	}

	m_fOpened = true;

	return true;
}


void IODevice::Close()
{
	if (m_fOpened)
	{
		CloseHandle(m_oRead.hEvent);
		CloseHandle(m_oWrite.hEvent);
		CloseHandle(m_hFile);

		m_fOpened = false;
	}
}


bool IODevice::Read(BYTE* pbBuf, DWORD dwBytesToRead, DWORD dwTimeOut)
{
	DWORD dwBytesRead, dwError;
	ULONGLONG t0;

	if (!ReadFile(m_hFile, pbBuf, dwBytesToRead, &dwBytesRead, &m_oRead))
	{
		dwError = GetLastError();
		if (dwError != ERROR_IO_PENDING)
			return false;

		t0 = GetTickCount64();

		while (!GetOverlappedResult(m_hFile, &m_oRead, &dwBytesRead, FALSE))
		{
			dwError = GetLastError();
			if (dwError != ERROR_IO_INCOMPLETE)
				return false;

			if (GetTickCount64() - t0 > dwTimeOut)
			{
				SetLastError(ERROR_TIMEOUT);
				return false;
			}

			Sleep(10);
		}
	}

	return true;
}


bool IODevice::Write(BYTE* pbBuf, DWORD dwBytesToWrite, DWORD dwTimeOut)
{
	DWORD dwBytesWritten, dwError;
	uint64_t t0;

	if (!WriteFile(m_hFile, pbBuf, dwBytesToWrite, &dwBytesWritten, &m_oWrite))
	{
		dwError = GetLastError();
		if (dwError != ERROR_IO_PENDING)
			return false;

		t0 = GetTickCount64();

		while (!GetOverlappedResult(m_hFile, &m_oWrite, &dwBytesWritten, FALSE))
		{
			dwError = GetLastError();
			if (dwError != ERROR_IO_INCOMPLETE)
				return false;

			if (GetTickCount64() - t0 > dwTimeOut)
			{
				SetLastError(ERROR_TIMEOUT);
				return false;
			}

			Sleep(1);
		}
	}

	return true;
}


bool IODevice::InitOverlappeds()
{
	ZeroMemory(&m_oRead, sizeof(m_oRead));
	ZeroMemory(&m_oWrite, sizeof(m_oWrite));

	m_oRead.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_oRead.hEvent == NULL)
		return false;

	m_oWrite.hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_oWrite.hEvent == NULL)
	{
		CloseHandle(m_oRead.hEvent);
		return false;
	}

	return true;
}
