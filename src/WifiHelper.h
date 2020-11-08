#ifndef WINMUTEIFWIF_WIFIHELPER_H
#define WINMUTEIFWIF_WIFIHELPER_H

#include <memory> // PImpl
#include <string>
#include <list>

class WifiHelper {
public:
    WifiHelper();

    std::list<std::string> getSsidNames();

    virtual ~WifiHelper();

private:
    // Internal implementation class
    class Impl;

    // Pointer to the internal implementation
    std::unique_ptr<Impl> pimpl;
};


#endif //WINMUTEIFWIF_WIFIHELPER_H
