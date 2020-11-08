#include "WinMuteIfWifi.h"
#include "WifiHelper.h"
#include <iostream>

#include <cwchar>
#include <endpointvolume.h>
#include <mmdeviceapi.h>
#include <objbase.h>
#include <wlanapi.h>
#include <string>
#include <vector>

// Needed to link with Wlanapi.lib and Ole32.lib
#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "ole32.lib")

int main(int argc, char *argv[]) {
    // Has to be called in every thread that uses COM
    CoInitialize(nullptr);

    std::vector<std::string> allArgs(argv, argv + argc);
    std::unique_ptr<WifiHelper> wifiHelper(new WifiHelper);
    std::list<std::string> ssids = wifiHelper->getSsidNames();

    for (std::string givenSsid : allArgs) {
        if (std::find(ssids.begin(), ssids.end(), givenSsid) != ssids.end()) {
            WinMuteIfWifi::mute();
            return 0;
        }
    }
}

void WinMuteIfWifi::mute() {
    IMMDeviceEnumerator *deviceEnumerator;
    IMMDevice *defaultDevice;
    IAudioEndpointVolume *endpointVolume;

    if (SUCCEEDED(
            CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator),
                             reinterpret_cast<LPVOID *>(&deviceEnumerator)))) {
        if (SUCCEEDED(deviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &defaultDevice))) {
            if (SUCCEEDED(defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL,
                                                  reinterpret_cast<LPVOID *>(&endpointVolume)))) {
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
