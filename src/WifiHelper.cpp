#include "WifiHelper.h"

#include <objbase.h>
#include <wlanapi.h>
#include <list>
#include <stdexcept>

// Needed to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

struct WifiHelper::Impl {
    HANDLE hClient;
    PWLAN_INTERFACE_INFO_LIST pIfList;
};

WifiHelper::WifiHelper() : pimpl(new Impl()) {
    const DWORD dwMaxClient = 2;
    DWORD dwCurVersion = 0;

    DWORD dwResult = WlanOpenHandle(dwMaxClient, nullptr, &dwCurVersion, &pimpl->hClient);
    if (FAILED(dwResult)) {
        throw std::runtime_error("WlanOpenHandle failed with error: " + dwResult);
    } else {
        dwResult = WlanEnumInterfaces(pimpl->hClient, nullptr, &pimpl->pIfList);
        if (FAILED(dwResult)) {
            throw std::runtime_error("WlanEnumInterfaces failed with error: " + dwResult);
        }
    }
}

WifiHelper::~WifiHelper() {
    WlanFreeMemory(pimpl->pIfList);
}

std::list<std::string> WifiHelper::getSsidNames() {
    std::list<std::string> result;

    PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = nullptr;
    DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
    WLAN_OPCODE_VALUE_TYPE opCode;

    for (int i = 0; i < static_cast<int>(pimpl->pIfList->dwNumberOfItems); i++) {
        WLAN_INTERFACE_INFO *pIfInfo = static_cast<WLAN_INTERFACE_INFO *>(&pimpl->pIfList->InterfaceInfo[i]);

        // Call WlanQueryInterface to get current connection attributes
        if (pIfInfo->isState == wlan_interface_state_connected) {
            WlanQueryInterface(pimpl->hClient, &pIfInfo->InterfaceGuid,
                               wlan_intf_opcode_current_connection,
                               nullptr,
                               &connectInfoSize,
                               reinterpret_cast<PVOID *>(&pConnectInfo),
                               &opCode);

            result.push_back(
                    std::string(reinterpret_cast<char *>(pConnectInfo->wlanAssociationAttributes.dot11Ssid.ucSSID),
                                pConnectInfo->wlanAssociationAttributes.dot11Ssid.uSSIDLength));
        }
    }

    if (pConnectInfo != nullptr) {
        WlanFreeMemory(pConnectInfo);
        pConnectInfo = nullptr;
    }

    return result;
}
