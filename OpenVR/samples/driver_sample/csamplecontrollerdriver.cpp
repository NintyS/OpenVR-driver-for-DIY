#include "csamplecontrollerdriver.h"
#include "basics.h"

#include <math.h>

using namespace vr;

CSampleControllerDriver::CSampleControllerDriver()
{
    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
    m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
}

void CSampleControllerDriver::SetControllerIndex(int32_t CtrlIndex)
{
    ControllerIndex = CtrlIndex;
}

CSampleControllerDriver::~CSampleControllerDriver()
{
    delete m_pPositionUpdaterThread;
    delete m_pButtonsUpdaterThread;
}

vr::EVRInitError CSampleControllerDriver::Activate(vr::TrackedDeviceIndex_t unObjectId)
{
    m_unObjectId = unObjectId;
    m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ControllerType_String, "oculus_touch");
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_LegacyInputProfile_String, "vive_controller");

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ManufacturerName_String, "Oculus");

    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_TrackingSystemName_String, "oculus");
    vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_DeviceClass_Int32, TrackedDeviceClass_Controller);

    switch (ControllerIndex) {
    case 1:
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "CTRL1Serial");
            vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Quest2 (Left Controller)");
            vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "oculus_quest2_controller_left");
            break;
    case 2:
        vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "CTRL2Serial");
            vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_ModelNumber_String, "Oculus Quest2 (Right Controller)");
            vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, vr::Prop_RenderModelName_String, "oculus_quest2_controller_right");
            break;
    }

    uint64_t supportedButtons = 0xFFFFFFFFFFFFFFFFULL;
    vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, vr::Prop_SupportedButtons_Uint64, supportedButtons);

    // return a constant that's not 0 (invalid) or 1 (reserved for Oculus)
    vr::VRProperties()->SetUint64Property( m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 1 );

    // avoid "not fullscreen" warnings from vrmonitor
    //vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_IsOnDesktop_Bool, false );

    // our sample device isn't actually tracked, so set this property to avoid having the icon blink in the status window
    //vr::VRProperties()->SetBoolProperty( m_ulPropertyContainer, Prop_NeverTracked_Bool, false );

    // even though we won't ever track we want to pretend to be the right hand so binding will work as expected

    switch (ControllerIndex) {
    case 1:
        vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_LeftHand);
        break;
    case 2:
        vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, TrackedControllerRole_RightHand);
        break;
    }

    // this file tells the UI what to show the user for binding this controller as well as what default bindings should
    // be for legacy or other apps
    vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_InputProfilePath_String, "{sample}/resources/inputs/touch_profile.json");

    //  Buttons handles
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/a/click", &HButtons[0]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/b/click", &HButtons[1]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/x/click", &HButtons[2]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/y/click", &HButtons[3]);

    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/click", &HButtons[4]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &HButtons[5]);
    vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/joystick/click", &HButtons[6]);

    // Analog handles
    vr::VRDriverInput()->CreateScalarComponent(
                m_ulPropertyContainer, "/input/joystick/x", &HAnalog[0],
            vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedTwoSided
            );
    vr::VRDriverInput()->CreateScalarComponent(
                m_ulPropertyContainer, "/input/joystick/y", &HAnalog[1],
            vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedTwoSided
            );
    vr::VRDriverInput()->CreateScalarComponent(
                m_ulPropertyContainer, "/input/trigger/value", &HAnalog[2],
            vr::EVRScalarType::VRScalarType_Absolute, vr::EVRScalarUnits::VRScalarUnits_NormalizedOneSided
            );

    vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, vr::Prop_Axis0Type_Int32, vr::k_eControllerAxis_TrackPad);

    // create our haptic component
    vr::VRDriverInput()->CreateHapticComponent(m_ulPropertyContainer, "/output/haptic", &m_compHaptic);

    m_pPositionUpdaterThread = new std::thread(&CSampleControllerDriver::PositionUpdater, this);
    m_pButtonsUpdaterThread = new std::thread(&CSampleControllerDriver::ButtonsUpdater, this);

    m_pButtonsUpdaterThread->detach();
    m_pPositionUpdaterThread->detach();

    m_bShowController = true;

    return VRInitError_None;
}

void CSampleControllerDriver::Deactivate()
{
    if(m_bShowController.exchange(false)) {
        m_pPositionUpdaterThread->join();
        m_pButtonsUpdaterThread->join();
    }

    m_unObjectId = vr::k_unTrackedDeviceIndexInvalid;
}

void CSampleControllerDriver::EnterStandby()
{
}

void *CSampleControllerDriver::GetComponent(const char *pchComponentNameAndVersion)
{
//    if (!_stricmp(pchComponentNameAndVersion, vr::IVRDisplayComponent_Version)) {
//        return (vr::IVRDisplayComponent *)this;
//    }

    return nullptr;
}

void CSampleControllerDriver::PowerOff()
{

}

void CSampleControllerDriver::DebugRequest(const char *pchRequest, char *pchResponseBuffer, uint32_t unResponseBufferSize)
{
    if (unResponseBufferSize >= 1) {
        pchResponseBuffer[0] = 0;
    }
}

DriverPose_t CSampleControllerDriver::GetPose() {
    DriverPose_t pose = {0};
    //pose.poseIsValid = false;
    pose.poseIsValid = true;
    //pose.result = TrackingResult_Calibrating_OutOfRange;
    pose.result = TrackingResult_Running_OK;
    pose.deviceIsConnected = true;

    pose.qWorldFromDriverRotation = HmdQuaternion_Init(1, 0, 0, 0);
    pose.qDriverFromHeadRotation = HmdQuaternion_Init(1, 0, 0, 0);

    pose.vecPosition[0] = HMDPosition.x + ControllerPosition.x;
    pose.vecPosition[1] = (HMDPosition.y + ControllerPosition.y) - 1.5;
    pose.vecPosition[2] = HMDPosition.z + -ControllerPosition.z;

    pose.qRotation.w = ControllerPosition.kata;
    pose.qRotation.x = -ControllerPosition.yaw;
    pose.qRotation.y = -ControllerPosition.pitch;
    pose.qRotation.z = ControllerPosition.roll;

    return pose;
}

void CSampleControllerDriver::PositionUpdater() {
    while (m_bShowController) {
        if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
            vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(), sizeof(DriverPose_t));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

void CSampleControllerDriver::RunFrame()
{
//    std::cout << "\033[31m RunFrame \033[0m" << std::endl;
}

void CSampleControllerDriver::ProcessEvent(const vr::VREvent_t &vrEvent)
{
    switch (vrEvent.eventType) {
    case vr::VREvent_Input_HapticVibration:
        if (vrEvent.data.hapticVibration.componentHandle == m_compHaptic) {
            // This is where you would send a signal to your hardware to trigger actual haptic feedback
            //DriverLog( "BUZZ!\n" );
        }
        break;
    }
}

std::string CSampleControllerDriver::GetSerialNumber() const
{
    switch (ControllerIndex) {
        case 1:
            return "CTRL1Serial";
            break;
        case 2:
            return "CTRL2Serial";
            break;
        default:
            return "Don't exist";
            break;
    }
}

void CSampleControllerDriver::SetPositions(Point3D &controller, Point3D &hmd) {
    ControllerPosition = controller;
    HMDPosition = hmd;
}

void CSampleControllerDriver::SetButtons(Buttons &buttons) {
    ControllerButtons = buttons;
}

void CSampleControllerDriver::ButtonsUpdater() {
    while (m_bShowController) {

        bool a = false;
        bool b = false;
        bool x = false;
        bool y = false;

        bool grip = false;
        bool system = false;
        bool joystick = false;

        if(ControllerButtons.ButtonPrimary == "Pressed") {
            a = true;
        }

        if(ControllerButtons.ButtonSecondary == "Pressed") {
            b = true;
        }

//        if(ControllerButtons.sys == "Pressed") {
//            grip = true;
//        }

        if(ControllerButtons.ButtonGrip == "Pressed") {
            grip = true;
        }

        if(ControllerButtons.ButtonJoystick == "Pressed") {
            joystick = true;
        }

        if(ControllerIndex == 1) {
            vr::VRDriverInput()->UpdateBooleanComponent(HButtons[0], a, 0); //A
            vr::VRDriverInput()->UpdateBooleanComponent(HButtons[1], b, 0); //B
        } else {
            vr::VRDriverInput()->UpdateBooleanComponent(HButtons[2], x, 0); //X
            vr::VRDriverInput()->UpdateBooleanComponent(HButtons[3], y, 0); //Y
        }
        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[4], grip, 0); //Grip
//        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[5], grip, 0); //System

        vr::VRDriverInput()->UpdateBooleanComponent(HButtons[6], joystick, 0); //Joystick

        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[0], ControllerButtons.JoyStickX, 0); //Trackpad x
        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[1], ControllerButtons.JoyStickY, 0); //Trackpad y

        vr::VRDriverInput()->UpdateScalarComponent(HAnalog[2], ControllerButtons.PressureTrigger, 0);

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

