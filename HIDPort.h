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

	BYTE m_RXBuf[RB_SIZE_MAX]; // 用于读的缓冲区
	DWORD m_RXPos;			// 向读缓冲区写入数据的位置(偏移)
	DWORD m_readPos;		// 从读缓冲区读取数据的位置(偏移)

	std::vector<BYTE> m_TXBuf; // 用于写的缓冲区
	DWORD m_TXPos;	// 向写缓冲区中写入数据的位置(偏移)

	HANDLE m_hPolling;
	bool m_fStop;
};
