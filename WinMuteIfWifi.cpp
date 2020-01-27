#include "WinMuteIfWifi.h"

#include <cwchar>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <objbase.h>
#include <windows.h>
#include <wlanapi.h>

// Needed to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

int wmain(int argc, wchar_t* argv[])
{
	// Has to be called in every thread that uses COM
	CoInitialize(nullptr);

	if (argv[1] == nullptr) {
		wprintf(L"Bad argument. Please specify the SSID of the Wifi as a argument.\n");
		return 160;
	}
	if (isConnectedToWifiSSID(argv[1])) {
		mute();
		return 0;
	}
	else {
		return 2;
	}
}

bool isConnectedToWifiSSID(wchar_t* ssid)
{
	// Declare and initialize variables.
	HANDLE hClient = nullptr;
	const DWORD dwMaxClient = 2;
	DWORD dwCurVersion = 0;
	DWORD dwResult;

	bool result = false;

	// variables used for WlanEnumInterfaces
	PWLAN_INTERFACE_INFO_LIST pIfList;

	// variables used for WlanQueryInterfaces for opcode = wlan_intf_opcode_current_connection
	PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = nullptr;
	DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
	WLAN_OPCODE_VALUE_TYPE opCode;

	dwResult = WlanOpenHandle(dwMaxClient, nullptr, &dwCurVersion, &hClient);
	if (FAILED(dwResult)) {
		wprintf(L"WlanOpenHandle failed with error: %u\n", dwResult);
	}

	dwResult = WlanEnumInterfaces(hClient, nullptr, &pIfList);
	if (FAILED(dwResult)) {
		wprintf(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
	}
	else {
		for (int i = 0; i < static_cast<int>(pIfList->dwNumberOfItems); i++) {
			auto pIfInfo = static_cast<WLAN_INTERFACE_INFO*>(&pIfList->InterfaceInfo[i]);

			// Call WlanQueryInterface to get current connection attributes
			if (pIfInfo->isState == wlan_interface_state_connected) {
				dwResult = WlanQueryInterface(hClient, &pIfInfo->InterfaceGuid,
					wlan_intf_opcode_current_connection,
					nullptr,
					&connectInfoSize,
					reinterpret_cast<PVOID*>(&pConnectInfo),
					&opCode);

				if (FAILED(dwResult)) {
					wprintf(L"WlanQueryInterface failed with error: %u\n", dwResult);
				}
				else {
					if (wcslen(ssid) == pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength) {
						result = true;
						for (unsigned int k = 0;
							k < pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength;
							k++) {

							if (pConnectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID[k] != ssid[k])
							{
								result = false;
								break;
							}
						}
					}
				}
			}
			else {
				wprintf(L"Not connected to a Wifi\n");
			}
		}
	}
	if (pConnectInfo != nullptr) {
		WlanFreeMemory(pConnectInfo);
		pConnectInfo = nullptr;
	}

	if (pIfList != nullptr) {
		WlanFreeMemory(pIfList);
		pIfList = nullptr;
	}

	return result;
}

void mute()
{
	IMMDeviceEnumerator* deviceEnumerator;
	IMMDevice* defaultDevice;
	IAudioEndpointVolume* endpointVolume;

	if (SUCCEEDED(CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), reinterpret_cast<LPVOID*>(&deviceEnumerator)))) {
		if (SUCCEEDED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice))) {
			if (SUCCEEDED(defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<LPVOID*>(&endpointVolume)))) {
				if (endpointVolume != nullptr) {
					BOOL isMuted = FALSE;
					endpointVolume->GetMute(&isMuted);
					if (!isMuted) {
						endpointVolume->SetMute(TRUE, nullptr);
					}
					endpointVolume->Release();
				}
				defaultDevice->Release();
			}
			deviceEnumerator->Release();
		}
	}
}