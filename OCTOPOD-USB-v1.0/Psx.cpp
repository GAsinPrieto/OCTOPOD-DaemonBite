/*  PSX Controller Decoder Library (Psx.cpp)
	Written by: Kevin Ahrendt June 22nd, 2008

	Controller protocol implemented using Andrew J McCubbin's analysis.
	http://www.gamesx.com/controldata/psxcont/psxcont.htm

	Shift command is based on tutorial examples for ShiftIn and ShiftOut
	functions both written by Carlyn Maw and Tom Igoe
	http://www.arduino.cc/en/Tutorial/ShiftIn
	http://www.arduino.cc/en/Tutorial/ShiftOut

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Psx.h"

Psx::Psx()
{
}

byte Psx::shift(byte _dataOut)							// Does the actual shifting, both in and out simultaneously
{
  _temp = 0;
  _dataIn = 0;

  for (_i = 0; _i <= 7; _i++)
  {


    if ( _dataOut & (1 << _i) ) digitalWrite(_cmndPin, HIGH);	// Writes out the _dataOut bits
    else digitalWrite(_cmndPin, LOW);

    digitalWrite(_clockPin, LOW);

    delayMicroseconds(_delay);

    _temp = digitalRead(_dataPin);					// Reads the data pin
    if (_temp)
    {
      /*if (_dataOut != 0x42)*/ _dataIn = _dataIn | (B10000000 >> _i);		// Shifts the read data into _dataIn
      //else _dataIn = _dataIn | (B00000001 << _i); //no shifting when asking for the controller mode
    }

    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(_delay);
  }
  return _dataIn;
}

byte Psx::noshift(byte _dataOut)              // Does NOT the actual shifting, both in and out simultaneously
{
  _temp = 0;
  _dataIn = 0;

  for (_i = 0; _i <= 7; _i++)
  {


    if ( _dataOut & (1 << _i) ) digitalWrite(_cmndPin, HIGH); // Writes out the _dataOut bits
    else digitalWrite(_cmndPin, LOW);

    digitalWrite(_clockPin, LOW);

    delayMicroseconds(_delay);

    _temp = digitalRead(_dataPin);          // Reads the data pin
    if (_temp)
    {
      /*if (_dataOut != 0x42) _dataIn = _dataIn | (B10000000 >> _i);    // Shifts the read data into _dataIn
        else*/ _dataIn = _dataIn | (B00000001 << _i); //no shifting when asking for the controller mode
    }

    digitalWrite(_clockPin, HIGH);
    delayMicroseconds(_delay);
  }
  return _dataIn;
}

void Psx::setupPins(byte dataPin, byte cmndPin, byte attPin, byte clockPin, byte ackPin, byte delay_)
{
  pinMode(dataPin, INPUT_PULLUP);
  //digitalWrite(dataPin, HIGH);	// Turn on internal pull-up
  _dataPin = dataPin;

  pinMode(ackPin, INPUT_PULLUP);
  //digitalWrite(ackPin, HIGH);  // Turn on internal pull-up
  _ackPin = ackPin;

  pinMode(cmndPin, OUTPUT);
  _cmndPin = cmndPin;

  pinMode(attPin, OUTPUT);
  _attPin = attPin;
  digitalWrite(_attPin, HIGH);

  pinMode(clockPin, OUTPUT);
  _clockPin = clockPin;
  digitalWrite(_clockPin, HIGH);

  _delay = delay_;
}


/*unsigned int*/ void Psx::read(uint16_t * data1, uint16_t * data2, uint16_t * data3, uint16_t * data4, uint16_t * data5, uint16_t * data6, byte * mode)
{
  byte motor1=0x00;
  byte motor2=0x00;
  
  
  digitalWrite(_attPin, LOW);

  shift(0x01);
  while (_ackPin);
  _mode = noshift(0x42);
  while (_ackPin);
  shift(0xFF);
  while (_ackPin);
  *mode = _mode;

  if (_mode == 0x41) //digital
  {
    _data1 = ~shift(motor1);
    while (_ackPin);
    _data2 = ~shift(motor2);

    *data1 = _data1;
    *data2 = _data2;
    digitalWrite(_attPin, HIGH);

    //_dataOut = (_data2 << 8) | _data1;


    return;// _dataOut;
  }
  else if (_mode == 0x73 || _mode == 0x53) //just analog (red, green)
  {
    _data1 = ~shift(motor1);
    while (_ackPin);
    _data2 = ~shift(motor2);
    while (_ackPin);
    _data3 = ~noshift(0xFF);
    while (_ackPin);
    _data4 = ~noshift(0xFF);
    while (_ackPin);
    _data5 = ~noshift(0xFF);
    while (_ackPin);
    _data6 = ~noshift(0xFF);

    *data1 = _data1;
    *data2 = _data2;
    *data3 = _data3;
    *data4 = _data4;
    *data5 = _data5;
    *data6 = _data6;

    digitalWrite(_attPin, HIGH);

    //_dataOut = (_data2 << 8) | _data1;
    return;// _dataOut;
  }
}



void Psx::rumble(byte mode)
{

  //enter config
  digitalWrite(_attPin, LOW);

  shift(0x01);
  while (_ackPin);
  shift(0x43);
  while (_ackPin);
  shift(0x00);
  while (_ackPin);
  shift(0x01);
  while (_ackPin);
  shift(0x00);
  while (_ackPin);

  digitalWrite(_attPin, HIGH);

  delay(200);

  /*
    //analog mode by default
    digitalWrite(_attPin, LOW);

    shift(0x01);
    while (_ackPin);
    shift(0x44);
    while (_ackPin);
    shift(0x00);
    while (_ackPin);
    shift(0x01);
    while (_ackPin);
    shift(0x03);
    while (_ackPin);
    shift(0x00);
    while (_ackPin);
    shift(0x00);
    while (_ackPin);
    shift(0x00);
    while (_ackPin);
    shift(0x00);
    while (_ackPin);

    digitalWrite(_attPin, HIGH);

    delay(200);
  */
  //map motors
  digitalWrite(_attPin, LOW);

  shift(0x01);
  while (_ackPin);
  shift(0x4D);
  while (_ackPin);
  shift(0x00);
  while (_ackPin);
  shift(0x00);
  while (_ackPin);
  shift(0x01);
  while (_ackPin);
  if (mode != 0x41)
  {
    shift(0xff);
    while (_ackPin);
    shift(0xff);
    while (_ackPin);
    shift(0xff);
    while (_ackPin);
    shift(0xff);
    while (_ackPin);
  }

  digitalWrite(_attPin, HIGH);

  delay(100);

  //exit cofig
  digitalWrite(_attPin, LOW);

  shift(0x01);
  while (_ackPin);
  shift(0x43);
  while (_ackPin);
  shift(0x00);
  while (_ackPin);
  shift(0x00);
  while (_ackPin);
  shift(0x5a);
  while (_ackPin);
  if (mode != 0x41)
  {
    shift(0x5a);
    while (_ackPin);
    shift(0x5a);
    while (_ackPin);
    shift(0x5a);
    while (_ackPin);
    shift(0x5a);
    while (_ackPin);
  }

  digitalWrite(_attPin, HIGH);

  return;
}
