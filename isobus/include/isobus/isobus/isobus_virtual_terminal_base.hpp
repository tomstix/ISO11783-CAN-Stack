//================================================================================================
/// @file isobus_virtual_terminal_objects.hpp
///
/// @brief A base class for virtual terminal client and server
/// @author Tom Stirnkorb
///
/// @copyright 2023 Adrian Del Grosso
//================================================================================================

#ifndef ISOBUS_VIRTUAL_TERMINAL_BASE_HPP
#define ISOBUS_VIRTUAL_TERMINAL_BASE_HPP

#include <string>
#include <vector>
#include <memory>

namespace isobus
{
    class VirtualTerminalBase
    {
    public:
        /// @brief Typedef for 16bit object IDs
        typedef std::uint16_t ObjectID;

        /// @brief The types of objects in an object pool by object type byte value
        enum class ObjectType : std::uint8_t
        {
            WorkingSet = 0,                       ///< Top level object that describes an implement’s ECU or group of ECUs
            DataMask = 1,                         ///< Top level object that contains other objects. A Data Mask is activated by a Working Set to become the active set of objects on the VT display.
            AlarmMask = 2,                        ///< Top level object that contains other objects. Describes an alarm display.
            Container = 3,                        ///< Used to group objects.
            WindowMask = 34,                      ///< Top level object that contains other objects. The Window Mask is activated by the VT.
            SoftKeyMask = 4,                      ///< Top level object that contains Key objects.
            Key = 5,                              ///< Used to describe a Soft Key.
            Button = 6,                           ///< Used to describe a Button control.
            KeyGroup = 35,                        ///< Top level object that contains Key objects.
            InputBoolean = 7,                     ///< Used to input a TRUE/FALSE type input.
            InputString = 8,                      ///< Used to input a character string
            InputNumber = 9,                      ///< Used to input an integer or float numeric.
            InputList = 10,                       ///< Used to select an item from a pre-defined list.
            OutputString = 11,                    ///< Used to output a character string.
            OutputNumber = 12,                    ///< Used to output an integer or float numeric.
            OutputList = 37,                      ///< Used to output a list item.
            OutputLine = 13,                      ///< Used to output a line.
            OutputRectangle = 14,                 ///< Used to output a rectangle or square.
            OutputEllipse = 15,                   ///< Used to output an ellipse or circle.
            OutputPolygon = 16,                   ///< Used to output a polygon.
            OutputMeter = 17,                     ///< Used to output a meter.
            OutputLinearBarGraph = 18,            ///< Used to output a linear bar graph.
            OutputArchedBarGraph = 19,            ///< Used to output an arched bar graph.
            GraphicsContext = 36,                 ///< Used to output a graphics context.
            Animation = 44,                       ///< The Animation object is used to display simple animations
            PictureGraphic = 20,                  ///< Used to output a picture graphic (bitmap).
            NumberVariable = 21,                  ///< Used to store a 32-bit unsigned integer value.
            StringVariable = 22,                  ///< Used to store a fixed length string value.
            FontAttributes = 23,                  ///< Used to group font based attributes. Can only be referenced by other objects.
            LineAttributes = 24,                  ///< Used to group line based attributes. Can only be referenced by other objects.
            FillAttributes = 25,                  ///< Used to group fill based attributes. Can only be referenced by other objects
            InputAttributes = 26,                 ///< Used to specify a list of valid characters. Can only be referenced by input field objects.
            ExtendedInputAttributes = 38,         ///< Used to specify a list of valid WideChars. Can only be referenced by Input Field Objects.
            ColourMap = 39,                       ///< Used to specify a colour table object.
            ObjectLabelRefrence = 40,             ///< Used to specify an object label.
            ObjectPointer = 27,                   ///< Used to reference another object.
            ExternalObjectDefinition = 41,        ///< Used to list the objects that may be referenced from another Working Set
            ExternalReferenceNAME = 42,           ///< Used to identify the WS Master of a Working Set that can be referenced
            ExternalObjectPointer = 43,           ///< Used to reference an object in another Working Set
            Macro = 28,                           ///< Special object that contains a list of commands that can be executed in response to an event.
            AuxiliaryFunctionType1 = 29,          ///< The Auxiliary Function Type 1 object defines the designator and function type for an Auxiliary Function.
            AuxiliaryInputType1 = 30,             ///< The Auxiliary Input Type 1 object defines the designator, key number, and function type for an auxiliary input.
            AuxiliaryFunctionType2 = 31,          ///< The Auxiliary Function Type 2 object defines the designator and function type for an Auxiliary Function.
            AuxiliaryInputType2 = 32,             ///< The Auxiliary Input Type 2 object defines the designator, key number, and function type for an Auxiliary Input.
            AuxiliaryControlDesignatorType2 = 33, ///< Used to reference Auxiliary Input Type 2 object or Auxiliary Function Type 2 object.
            ManufacturerDefined1 = 240,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined2 = 241,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined3 = 242,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined4 = 243,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined5 = 244,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined6 = 245,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined7 = 246,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined8 = 247,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined9 = 248,           ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined10 = 249,          ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined11 = 250,          ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined12 = 251,          ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined13 = 252,          ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined14 = 253,          ///< Manufacturer defined objects should not be sent to any other Vendors VT
            ManufacturerDefined15 = 254,          ///< Manufacturer defined objects should not be sent to any other Vendors VT
            Reserved = 255                        ///< Reserved for future use. (See Clause D.14 Get Supported Objects message)
        };

        /// @brief Enumerates the states that can be sent with a hide/show object command
        enum class HideShowObjectCommand : std::uint8_t
        {
            HideObject = 0, ///< Hides the object
            ShowObject = 1  ///< Shows an object
        };

        /// @brief Enumerates the states that can be sent with an enable/disable object command
        enum class EnableDisableObjectCommand : std::uint8_t
        {
            DisableObject = 0, ///< Disables a compatible object
            EnableObject = 1   ///< Enables a compatible object
        };

        /// @brief Enumerates the states that can be sent with a select input object options command
        enum class SelectInputObjectOptions : std::uint8_t
        {
            ActivateObjectForDataInput = 0x00, ///< Activates an object for data input
            SetFocusToObject = 0xFF            ///< Focuses the object (usually this draws a temporary box around it)
        };

        /// @brief The different VT versions that a client or server might support
        enum class VTVersion
        {
            Version2OrOlder,   ///< Client or server supports VT version 2 or lower
            Version3,          ///< Client or server supports all of VT version 3
            Version4,          ///< Client or server supports all of VT version 4
            Version5,          ///< Client or server supports all of VT version 5
            Version6,          ///< Client or server supports all of VT version 6
            ReservedOrUnknown, ///< Reserved value, not to be used
        };

        /// @brief Enumerates the different line directions that can be used when changing an endpoint of an object
        enum class LineDirection : std::uint8_t
        {
            TopLeftToBottomRightOfEnclosingVirtualRectangle = 0, ///< Draws the line from top left to bottom right of the enclosing virtual rectangle
            BottomLeftToTopRightOfEnclosingVirtualRectangle = 1  ///< Draws the line from bottom left to top right of the enclosing virtual rectangle
        };

        /// @brief Enumerates the different font sizes
        enum class FontSize : std::uint8_t
        {
            Size6x8 = 0,      ///< 6x8 Font size
            Size8x8 = 1,      ///< 8x8 Font size
            Size8x12 = 2,     ///< 8x12 Font size
            Size12x16 = 3,    ///< 12x16 Font size
            Size16x16 = 4,    ///< 16x16 Font size
            Size16x24 = 5,    ///< 16x24 Font size
            Size24x32 = 6,    ///< 24x32 Font size
            Size32x32 = 7,    ///< 32x32 Font size
            Size32x48 = 8,    ///< 32x48 Font size
            Size48x64 = 9,    ///< 48x64 Font size
            Size64x64 = 10,   ///< 64x64 Font size
            Size64x96 = 11,   ///< 64x96 Font size
            Size96x128 = 12,  ///< 96x128 Font size
            Size128x128 = 13, ///< 128x128 Font size
            Size128x192 = 14  ///< 128x192 Font size
        };

        /// @brief Enumerates the font style options that can be encoded in a font style bitfield
        enum class FontStyleBits : std::uint8_t
        {
            Bold = 0,                     ///< Bold font style
            CrossedOut = 1,               ///< Crossed-out font style (strikethrough)
            Underlined = 2,               ///< Underlined font style
            Italic = 3,                   ///< Italic font style
            Inverted = 4,                 ///< Inverted font style (upside down)
            Flashing = 5,                 ///< Flashing font style
            FlashingHidden = 6,           ///< Flashing between hidden and shown font style
            ProportionalFontRendering = 7 ///< Enables proportional font rendering if supported by the server
        };

        /// @brief Enumerates the different font types
        enum class FontType : std::uint8_t
        {
            ISO8859_1 = 0,          ///< ISO Latin 1
            ISO8859_15 = 1,         ///< ISO Latin 9
            ISO8859_2 = 2,          ///< ISO Latin 2
            Reserved_1 = 3,         ///< Reserved
            ISO8859_4 = 4,          ///< ISO Latin 4
            ISO8859_5 = 5,          ///< Cyrillic
            Reserved_2 = 6,         ///< Reserved
            ISO8859_7 = 7,          ///< Greek
            ReservedEnd = 239,      ///< Reserved from ISO8859_7 to this value
            ProprietaryBegin = 240, ///< The beginning of the proprietary range
            ProprietaryEnd = 255    ///< The end of the proprietary region
        };

        /// @brief Enumerates the different fill types for an object
        enum class FillType : std::uint8_t
        {
            NoFill = 0,                                     ///< No fill will be applied
            FillWithLineColor = 1,                          ///< Fill with the color of the outline of the shape
            FillWithSpecifiedColorInFillColorAttribute = 2, ///< Fill with the color specified by a fill attribute
            FillWithPatternGivenByFillPatternAttribute = 3  ///< Fill with a patter provided by a fill pattern attribute
        };

        /// @brief The types of object pool masks
        enum class MaskType : std::uint8_t
        {
            DataMask = 1, ///< A data mask, used in normal circumstances
            AlarmMask = 2 ///< An alarm mask, which has different metadata related to popping up alarms, like priority
        };

        /// @brief The allowable priorities of an alarm mask
        enum class AlarmMaskPriority : std::uint8_t
        {
            High = 0,   ///< Overrides lower priority alarm masks
            Medium = 1, ///< Overrides low priority alarm masks
            Low = 2     ///< Overrides data masks
        };

        /// @brief Denotes the lock/unlock state of a mask. Used to freeze/unfreeze rendering of a mask.
        enum class MaskLockState : std::uint8_t
        {
            UnlockMask = 0, ///< Renders the mask normally
            LockMask = 1    ///< Locks the mask so rendering of it is not updated until it is unlocked or a timeout occurs
        };

        /// @brief The different key activation codes that a button press can generate
        enum class KeyActivationCode : std::uint8_t
        {
            ButtonUnlatchedOrReleased = 0, ///< Button is released
            ButtonPressedOrLatched = 1,    ///< Button is pressed
            ButtonStillHeld = 2,           ///< Button is being held down (sent cyclically)
            ButtonPressAborted = 3         ///< Press was aborted (user navigated away from the button and did not release it)
        };

        /// @brief Enumerates the errors that can be present in an ESC message
        enum class ESCMessageErrorCode : std::uint8_t
        {
            NoError = 0,          ///< No error occurred
            NoInputFieldOpen = 1, ///< No input field is open
            OtherError = 5        ///< Error is not one of the above
        };

        /// @brief Enumerates the different events that can be associated with a macro
        enum class MacroEventID : std::uint8_t
        {
            Reserved = 0,                   ///< Reserved
            OnActivate = 1,                 ///< Event on activation of an object (such as for data input)
            OnDeactivate = 2,               ///< Event on deactivation of an object
            OnShow = 3,                     ///< Event on an object being shown
            OnHide = 4,                     ///< Event on an object being hidden
            OnEnable = 5,                   ///< Event on enable of an object
            OnDisable = 6,                  ///< Event on disabling an object
            OnChangeActiveMask = 7,         ///< Event on changing the active mask
            OnChangeSoftKeyMask = 8,        ///< Event on change of the soft key mask
            OnChangeAttribute = 9,          ///< Event on change of an attribute value
            OnChangeBackgroundColor = 10,   ///< Event on change of a background color
            OnChangeFontAttributes = 11,    ///< Event on change of a font attribute
            OnChangeLineAttributes = 12,    ///< Event on change of a line attribute
            OnChangeFillAttributes = 13,    ///< Event on change of a fill attribute
            OnChangeChildLocation = 14,     ///< Event on change of a child objects location
            OnChangeSize = 15,              ///< Event on change of an object size
            OnChangeValue = 16,             ///< Event on change of an object value (like via `change numeric value`)
            OnChangePriority = 17,          ///< Event on change of a mask's priority
            OnChangeEndPoint = 18,          ///< Event on change of an object endpoint
            OnInputFieldSelection = 19,     ///< Event when an input field is selected
            OnInputFieldDeselection = 20,   ///< Event on deselection of an input field
            OnESC = 21,                     ///< Event on ESC (escape)
            OnEntryOfValue = 22,            ///< Event on entry of a value
            OnEntryOfNewValue = 23,         ///< Event on entry of a *new* value
            OnKeyPress = 24,                ///< Event on the press of a key
            OnKeyRelease = 25,              ///< Event on the release of a key
            OnChangeChildPosition = 26,     ///< Event on changing a child object's position
            OnPointingEventPress = 27,      ///< Event on a pointing event press
            OnPointingEventRelease = 28,    ///< Event on a pointing event release
            ReservedBegin = 29,             ///< Beginning of the reserved range
            ReservedEnd = 254,              ///< End of the reserved range
            UseExtendedMacroReference = 255 ///< Use extended macro reference
        };

        /// @brief Enumerates the various VT server graphics modes
        enum class GraphicMode : std::uint8_t
        {
            Monochrome = 0,             ///< Monochromatic graphics mode (1 bit)
            SixteenColour = 1,          ///< 16 Color mode (4 bit)
            TwoHundredFiftySixColor = 2 ///< 256 Color mode (8 bit)
        };

        static constexpr std::uint16_t NULL_OBJECT_ID = 0xFFFF; ///< The NULL Object ID, usually drawn as blank space

    protected:
        /// @brief Enumerates the multiplexor byte values for VT commands
        enum class Function : std::uint8_t
        {
            SoftKeyActivationMessage = 0x00,
            ButtonActivationMessage = 0x01,
            PointingEventMessage = 0x02,
            VTSelectInputObjectMessage = 0x03,
            VTESCMessage = 0x04,
            VTChangeNumericValueMessage = 0x05,
            VTChangeActiveMaskMessage = 0x06,
            VTChangeSoftKeyMaskMessage = 0x07,
            VTChangeStringValueMessage = 0x08,
            VTOnUserLayoutHideShowMessage = 0x09,
            VTControlAudioSignalTerminationMessage = 0x0A,
            ObjectPoolTransferMessage = 0x11,
            EndOfObjectPoolMessage = 0x12,
            AuxiliaryAssignmentTypeOneCommand = 0x20,
            AuxiliaryInputTypeOneStatus = 0x21,
            PreferredAssignmentCommand = 0x22,
            AuxiliaryInputTypeTwoMaintenanceMessage = 0x23,
            AuxiliaryAssignmentTypeTwoCommand = 0x24,
            AuxiliaryInputStatusTypeTwoEnableCommand = 0x25,
            AuxiliaryInputTypeTwoStatusMessage = 0x26,
            AuxiliaryCapabilitiesRequest = 0x27,
            SelectActiveWorkingSet = 0x90,
            ESCCommand = 0x92,
            HideShowObjectCommand = 0xA0,
            EnableDisableObjectCommand = 0xA1,
            SelectInputObjectCommand = 0xA2,
            ControlAudioSignalCommand = 0xA3,
            SetAudioVolumeCommand = 0xA4,
            ChangeChildLocationCommand = 0xA5,
            ChangeSizeCommand = 0xA6,
            ChangeBackgroundColourCommand = 0xA7,
            ChangeNumericValueCommand = 0xA8,
            ChangeEndPointCommand = 0xA9,
            ChangeFontAttributesCommand = 0xAA,
            ChangeLineAttributesCommand = 0xAB,
            ChangeFillAttributesCommand = 0xAC,
            ChangeActiveMaskCommand = 0xAD,
            ChangeSoftKeyMaskCommand = 0xAE,
            ChangeAttributeCommand = 0xAF,
            ChangePriorityCommand = 0xB0,
            ChangeListItemCommand = 0xB1,
            DeleteObjectPoolCommand = 0xB2,
            ChangeStringValueCommand = 0xB3,
            ChangeChildPositionCommand = 0xB4,
            ChangeObjectLabelCommand = 0xB5,
            ChangePolygonPointCommand = 0xB6,
            ChangePolygonScaleCommand = 0xB7,
            GraphicsContextCommand = 0xB8,
            GetAttributeValueMessage = 0xB9,
            SelectColourMapCommand = 0xBA,
            IdentifyVTMessage = 0xBB,
            ExecuteExtendedMacroCommand = 0xBC,
            LockUnlockMaskCommand = 0xBD,
            ExecuteMacroCommand = 0xBE,
            GetMemoryMessage = 0xC0,
            GetSupportedWidecharsMessage = 0xC1,
            GetNumberOfSoftKeysMessage = 0xC2,
            GetTextFontDataMessage = 0xC3,
            GetWindowMaskDataMessage = 0xC4,
            GetSupportedObjectsMessage = 0xC5,
            GetHardwareMessage = 0xC7,
            StoreVersionCommand = 0xD0,
            LoadVersionCommand = 0xD1,
            DeleteVersionCommand = 0xD2,
            ExtendedGetVersionsMessage = 0xD3,
            ExtendedStoreVersionCommand = 0xD4,
            ExtendedLoadVersionCommand = 0xD5,
            ExtendedDeleteVersionCommand = 0xD6,
            GetVersionsMessage = 0xDF,
            GetVersionsResponse = 0xE0,
            UnsupportedVTFunctionMessage = 0xFD,
            VTStatusMessage = 0xFE,
            WorkingSetMaintenanceMessage = 0xFF
        };

        /// @brief Enumerates the command types for graphics context objects
        enum class GraphicsContextSubCommandID : std::uint8_t
        {
            SetGraphicsCursor = 0x00,           ///< Sets the graphics cursor x/y attributes
            MoveGraphicsCursor = 0x01,          ///< Moves the cursor relative to current location
            SetForegroundColor = 0x02,          ///< Sets the foreground color
            SetBackgroundColor = 0x03,          ///< Sets the background color
            SetLineAttributesObjectID = 0x04,   ///< Sets the line attribute object ID
            SetFillAttributesObjectID = 0x05,   ///< Sets the fill attribute object ID
            SetFontAttributesObjectID = 0x06,   ///< Sets the font attribute object ID
            EraseRectangle = 0x07,              ///< Erases a rectangle
            DrawPoint = 0x08,                   ///< Draws a point
            DrawLine = 0x09,                    ///< Draws a line
            DrawRectangle = 0x0A,               ///< Draws a rectangle
            DrawClosedEllipse = 0x0B,           ///< Draws a closed ellipse
            DrawPolygon = 0x0C,                 ///< Draws polygon
            DrawText = 0x0D,                    ///< Draws text
            PanViewport = 0x0E,                 ///< Pans viewport
            ZoomViewport = 0x0F,                ///< Zooms the viewport
            PanAndZoomViewport = 0x10,          ///< Pan and zooms the viewport
            ChangeViewportSize = 0x11,          ///< Changes the viewport size
            DrawVTObject = 0x12,                ///< Draws a VT object
            CopyCanvasToPictureGraphic = 0x13,  ///< Copies the canvas to picture graphic object
            CopyViewportToPictureGraphic = 0x14 ///< Copies the viewport to picture graphic object
        };
    };
} // namespace isobus

#endif // ISOBUS_VIRTUAL_TERMINAL_BASE_HPP
