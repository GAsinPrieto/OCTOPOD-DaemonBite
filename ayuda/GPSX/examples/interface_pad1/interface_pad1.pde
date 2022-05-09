/*
This example demonstrates all features of the library.

- The controller starts up in ANALOG mode, and is locked
  (user cannot switch modes using the ANALOG button)
  
- SQUARE and CROSS buttons toggle between ANALOG and DIGITAL modes.
  also, at this point the lock is released.
  
- CIRCLE button rotates the motor1 while pressed. This motor has
  a fixed rotation speed and its state is limited to on or off

- While pressing TRIANGLE, motor2 rotates. Its rotation speed is
  controlled by the X-axis of the left analog stick.

Pin layouts are hardcoded in GPSX.c.
For function details, refer to documents attached to this library.
*/

#include <GPSXClass.h>


void setup()
{
  Serial.begin(38400);
  PSX.mode(PSX_PAD1, MODE_ANALOG, MODE_LOCK);
  
  PSX.motorEnable(PSX_PAD1, MOTOR1_ENABLE, MOTOR2_ENABLE);
  
  // Poll current state once.
  PSX.updateState(PSX_PAD1);
}

void loop()
{
  PSX.updateState(PSX_PAD1);

  if (PRESSED_CIRCLE(PSX_PAD1)) {
    Serial.println("Pressed circle");
    PSX.motor(PSX_PAD1, MOTOR1_ON, 0x00);
  }

  if (RELEASED_CIRCLE(PSX_PAD1)) {
    Serial.println("Released circle");
    PSX.motor(PSX_PAD1, MOTOR1_OFF, 0x00);
  }    

  if (PRESSED_SQUARE(PSX_PAD1)) {
    Serial.println("Pressed square");
    PSX.mode(PSX_PAD1, MODE_DIGITAL, MODE_UNLOCK);
  }

  if (RELEASED_SQUARE(PSX_PAD1)) {
    Serial.println("Released square");
  }

  if (PRESSED_CROSS(PSX_PAD1)) {
    Serial.println("Pressed cross");
    PSX.mode(PSX_PAD1, MODE_ANALOG, MODE_UNLOCK);
    PSX.motorEnable(PSX_PAD1, MOTOR1_ENABLE, MOTOR2_ENABLE);
  }

  if (RELEASED_CROSS(PSX_PAD1)) {
    Serial.println("Released cross");
  }

  if (IS_DOWN_TRIANGLE(PSX_PAD1)) {
    PSX.motor(PSX_PAD1, MOTOR1_OFF, ANALOG_LEFT_X(PSX_PAD1));
  }

  if (PRESSED_TRIANGLE(PSX_PAD1)) {
    Serial.println("Pressed triangle");
  }

  if (RELEASED_TRIANGLE(PSX_PAD1)) {
    Serial.println("Released triangle");
    PSX.motor(PSX_PAD1, MOTOR1_OFF, 0x00);
  }
}
