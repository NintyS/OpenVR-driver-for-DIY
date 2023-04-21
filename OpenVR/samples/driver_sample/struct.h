//
// Created by nintys on 25.03.23.
//

#ifndef OPENVR_DRIVER_FOR_DIY_STRUCT_H
#define OPENVR_DRIVER_FOR_DIY_STRUCT_H

struct Point3D {

    Point3D(float x, float y, float z, float yaw, float pitch, float roll, float kata) : x(x), y(y), z(z), yaw(yaw), pitch(pitch), roll(roll), kata(kata) {};
    Point3D() : x(0), y(0), z(0), yaw(0), pitch(0), roll(0), kata(0) {};

    ~Point3D() {
        x = 0;
        y = 0;
        z = 0;

        yaw = 0;
        pitch = 0;
        roll = 0;
        kata = 0;
    };

    float x;
    float y;
    float z;

    float yaw;
    float pitch;
    float roll;
    float kata;
};

enum ButtonState {
    None,
    Touched,
    Pressed
};

struct Buttons {

    std::string ButtonPrimary;
    std::string ButtonSecondary;
    std::string ButtonSuper;
    std::string ButtonThumbrest;

    std::string ButtonGrip;
    std::string ButtonTrigger;
    std::string ButtonJoystick;

    float PressureTrigger = 0;
    float PressureGrip = 0;
    float JoyStickX = 0;
    float JoyStickY = 0;
};

#endif //OPENVR_DRIVER_FOR_DIY_STRUCT_H
