//
// Created by nintys on 05.04.23.
//

#ifndef CSAMPLEDISPLAYDEVICE_H
#define CSAMPLEDISPLAYDEVICE_H

#include <openvr_driver.h>
#include <GL/glut.h>
#include <iostream>

class CSampleDisplayDevice : public vr::ITrackedDeviceServerDriver, public vr::IVRVirtualDisplay {

public:
    CSampleDisplayDevice();

    virtual ~CSampleDisplayDevice();

    virtual vr::EVRInitError Activate(vr::TrackedDeviceIndex_t unObjectId);

    virtual void Deactivate();

    virtual  void EnterStandby();

    virtual void *GetComponent(const char *pchComponentNameAndVersion);

    virtual void DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize);

    virtual vr::DriverPose_t GetPose();

    virtual void Present( const vr::PresentInfo_t *pPresentInfo, uint32_t unPresentInfoSize ) override;

    virtual void WaitForPresent() override;

    virtual bool GetTimeSinceLastVsync( float *pfSecondsSinceLastVsync, uint64_t *pulFrameCounter ) override;


private:
    GLuint64 m_glTextureHandle;



};


#endif //CSAMPLEDISPLAYDEVICE_H
