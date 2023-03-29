/** @file enums.as */

enum RenderTargetFlags
{
  kClear  = 0,
  kResize = 1
}

namespace Asset //! Namespace containing all asset related classes, enums and functions
{
    /**
    * @enum TextureFormat
    * @brief All of the different texture formats that are supported by the engine
    * @author Hilze Vonck 
    * @public
    **/
    enum TextureFormat
    {
        kUnknown, //!< Unknown format 0 bits per pixel
        kR8G8B8A8, //!< RGBA UNORM 32 bits per pixel
        kR16G16B16A16, //!< RGBA float 64 bits per pixel
        kR32G32B32A32, //!< RGBA float 128 bits per pixel
        kR16G16, //!< RG float 32 bits per pixel
        kR32G32, //!< RG float 64 bits per pixel
        kR32, //!< R float 32 bits per pixel
        kR16, //!< R float 16 bits per pixel
        kR24G8 //!< RG Depth Stencil 32 bits per pixel
    };
}

namespace Input //! Namespace containing all input classes, enums and functions
{
    /**
    * @enum Input::Buttons
    * @brief An enum containing all buttons that can be used for input. This enum contains both mouse and controller buttons
    * @author Hilze Vonck
    **/
    enum Buttons
    {
        kLMB = 0, //!< Left Mouse Button (LMB)
        kMMB = 1, //!< Middle Mouse Button (MMB)
        kRMB = 2, //!< Right Mouse Button (RMB)
        kA = 3, //!< Controller A
        kB = 4, //!< Controller B
        kX = 5, //!< Controller X
        kY = 6, //!<  Controller Y
        kUp = 7, //!< Controller DPad Up
        kDown = 8, //!< Controller DPad Down
        kLeft = 9, //!< Controller DPad Left
        kRight = 10, //!< Controller DPad Right
        kBumperL = 11, //!< Controller Left Bumper
        kBumperR = 12, //!< Controller Right Bumper
        kStickL = 13, //!< Controller Left Stick
        kStickR = 14, //!< Controller Right Stick
        kStart = 15, //!< Controller Start
        kBack = 16, //!< Controller Select
    };
    /**
    * @enum Input::Axes
    * @brief An enum containing all axes that can be used for input. This enum contains both mouse and controller axes
    * @author Hilze Vonck
    **/
    enum Axes
    {
        kMouseX = 0, //!< Mouse X Axis
        kMouseY = 1, //!< Mouse Y Axis
        kScroll = 2, //!< Mouse Scroll Wheel
        kStickLX = 3, //!< Controller Left Stick X Axis
        kStickLY = 4, //!< Controller Left Stick Y Axis
        kStickRX = 5, //!< Controller Right Stick X Axis
        kStickRY = 6, //!< Controller Right Stick Y Axis
        kTriggerL = 7, //!< Controller Left Trigger
        kTriggerR = 8 //!< Controller Right Trigger
    };
    /**
    * @enum Input::Keys
    * @brief An enum containing all keys that can be used for input. This enum only contains keyboard keys
    * @author Hilze Vonck
    **/
    enum Keys
    {
        kA = 65, //!< Keyboard A
        kB = 66, //!< Keyboard B
        kC = 67, //!< Keyboard C
        kD = 68, //!< Keyboard D
        kE = 69, //!< Keyboard E
        kF = 70, //!< Keyboard F
        kG = 71, //!< Keyboard G
        kH = 72, //!< Keyboard H
        kI = 73, //!< Keyboard I
        kJ = 74, //!< Keyboard J
        kK = 75, //!< Keyboard K
        kL = 76, //!< Keyboard L
        kM = 77, //!< Keyboard M
        kN = 78, //!< Keyboard N
        kO = 79, //!< Keyboard O
        kP = 80, //!< Keyboard P
        kQ = 81, //!< Keyboard Q
        kR = 82, //!< Keyboard R
        kS = 83, //!< Keyboard S
        kT = 84, //!< Keyboard T
        kU = 85, //!< Keyboard U
        kV = 86, //!< Keyboard V
        kW = 87, //!< Keyboard W
        kX = 88, //!< Keyboard X
        kY = 89, //!< Keyboard Y
        kZ = 90, //!< Keyboard Z

        kTilde = 126, //!< Keyboard Tilde

        k0 = 48, //!< Keyboard Number Row 0
        k1 = 49, //!< Keyboard Number Row 1
        k2 = 50, //!< Keyboard Number Row 2
        k3 = 51, //!< Keyboard Number Row 3
        k4 = 52, //!< Keyboard Number Row 4
        k5 = 53, //!< Keyboard Number Row 5
        k6 = 54, //!< Keyboard Number Row 6
        k7 = 55, //!< Keyboard Number Row 7
        k8 = 56, //!< Keyboard Number Row 8
        k9 = 57, //!< Keyboard Number Row 9

        kNum0 = 0x60, //!< Keyboard Number Pad 0 
        kNum1 = 0x61, //!< Keyboard Number Pad 1
        kNum2 = 0x62, //!< Keyboard Number Pad 2
        kNum3 = 0x63, //!< Keyboard Number Pad 3
        kNum4 = 0x64, //!< Keyboard Number Pad 4
        kNum5 = 0x65, //!< Keyboard Number Pad 5
        kNum6 = 0x66, //!< Keyboard Number Pad 6
        kNum7 = 0x67, //!< Keyboard Number Pad 7
        kNum8 = 0x68, //!< Keyboard Number Pad 8
        kNum9 = 0x69, //!< Keyboard Number Pad 9

        kTab = 0x09, //!< Keyboard Tab
        kBackspace = 0x08, //!< Keyboard Backspace
        kReturn = 0x0D, //!< Keyboard Return (Enter)
        kEnter = kReturn, //!< Keyboard Enter (Return)
        kSpace = 0x20, //!< Keyboard Space
        kShift = 0x10, //!< Keyboard Shift (Left & Right)
        kLShift = 0xA0, //!< Keyboard Left Shift (not working)
        kRShift = 0xA1, //!< Keyboard Right Shift (not working)
        kControl = 0x11, //!< Keyboard Control (Left & Right)
        kLControl = 0xA2, //!< Keyboard Left Control (not working)
        kRControl = 0xA3, //!< Keyboard Right Control (not working)

        kLeft = 0x25, //!< Keyboard Arrow Keys Left
        kUp = 0x26, //!< Keyboard Arrow Keys Up
        kRight = 0x27, //!< Keyboard Arrow Keys Right
        kDown = 0x28, //!< Keyboard Arrow Keys Down
    };
}

/**
* @enum ShadowType
* @brief All of the different ways a light can cast shadows
* @author Hilze Vonck
* @public
**/
enum ShadowType
{
  kNone, //!< The light does not cast shadows
  kDynamic, //!< The light casts shadows every frame
  kGenerateOnce, //!< The light casts shadows only once
  kGenerated, //!< The light has already casted shadows
};

enum WaveSourceState
{
    kInitial,
    kPlaying,
    kPaused,
    kStopped
};

enum RigidBodyConstraints
{
    kNone = 0,
    kX = 1 << 0,
    kY = 1 << 1,
    kZ = 1 << 2,
};