#include "pch.h"
#include "HIDPort.h"

//#include <algorithm>
#include <SetupAPI.h>

HIDPort::HIDPort()
	: m_fOpened(false)
{

}


HIDPort::~HIDPort()
{
	Close();
}


bool HIDPort::Find(USHORT VID, USHORT PID, CString& strDevicePath)
{
	GUID guid;
	HDEVINFO hDevInfo;

	HidD_GetHidGuid(&guid);

	hDevInfo = SetupDiGetClassDevs(&guid,
		NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hDevInfo == INVALID_HANDLE_VALUE)
		return false;

	SP_DEVICE_INTERFACE_DATA InterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA pDetailData;
	SP_DEVINFO_DATA DevData;
	HANDLE FileHandle;
	HIDD_ATTRIBUTES HidAttributes;
	DWORD RequiredSize;
	DWORD MemberIndex = 0;
	bool found = false;

	InterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	pDetailData = NULL;

	while (!found)
	{
		if (!SetupDiEnumDeviceInterfaces(hDevInfo,
			NULL, &guid, MemberIndex++, &InterfaceData))
		{
			if (GetLastError() == ERROR_NO_MORE_ITEMS)
				break;

			continue;
		}

		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo,
			&InterfaceData, NULL, 0, &RequiredSize, NULL))
		{
			DWORD dwError = GetLastError();
			if (dwError != ERROR_INSUFFICIENT_BUFFER)
			{
				break;
			}
		}

		pDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(RequiredSize);
		pDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		DevData.cbSize = sizeof(SP_DEVINFO_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hDevInfo,
			&InterfaceData, pDetailData, RequiredSize, &RequiredSize, &DevData))
		{
			break;
		}

		FileHandle = CreateFile(pDetailData->DevicePath,
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
			OPEN_EXISTING, 0, NULL);
		if (FileHandle != INVALID_HANDLE_VALUE)
		{
			HidAttributes.Size = sizeof(HIDD_ATTRIBUTES);
			if (HidD_GetAttributes(FileHandle, &HidAttributes))
			{
				if (HidAttributes.VendorID == VID
					&& HidAttributes.ProductID == PID)
				{
					found = true;
					strDevicePath = pDetailData->DevicePath;;
				}
			}

			CloseHandle(FileHandle);
		}

		free(pDetailData);
		pDetailData = NULL;
	}

	SetupDiDestroyDeviceInfoList(hDevInfo);

	return found;
}


bool HIDPort::Open(LPCTSTR pszDevicePath)
{
	if (!m_IODevice.Open(pszDevicePath))
		return false;

	if (!GetCaps())
		return false;

	m_RXPos = m_readPos = 0;
	m_TXBuf.resize(m_caps.OutputReportByteLength);
	m_TXPos = 1;

	if (!StartPolling())
		return false;

	m_fOpened = true;

	return true;
}


bool HIDPort::Close()
{
	if (m_fOpened)
	{
		if (!StopPolling())
			return false;

		m_IODevice.Close();
		m_fOpened = false;
	}

	return true;
}


DWORD HIDPort::BytesCanRead() const
{
	return (m_RXPos - m_readPos) & RB_SIZE_MOD;
}


void HIDPort::Read(BYTE* pbBuf, DWORD dwBytesToRead)
{
	for (DWORD i = 0; i < dwBytesToRead; i++)
	{
		pbBuf[i] = m_RXBuf[m_readPos++];
		m_readPos &= RB_SIZE_MOD;
	}
}


bool HIDPort::Write(BYTE* pbBuf, DWORD dwBytesToWrite)
{
	DWORD n;

	while (dwBytesToWrite > 0)
	{
		n = m_caps.OutputReportByteLength - m_TXPos;
		if (dwBytesToWrite < n)
			n = dwBytesToWrite;

		if (n > 0)
		{
			memcpy(&m_TXBuf[m_TXPos], pbBuf, n);
			m_TXPos += n;

			pbBuf += n;
			dwBytesToWrite -= n;
		}

		if (m_TXPos >= m_caps.OutputReportByteLength)
		{
			if (!m_IODevice.Write(m_TXBuf.data(), m_caps.OutputReportByteLength, 100))
				return false;

			m_TXPos = 1;
		}
	}

	return true;
}


bool HIDPort::Flush()
{
	DWORD n;

	if (m_TXPos > 1)
	{
		n = m_caps.OutputReportByteLength - m_TXPos;
		if (n > 0)
			memset(&m_TXBuf[m_TXPos], 0, n);

		if (!m_IODevice.Write(m_TXBuf.data(), m_caps.OutputReportByteLength, 1000))
			return false;

		m_TXPos = 1;
	}

	return true;
}


DWORD WINAPI HIDPort::PollingThread(void* pvParam)
{
	HIDPort* port = (HIDPort*)pvParam;
	return port->DoPoll();
}


DWORD HIDPort::DoPoll()
{
	std::vector<BYTE> report;
	report.resize(m_caps.InputReportByteLength);

	DWORD dwError;

	while (!m_fStop)
	{
		if (!m_IODevice.Read(report.data(), m_caps.InputReportByteLength, 500))
		{
			dwError = GetLastError();
			if (dwError != ERROR_TIMEOUT)
			{
				if (m_pCB != nullptr)
					m_pCB->OnReadError(dwError);
				break;
			}

			continue;
		}

		BufferData(&report[1], m_caps.InputReportByteLength - 1);
	}

	return 0;
}


void HIDPort::BufferData(BYTE* pbData, DWORD dwDataLength)
{
	DWORD n;

	while (dwDataLength > 0)
	{
		n = RB_SIZE_MAX - m_RXPos;
		if (dwDataLength < n)
			n = dwDataLength;

		memcpy(&m_RXBuf[m_RXPos], pbData, n);
		m_RXPos += n;
		m_RXPos &= RB_SIZE_MOD;

		pbData += n;
		dwDataLength -= n;
	}
}


bool HIDPort::StartPolling()
{
	m_fStop = false;
	m_hPolling = CreateThread(NULL, 0, PollingThread, this, 0, NULL);
	if (m_hPolling == NULL)
		return false;

	return true;
}


bool HIDPort::StopPolling()
{
	DWORD dwWait;
	m_fStop = true;
	dwWait = WaitForSingleObject(m_hPolling, INFINITE);
	if (dwWait != WAIT_OBJECT_0)
		return false;

	return true;
}


bool HIDPort::GetCaps()
{
	PHIDP_PREPARSED_DATA  pPreparsedData;

	if (!HidD_GetPreparsedData(m_IODevice, &pPreparsedData))
		return false;

	if (HidP_GetCaps(pPreparsedData, &m_caps) != HIDP_STATUS_SUCCESS)
	{
		HidD_FreePreparsedData(pPreparsedData);
		return false;
	}

	HidD_FreePreparsedData(pPreparsedData);

	return true;
}

