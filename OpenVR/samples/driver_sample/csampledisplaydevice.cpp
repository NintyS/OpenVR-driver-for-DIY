//
// Created by nintys on 05.04.23.
//

#include "csampledisplaydevice.h"

CSampleDisplayDevice::CSampleDisplayDevice() {
    std::cout << "CSampleDisplayDevice::CSampleDisplayDevice()" << std::endl;
}

void CSampleDisplayDevice::Present(const vr::PresentInfo_t *pPresentInfo, uint32_t unPresentInfoSize) {
    std::cout << "CSampleDisplayDevice::Present()" << std::endl;
}