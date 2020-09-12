#pragma once

#include <vector>

#include "IODevice.h"

struct HIDPortCallback
{
	virtual void OnReadError(DWORD dwError) = 0;
};

class HIDPort
{
public:
	HIDPort();
	~HIDPort();

	static bool Find(USHORT VID, USHORT PID, CString &strDevicePath);

	bool Open(LPCTSTR pszDevicePath);
	bool Close();

	inline void SetCallback(HIDPortCallback* pCB) { m_pCB = pCB; }

	DWORD BytesCanRead() const;

	void Read(BYTE* pbBuf, DWORD dwBytesToRead);
	bool Write(BYTE* pbBuf, DWORD dwBytesToWrite);
	bool Flush();

private:
	static DWORD WINAPI PollingThread(void* pvParam);
	DWORD DoPoll();

	void BufferData(BYTE* pbData, DWORD dwDataLength);

	bool StartPolling();
	bool StopPolling();

	bool GetCaps();

private:
	enum 
	{ 
		RB_SIZE_MAX = 16384, 
		RB_SIZE_MOD = RB_SIZE_MAX - 1
	};

	IODevice m_IODevice;
	bool m_fOpened;
	HIDPortCallback* m_pCB;
	HIDP_CAPS m_caps;

	BYTE m_RXBuf[RB_SIZE_MAX]; // ���ڶ��Ļ�����
	DWORD m_RXPos;			// ���������д�����ݵ�λ��(ƫ��)
	DWORD m_readPos;		// �Ӷ���������ȡ���ݵ�λ��(ƫ��)

	std::vector<BYTE> m_TXBuf; // ����д�Ļ�����
	DWORD m_TXPos;	// ��д��������д�����ݵ�λ��(ƫ��)

	HANDLE m_hPolling;
	bool m_fStop;
};
