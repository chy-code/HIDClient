#pragma once


class IODevice
{
public:
	IODevice();
	~IODevice();

	bool Open(const TCHAR * pszDevicePath);
	void Close();

	bool Read(BYTE* pbBuf, DWORD dwBytesToRead, DWORD dwTimeOut);
	bool Write(BYTE* pbBuf, DWORD dwBytesToWrite, DWORD dwTimeOut);

	inline operator HANDLE() { return m_hFile; }

private:
	bool InitOverlappeds();

private:
	HANDLE m_hFile;
	OVERLAPPED m_oRead;
	OVERLAPPED m_oWrite;
	bool m_fOpened;
};

