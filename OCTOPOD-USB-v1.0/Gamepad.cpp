/*  Gamepad.cpp
 *   
 *  Based on the advanced HID library for Arduino: 
 *  https://github.com/NicoHood/HID
 *  Copyright (c) 2014-2015 NicoHood
 * 
 *  Copyright (c) 2020 Mikael Norrgård <http://daemonbite.com>
 *
 *  GNU GENERAL PUBLIC LICENSE
 *  Version 3, 29 June 2007
 *  
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  
 */
#include "Gamepad.h"

#define NOT_SELECTED 0
#define SNES 1
#define NEOGEO 2
#define GENESIS 3
#define PSX 4

int SISTEMAgp = NOT_SELECTED;

//GENERIC
static const uint8_t _hidReportDescriptor[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x09,                       // USAGE_MAXIMUM (Button 9)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x09,                       // REPORT_COUNT (9)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x07,                       // REPORT_SIZE (7)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)
      
      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x01,                       // USAGE (pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)
        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};




//SNES
static const uint8_t _hidReportDescriptorSNES[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x18,                       // USAGE_MAXIMUM (Button 24)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x18,                       // REPORT_COUNT (24)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x01,                       // USAGE (pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)
        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};

//GENESIS
static const uint8_t _hidReportDescriptorGEN[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x09,                       // USAGE_MAXIMUM (Button 9)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x09,                       // REPORT_COUNT (9)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x07,                       // REPORT_SIZE (7)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)
      
      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x01,                       // USAGE (pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)
        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};

//PSX
static const uint8_t _hidReportDescriptorPSX[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x0c,                       // USAGE_MAXIMUM (Button 12)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x0c,                       // REPORT_COUNT (12)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)
    

      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x39,                       // USAGE (HAT SWITCH)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x07,                       // LOGICAL_MAXIMUM (7)
      //0x35, 0x00,  // PHYSICAL_MINIMUM (0)
      //0x46, 0x3B, 0x01, // PHYSICAL_MAXIMUM (315)
      //0x65, 0x14, // UNIT (Eng Rot:Angular Pos)
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x95, 0x01,                       // REPORT_COUNT (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)

      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x05,                       // USAGE (Game Pad)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)        
        0x09, 0x32,                       // USAGE (Z)
        0x09, 0x35,                       // USAGE (RZ)
        0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
        0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
        0x15, 0x00,                       // LOGICAL_MINIMUM (0)
        0x26, 0xff, 0x00,                       // LOGICAL_MAXIMUM (255)
        0x95, 0x04,                       // REPORT_COUNT (4)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0, 
};

/*static const uint8_t _hidReportDescriptorPSX[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x0c,                       // USAGE_MAXIMUM (Button 12)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x0c,                       // REPORT_COUNT (12)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)
    
*/
      /*0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x39,                       // USAGE (HAT SWITCH)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x07,                       // LOGICAL_MAXIMUM (7)
      //0x35, 0x00,  // PHYSICAL_MINIMUM (0)
      //0x46, 0x3B, 0x01, // PHYSICAL_MAXIMUM (315)
      //0x65, 0x14, // UNIT (Eng Rot:Angular Pos)
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x95, 0x01,                       // REPORT_COUNT (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)*/
/*
      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      //0x09, 0x05,                       // USAGE (Game Pad)
      0x09, 0x01,                       // USAGE (Pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
*/        /*0x09, 0x32,                       // USAGE (Z)
        0x09, 0x35,                       // USAGE (RZ)*/
/*        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)        
        //0x35, 0x00,                    //     PHYSICAL_MINIMUM (0)
        //0x46, 0xff, 0x00,              //     PHYSICAL_MAXIMUM (255)
*/        /*0x15, 0x00,                       // LOGICAL_MINIMUM (-127)
        0x26, 0xff, 0x00,                       // LOGICAL_MAXIMUM (127)*/
/*        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        //0x95, 0x04,                       // REPORT_COUNT (4)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0, 
};*/

//NES
static const uint8_t _hidReportDescriptorNES[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x04,                       // USAGE_MAXIMUM (Button 4)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x04,                       // REPORT_COUNT (4)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)

      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x01,                       // USAGE (pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)
        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};

//NEOGEO
static const uint8_t _hidReportDescriptorNG[] PROGMEM = {
  0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
  0x09, 0x04,                       // USAGE (Joystick) (Maybe change to gamepad? I don't think so but...)
  0xa1, 0x01,                       // COLLECTION (Application)
    0xa1, 0x00,                       // COLLECTION (Physical)
    
      0x05, 0x09,                       // USAGE_PAGE (Button)
      0x19, 0x01,                       // USAGE_MINIMUM (Button 1)
      0x29, 0x0c,                       // USAGE_MAXIMUM (Button 12)
      0x15, 0x00,                       // LOGICAL_MINIMUM (0)
      0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
      0x95, 0x0c,                       // REPORT_COUNT (12)
      0x75, 0x01,                       // REPORT_SIZE (1)
      0x81, 0x02,                       // INPUT (Data,Var,Abs)

      0x95, 0x01,                       // REPORT_COUNT (1) ; pad out the bits into a number divisible by 8
      0x75, 0x04,                       // REPORT_SIZE (4)
      0x81, 0x03,                       // INPUT (Const,Var,Abs)

      0x05, 0x01,                       // USAGE_PAGE (Generic Desktop)
      0x09, 0x01,                       // USAGE (pointer)
      0xa1, 0x00,                       // COLLECTION (Physical) 
        0x09, 0x30,                       // USAGE (X)
        0x09, 0x31,                       // USAGE (Y)
        0x15, 0xff,                       // LOGICAL_MINIMUM (-1)
        0x25, 0x01,                       // LOGICAL_MAXIMUM (1)
        0x95, 0x02,                       // REPORT_COUNT (2)
        0x75, 0x08,                       // REPORT_SIZE (8)
        0x81, 0x02,                       // INPUT (Data,Var,Abs)
      0xc0,                             // END_COLLECTION

    0xc0,                             // END_COLLECTION
  0xc0,                             // END_COLLECTION 
};

//Gamepad_::Gamepad_(void) : PluggableUSBModule(1, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1)
Gamepad_::Gamepad_(int SYSTEM) : PluggableUSBModule(1, 1, epType), protocol(HID_REPORT_PROTOCOL), idle(1)
{
  SISTEMAgp=SYSTEM;
  epType[0] = EP_TYPE_INTERRUPT_IN;
  PluggableUSB().plug(this);
}

int Gamepad_::getInterface(uint8_t* interfaceCount)
{
  *interfaceCount += 1; // uses 1
  if (SISTEMAgp == GENESIS)
  {
    HIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptorGEN)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }
  /*else if (SISTEMAgp == NES)
  {
    HIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptorNES)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }*/
  else if (SISTEMAgp == SNES)
  {
    HIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptorSNES)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }
  else if (SISTEMAgp == PSX)
  {
    HIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptorPSX)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }
  else if (SISTEMAgp == NEOGEO)
  {
    HIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptorNG)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }
  else
  {
      HIDDescriptor hidInterface = {
      D_INTERFACE(pluggedInterface, 1, USB_DEVICE_CLASS_HUMAN_INTERFACE, HID_SUBCLASS_NONE, HID_PROTOCOL_NONE),
      D_HIDREPORT(sizeof(_hidReportDescriptor)),
      D_ENDPOINT(USB_ENDPOINT_IN(pluggedEndpoint), USB_ENDPOINT_TYPE_INTERRUPT, USB_EP_SIZE, 0x01)
    };
    return USB_SendControl(0, &hidInterface, sizeof(hidInterface));
  }
  
}

int Gamepad_::getDescriptor(USBSetup& setup)
{
  // Check if this is a HID Class Descriptor request
  if (setup.bmRequestType != REQUEST_DEVICETOHOST_STANDARD_INTERFACE) { return 0; }
  if (setup.wValueH != HID_REPORT_DESCRIPTOR_TYPE) { return 0; }

  // In a HID Class Descriptor wIndex cointains the interface number
  if (setup.wIndex != pluggedInterface) { return 0; }

  // Reset the protocol on reenumeration. Normally the host should not assume the state of the protocol
  // due to the USB specs, but Windows and Linux just assumes its in report mode.
  protocol = HID_REPORT_PROTOCOL;

  if (SISTEMAgp == GENESIS)
    return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorGEN, sizeof(_hidReportDescriptorGEN));
  /*else if (SISTEMAgp == NES)
    return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorNES, sizeof(_hidReportDescriptorNES));
  */else if (SISTEMAgp == SNES)
    return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorSNES, sizeof(_hidReportDescriptorSNES));
  else if (SISTEMAgp == NEOGEO)
    return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorNG, sizeof(_hidReportDescriptorNG));
  else if (SISTEMAgp == PSX)
    return USB_SendControl(TRANSFER_PGM, _hidReportDescriptorPSX, sizeof(_hidReportDescriptorPSX));
  else
    return USB_SendControl(TRANSFER_PGM, _hidReportDescriptor, sizeof(_hidReportDescriptor));
}

bool Gamepad_::setup(USBSetup& setup)
{
  if (pluggedInterface != setup.wIndex) {
    return false;
  }

  uint8_t request = setup.bRequest;
  uint8_t requestType = setup.bmRequestType;

  if (requestType == REQUEST_DEVICETOHOST_CLASS_INTERFACE)
  {
    if (request == HID_GET_REPORT) {
      // TODO: HID_GetReport();
      return true;
    }
    if (request == HID_GET_PROTOCOL) {
      // TODO: Send8(protocol);
      return true;
    }
  }

  if (requestType == REQUEST_HOSTTODEVICE_CLASS_INTERFACE)
  {
    if (request == HID_SET_PROTOCOL) {
      protocol = setup.wValueL;
      return true;
    }
    if (request == HID_SET_IDLE) {
      idle = setup.wValueL;
      return true;
    }
    if (request == HID_SET_REPORT)
    {
    }
  }

  return false;
}

void Gamepad_::reset()
{
  if (SISTEMAgp == SNES)
  {
    _GamepadReport_SNES.X = 0;
    _GamepadReport_SNES.Y = 0;
    _GamepadReport_SNES.buttons = 0;
  }
  /*if (SISTEMAgp == NES)
  {
    _GamepadReport_NES.X = 0;
    _GamepadReport_NES.Y = 0;
    _GamepadReport_NES.buttons = 0;
  }*/
  if (SISTEMAgp == GENESIS)
  {
    _GamepadReport_GENESIS.X = 0;
    _GamepadReport_GENESIS.Y = 0;
    _GamepadReport_GENESIS.buttons = 0;
  }
  if (SISTEMAgp == PSX)
  {
    _GamepadReport_PSX.Z = 0;
    _GamepadReport_PSX.RZ = 0;
    _GamepadReport_PSX.PoV = 0;
    _GamepadReport_PSX.X = 0;
    _GamepadReport_PSX.Y = 0;
    _GamepadReport_PSX.buttons = 0;
  }
  if (SISTEMAgp == NEOGEO)
  {
    _GamepadReport_NEOGEO.X = 0;
    _GamepadReport_NEOGEO.Y = 0;
    _GamepadReport_NEOGEO.buttons = 0;
  }
  if (SISTEMAgp == NOT_SELECTED)
  {
    _GamepadReport.X = 0;
    _GamepadReport.Y = 0;
    _GamepadReport.buttons = 0;
  }
  this->send();
}

void Gamepad_::send() 
{
  if (SISTEMAgp == SNES)
  {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport_SNES, sizeof(GamepadReport_SNES));
  }
  /*if (SISTEMAgp == NES)
  {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport_NES, sizeof(GamepadReport_NES));
  }*/
  if (SISTEMAgp == GENESIS)
  {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport_GENESIS, sizeof(GamepadReport_GENESIS));
  }
  if (SISTEMAgp == PSX)
  {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport_PSX, sizeof(GamepadReport_PSX));
  }
  if (SISTEMAgp == NEOGEO)
  {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport_NEOGEO, sizeof(GamepadReport_NEOGEO));
  }
  if (SISTEMAgp == NOT_SELECTED)
  {
    USB_Send(pluggedEndpoint | TRANSFER_RELEASE, &_GamepadReport, sizeof(GamepadReport));
  }
}

uint8_t Gamepad_::getShortName(char *name)
{
  if(!next) 
  {
    strcpy(name, gp_serial);
    return strlen(name);
  }
  return 0;
}
