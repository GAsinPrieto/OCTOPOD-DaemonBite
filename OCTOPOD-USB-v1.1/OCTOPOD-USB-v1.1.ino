#include <Keyboard.h>
#include "Gamepad.h"
#include "SegaControllers32U4.h"
#include "shift_74597.h"
#include "Psx.h"

#define NOT_SELECTED 0
#define SNES_ 1
#define NEOGEO_ 2
#define GENESIS_ 3
#define PSX_ 4



//NEO GEO
#define QH 2 //PD1 IN

#define DEBOUNCE 0          // 1=Diddly-squat-Delay-Debouncing™ activated, 0=Debounce deactivated
#define DEBOUNCE_TIME 10    // Debounce time in milliseconds
//#define DEBUG             // Enables debugging (sends debug data to usb serial)

shift_74597 myShifter = shift_74597(QH);

bool gamepad_init = true;

bool usbUpdate = false;     // Should gamepad data be sent to USB?
bool debounce = DEBOUNCE;   // Debounce?
uint8_t  pin;               // Used in for loops
uint32_t millisNow = 0;     // Used for Diddly-squat-Delay-Debouncing™

uint8_t  axesDirect[2] = {0x0f, 0x0f};
uint8_t  axes[2] = {0x0f, 0x0f};
uint8_t  axesPrev[2] = {0x0f, 0x0f};
uint8_t  axesBits[4] = {0x10, 0x20, 0x40, 0x80};
uint32_t axesMillis[4];

uint16_t buttonsDirect[2] = {0, 0};
uint16_t buttons_NG[2] = {0, 0};
uint16_t buttonsPrev_NG[2] = {0, 0};
uint16_t buttonsBits[12] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x100, 0x200, 0x400, 0x800};
uint32_t buttonsMillis[12];

char myInput0;
char myInput1;
char myInput2;

#ifdef DEBUG
char buf[16];
uint32_t millisSent = 0;
#endif

uint8_t reverse(uint8_t in)
{
  uint8_t out;
  out = 0;
  if (in & 0x01) out |= 0x80;
  if (in & 0x02) out |= 0x40;
  if (in & 0x04) out |= 0x20;
  if (in & 0x08) out |= 0x10;
  if (in & 0x10) out |= 0x08;
  if (in & 0x20) out |= 0x04;
  if (in & 0x40) out |= 0x02;
  if (in & 0x80) out |= 0x01;

  return (out);
}

//NES
int GAMEPAD_COUNT = 2;		// NOTE: No more than TWO gamepads are possible at the moment due to a USB HID issue.
#define GAMEPAD_COUNT_MAX 4  // NOTE: For some reason, can't have more than two gamepads without serial breaking. Can someone figure out why?
//       (It has something to do with how Arduino handles HID devices)
#define BUTTON_COUNT       8 // Standard NES controller has four buttons and four axes, totalling 8
#define BUTTON_READ_DELAY 20 // Delay between button reads in µs
#define MICROS_LATCH_NES       8 // 12µs according to specs (8 seems to work fine)
#define MICROS_CLOCK_NES       4 //  6µs according to specs (4 seems to work fine)
#define MICROS_PAUSE_NES       4 //  6µs according to specs (4 seems to work fine)

// Set up USB HID gamepads
//Gamepad_ Gamepad[GAMEPAD_COUNT];

//SNES;
#define MICROS_LATCH_SNES      10 // 12µs according to specs (8 seems to work fine)
#define MICROS_CLOCK_SNES       5 //  6µs according to specs (4 seems to work fine)
#define MICROS_PAUSE_SNES       5 //  6µs according to specs (4 seems to work fine)

#define CYCLES_LATCH     128 // 12µs according to specs (8 seems to work fine) (1 cycle @ 16MHz takes 62.5ns so 62.5ns * 128 = 8000ns = 8µs)
#define CYCLES_CLOCK      64 //  6µs according to specs (4 seems to work fine)
#define CYCLES_PAUSE      64 //  6µs according to specs (4 seems to work fine)


enum ControllerType {
  NONE,
  NES,
  SNES,
  NTT
};

//GENESIS
SegaControllers32U4 controllers;

// Controller previous states
word lastState[2] = {1, 1};




#define UP    0x01
#define DOWN  0x02
#define LEFT  0x04
#define RIGHT 0x08

#define NTT_CONTROL_BIT 0x20000000

#define DELAY_CYCLES(n) __builtin_avr_delay_cycles(n)

// Controllers
uint8_t buttons[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
uint8_t buttonsPrev[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
//uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B10000000,B01000000,B00100000,B00010000};
uint8_t gpBit[GAMEPAD_COUNT_MAX] = {B00000010, B00000100, B00001000, B00010000};


//SNES
uint32_t buttons_SNES[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
uint32_t buttonsPrev_SNES[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
ControllerType controllerType[GAMEPAD_COUNT_MAX] = {NONE, NONE, NONE, NONE};
uint32_t btnBits_SNES[32] = {0x10, 0x40, 0x400, 0x800, UP, DOWN, LEFT, RIGHT, 0x20, 0x80, 0x100, 0x200, // Standard SNES controller
                             0x10000000, 0x20000000, 0x40000000, 0x80000000, 0x1000, 0x2000, 0x4000, 0x8000, // NTT Data Keypad (NDK10)
                             0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000, 0x400000, 0x800000,
                             0x1000000, 0x2000000, 0x4000000, 0x8000000
                            };
uint8_t buttonCount = 32;

uint8_t btnBits_NES[BUTTON_COUNT] = {0x20, 0x10, 0x40, 0x80, UP, DOWN, LEFT, RIGHT};

uint8_t gp = 0;
// Timing
uint32_t microsButtons = 0;


//PSX
Psx Psx[2];

uint16_t buttons_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t buttonsPrev_PSX[GAMEPAD_COUNT_MAX] = {0, 0, 0, 0};
uint16_t horizontalR_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t horizontalRPrev_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t verticalR_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t verticalRPrev_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t horizontalL_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t horizontalLPrev_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t verticalL_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};
uint16_t verticalLPrev_PSX[GAMEPAD_COUNT_MAX]     = {0, 0, 0, 0};

#define dataPin   3
#define cmndPin   A11//12
#define attPin1   A0//30
#define attPin2   A6//4
#define clockPin  2
#define ackPin    0






const int pinPSX = A0;
const int pinSNES = A2;
const int pinNEOGEO = A1;
const int pinGENESIS = A3;


int outputValueSNES = 0;        // value output to the PWM (analog out)
int outputValueNES = 0;        // value output to the PWM (analog out)
int outputValueNEOGEO = 0;        // value output to the PWM (analog out)
int outputValueGENESIS = 0;        // value output to the PWM (analog out)
int SISTEMA = 0;

const char *gp_serial = "DECAPOD";

void setup() {

  Keyboard.begin();
  
  while (!(UDADDR & _BV(ADDEN))) { //check USB connection
    //SerialNotInit=true;
    DDRD  = B00000000;
    //PORTD = B00000000;
    PORTD = B01110111; //pull ups for PSX to properly work
    DDRF  = B00000000;
    PORTF = B00000000;
  }
  //PORTD = B00000000;

  pinMode(pinSNES, OUTPUT);
  pinMode(pinPSX, OUTPUT);
  pinMode(pinNEOGEO, OUTPUT);
  pinMode(pinGENESIS, OUTPUT);
  DDRD  &= ~B00100000; // inputs
  PORTD |=  B00100000; // enable internal pull-ups

  analogWrite(pinPSX, 255);
  if ((PIND & B00100000) >> 5 == 1) SISTEMA = PSX_;
  else {
    analogWrite(pinPSX, 0);
    analogWrite(pinSNES, 255);
    if ((PIND & B00100000) >> 5 == 1) SISTEMA = SNES_;
    else {
      analogWrite(pinSNES, 0);
      analogWrite(pinNEOGEO, 255);
      if ((PIND & B00100000) >> 5 == 1) SISTEMA = NEOGEO_;
      else {
        analogWrite(pinNEOGEO, 0);
        analogWrite(pinGENESIS, 255);
        if ((PIND & B00100000) >> 5 == 1) SISTEMA = GENESIS_;
      }
    }
  }

  pinMode(pinSNES, INPUT);
  pinMode(pinPSX, INPUT);
  pinMode(pinNEOGEO, INPUT);
  pinMode(pinGENESIS, INPUT);

}

void loop() {

  uint16_t data1, data2, data3, data4, data5, data6;
  byte mode, prevmode = 0;

  //if (SISTEMA == PSX_) GAMEPAD_COUNT = 1;
  Gamepad_ Gamepad[GAMEPAD_COUNT](SISTEMA);

  if (SISTEMA == GENESIS_)
  {
    controllers.setup_controllers();
    for (byte gp = 0; gp <= 1; gp++)
      Gamepad[gp].reset();
  }
  else if (SISTEMA == SNES_) {
    DDRD  |=  B10110000; // output

    PORTD &= ~B10110000; // low

    // Setup data pins (A0-A3 or PF7-PF4)
    DDRD  &= ~B00000110; // inputs
    PORTD |=  B00000110; // enable internal pull-ups

    delay(500);

    //SNES
    if (SISTEMA == SNES_) detectControllerTypes();
  }
  else if (SISTEMA == PSX_) {

    Psx[0].setupPins(dataPin, cmndPin, attPin1, clockPin, ackPin, 10);
    Psx[1].setupPins(dataPin, cmndPin, attPin2, clockPin, ackPin, 10);

    //Psx[0].rumble(0x41);
    //Psx[1].rumble(0x41);
  }
  else if (SISTEMA == NEOGEO_) {
    // Initialize debouncing timestamps
    for (pin = 0; pin < 4; pin++)
      axesMillis[pin] = 0;
    for (pin = 0; pin < 12; pin++)
      buttonsMillis[pin] = 0;

#ifdef DEBUG
    Serial.begin(115200);
#endif

    myShifter.init();
  }

  switch (SISTEMA) {
    case NOT_SELECTED:
      break;

    case SNES_:
      while (1)
      {
        // See if enough time has passed since last button read
        if ((micros() - microsButtons) > BUTTON_READ_DELAY)
        { // Pulse latch
          sendLatch();

          for (uint8_t btn = 0; btn < buttonCount; btn++)
          {
            for (gp = 0; gp < GAMEPAD_COUNT; gp++)
              //(PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
              (PIND & gpBit[gp]) ? buttons_SNES[gp] &= ~btnBits_SNES[btn] : buttons_SNES[gp] |= btnBits_SNES[btn];

            sendClock();
          }

          // Check gamepad type

          for (gp = 0; gp < GAMEPAD_COUNT; gp++)
          {
            //controllerType[gp] = NES;
            if (controllerType[gp] == NES) {   // NES
              Serial.println("NES");
              bitWrite(buttons_SNES[gp], 5, bitRead(buttons_SNES[gp], 4));
              bitWrite(buttons_SNES[gp], 4, bitRead(buttons_SNES[gp], 6));
              buttons_SNES[gp] &= 0xC3F;
              //Serial.println(buttons_SNES[gp]);
            }
            else if (controllerType[gp] == NTT) // SNES NTT Data Keypad
            {
              buttons_SNES[gp] &= 0x3FFFFFF;
              Serial.println("NTT");
            }
            else // SNES Gamepad
            {
              Serial.println("SNES");
              buttons_SNES[gp] &= 0xFFF;
            }

          }

          for (gp = 0; gp < GAMEPAD_COUNT; gp++)
          {
            // Has any buttons changed state?
            if (buttons_SNES[gp] != buttonsPrev_SNES[gp])
            {
              Gamepad[gp]._GamepadReport_SNES.buttons = (buttons_SNES[gp] >> 4); // First 4 bits are the axes
              Gamepad[gp]._GamepadReport_SNES.Y = ((buttons_SNES[gp] & DOWN) >> 1) - (buttons_SNES[gp] & UP);
              Gamepad[gp]._GamepadReport_SNES.X = ((buttons_SNES[gp] & RIGHT) >> 3) - ((buttons_SNES[gp] & LEFT) >> 2);
              buttonsPrev_SNES[gp] = buttons_SNES[gp];
              Gamepad[gp].send();
              Serial.println("SNES");
            }
          }

          microsButtons = micros();
        }
      }

      break;

    case GENESIS_:
      while (1)
      {
        Serial.println("GENESIS");

        for (gp = 0; gp < GAMEPAD_COUNT; gp++) {


          controllers.readState();

          if (controllers.currentState[gp] != lastState[gp])
          {
            Gamepad[gp]._GamepadReport_GENESIS.buttons = controllers.currentState[gp] >> 4;
            Gamepad[gp]._GamepadReport_GENESIS.Y = ((controllers.currentState[gp] & SC_BTN_DOWN) >> SC_BIT_SH_DOWN) - ((controllers.currentState[gp] & SC_BTN_UP) >> SC_BIT_SH_UP);
            Gamepad[gp]._GamepadReport_GENESIS.X = ((controllers.currentState[gp] & SC_BTN_RIGHT) >> SC_BIT_SH_RIGHT) - ((controllers.currentState[gp] & SC_BTN_LEFT) >> SC_BIT_SH_LEFT);
            Gamepad[gp].send();
            if (Gamepad[gp]._GamepadReport_GENESIS.Y)
              Keyboard.write(KEY_F12);
            lastState[gp] = controllers.currentState[gp];
          }
        }
      }

      delay(10);

      break;

    case NEOGEO_:
      buttonsDirect[0] = 0;
      buttonsDirect[1] = 0;

      while (1)
      {
        Serial.println("NEOGEO");
        // Get current time, the millis() function should take about 2µs to complete
        millisNow = millis();

        myShifter.load();
        myInput0 = myShifter.getByte();
        myInput1 = myShifter.getByte();
        myInput2 = myShifter.getByte();

        //for(uint8_t i=0; i<10; i++) // One iteration (when debounce is enabled) takes approximately 35µs to complete, so we don't need to check the time between every iteration
        //{
        // Read axis and button inputs (bitwise NOT results in a 1 when button/axis pressed)
        axesDirect[0] = ~(reverse(myInput0 & B00001111));//~(PINF & B11110000);
        buttonsDirect[0] = ~((myInput0 & B11110000) >> 4 | (myInput1 & B00001111) << 4 | (B11111111 << 8)); //~((PIND & B00011111) | ((PIND & B10000000) << 4) | ((PINB & B01111110) << 4));

        axesDirect[1] = ~((reverse(myInput1 & B11110000)) << 4); //~(PINF & B11110000);
        buttonsDirect[1] = ~((myInput2) | (B11110000 << 4));//~((PIND & B00011111) | ((PIND & B10000000) << 4) | ((PINB & B01111110) << 4));

        //if(debounce)
        //  {
        //  // Debounce axes
        //  for(pin=0; pin<4; pin++)
        //  {
        //    // Check if the current pin state is different to the stored state and that enough time has passed since last change
        //    if((axesDirect & axesBits[pin]) != (axes & axesBits[pin]) && (millisNow - axesMillis[pin]) > DEBOUNCE_TIME)
        //    {
        //      // Toggle the pin, we can safely do this because we know the current state is different to the stored state
        //      axes ^= axesBits[pin];
        //      // Update the timestamp for the pin
        //      axesMillis[pin] = millisNow;
        //    }
        //  }

        // Debounce buttons
        //  for(pin=0; pin<12; pin++)
        //  {
        // Check if the current pin state is different to the stored state and that enough time has passed since last change
        //    if((buttonsDirect & buttonsBits[pin]) != (buttons & buttonsBits[pin]) && (millisNow - buttonsMillis[pin]) > DEBOUNCE_TIME)
        //    {
        // Toggle the pin, we can safely do this because we know the current state is different to the stored state
        //      buttons ^= buttonsBits[pin];
        // Update the timestamp for the pin
        //      buttonsMillis[pin] = millisNow;
        //    }
        //  }
        //  }
        //  else
        //  {

        for (gp = 0; gp < GAMEPAD_COUNT; gp++) {
          axes[gp] = axesDirect[gp];
          buttons_NG[gp] = buttonsDirect[gp];

          //}

          // Has axis inputs changed?
          if (axes[gp] != axesPrev[gp])
          {

            // UP + DOWN = UP, SOCD (Simultaneous Opposite Cardinal Directions) Cleaner
            if (axes[gp] & B10000000)
              Gamepad[gp]._GamepadReport_NEOGEO.Y = -1;
            else if (axes[gp] & B01000000)
              Gamepad[gp]._GamepadReport_NEOGEO.Y = 1;
            else
              Gamepad[gp]._GamepadReport_NEOGEO.Y = 0;
            // UP + DOWN = NEUTRAL
            //Gamepad._GamepadReport_NEOGEO.Y = ((axes & B01000000)>>6) - ((axes & B10000000)>>7);
            // LEFT + RIGHT = NEUTRAL
            Gamepad[gp]._GamepadReport_NEOGEO.X = ((axes[gp] & B00010000) >> 4) - ((axes[gp] & B00100000) >> 5);
            axesPrev[gp] = axes[gp];
            usbUpdate = true;
          }

          //if(axes2 != axesPrev2)
          //  {
          // UP + DOWN = UP, SOCD (Simultaneous Opposite Cardinal Directions) Cleaner
          //  if(axes2 & B10000000)
          //    Gamepad[1]._GamepadReport_NEOGEO.Y = -1;
          //  else if(axes2 & B01000000)
          //    Gamepad[1]._GamepadReport_NEOGEO.Y = 1;
          //  else
          //    Gamepad[1]._GamepadReport_NEOGEO.Y = 0;
          // UP + DOWN = NEUTRAL
          //Gamepad._GamepadReport_NEOGEO.Y = ((axes & B01000000)>>6) - ((axes & B10000000)>>7);
          // LEFT + RIGHT = NEUTRAL
          //  Gamepad[1]._GamepadReport_NEOGEO.X = ((axes2 & B00010000)>>4) - ((axes2 & B00100000)>>5);
          //  axesPrev2 = axes2;
          //  usbUpdate2 = true;
          //  }

          // Has button inputs changed?
          if (buttons_NG[gp] != buttonsPrev_NG[gp])
          {

            Gamepad[gp]._GamepadReport_NEOGEO.buttons = buttons_NG[gp];
            buttonsPrev_NG[gp] = buttons_NG[gp];
            usbUpdate = true;
          }

          ///if(buttons2 != buttonsPrev2)
          //  {
          //  Gamepad[1]._GamepadReport_NEOGEO.buttons = buttons2;
          //  buttonsPrev2 = buttons2;
          //  usbUpdate2 = true;
          //  }

          // Should gamepad data be sent to USB?
          if (usbUpdate)
          {

            //if(usbUpdate1){
            Gamepad[gp].send();


            usbUpdate = false;
            //}
            //  if(usbUpdate2){
            //  Gamepad[1].send();
            //  usbUpdate2 = false;
            //  }

#ifdef DEBUG
            sprintf(buf, "%06lu: %d%d%d%d", millisNow - millisSent, ((axes & 0x10) >> 4), ((axes & 0x20) >> 5), ((axes & 0x40) >> 6), ((axes & 0x80) >> 7) );
            Serial.print(buf);
            sprintf(buf, " %d%d%d%d", (buttons_NG & 0x01), ((buttons_NG & 0x02) >> 1), ((buttons_NG & 0x04) >> 2), ((buttons_NG & 0x08) >> 3) );
            Serial.println(buf);
            millisSent = millisNow;
#endif
          }
        }
      }
      break;

    case PSX_:
      while (1)
      {


        // Send data to USB if values have changed
        for (gp = 0; gp < GAMEPAD_COUNT; gp++)
        {
          buttons_PSX[gp] = 0;
          /*buttons_PSX[gp] = */Psx[gp].read(&data1, &data2, &data3, &data4, &data5, &data6, &mode);

          if (mode & 0xF0 == 0xF0) {
            Psx[gp].exitConfig(); //in config mode, I must exit from config!! ATTENTION: revise this when trying to setup rumble
          }
          /*else*/ if (mode == 0x41) {
            buttons_PSX[gp] = (data2 << 8) | data1; //Digital mode
          }
          else if (mode == 0x73 || mode == 0x53) { //Analogue modes

            buttons_PSX[gp] = (data2 << 8) | data1;//need to tell appart digital from analogue???

            horizontalR_PSX[gp] = data3;
            verticalR_PSX[gp] = data4;
            horizontalL_PSX[gp] = data5;
            verticalL_PSX[gp] = data6;
          }

          /*if (mode != prevmode){
            Psx[gp].rumble(mode);
            prevmode = mode;
            }*/

          // Has any buttons changed state?
          if (buttons_PSX[gp] != buttonsPrev_PSX[gp] || horizontalR_PSX[gp] != horizontalRPrev_PSX[gp] || verticalR_PSX[gp] != verticalRPrev_PSX[gp] || horizontalL_PSX[gp] != horizontalLPrev_PSX[gp] || verticalL_PSX[gp] != verticalLPrev_PSX[gp])
          {
            //Gamepad[gp]._GamepadReport_PSX.buttons = buttons_PSX[gp] >> 4;

            Gamepad[gp]._GamepadReport_PSX.buttons = (buttons_PSX[gp] >> 12 & B00001111) | (buttons_PSX[gp] >> 6 & B00110000) | (buttons_PSX[gp] >> 2 & B11000000) | (buttons_PSX[gp] & B10000000) << 1 | (buttons_PSX[gp] & B00010000) << 5 | (buttons_PSX[gp] & B01000000) << 4 | (buttons_PSX[gp] & B00100000) << 6;

            /*(buttons_PSX[gp]>>12 & B00001111) //botones
              (buttons_PSX[gp]>>6 & B00110000) //gatillos1
              (buttons_PSX[gp]>>2 & B11000000) //gatillos2
              (buttons_PSX[gp] & B10000000)<<1 //select
              (buttons_PSX[gp] & B00010000)<<5 //start
              (buttons_PSX[gp] & B01000000)<<4 //L3
              (buttons_PSX[gp] & B00100000)<<6 //R3
            */


            if (mode == 0x73 || mode == 0x53) //analog
            {
              byte up = (buttons_PSX[gp] & psxUp) >> 3;
              byte dw = (buttons_PSX[gp] & psxDown) >> 1;
              byte rg = (buttons_PSX[gp] & psxRight) >> 2;
              byte lf = (buttons_PSX[gp] & psxLeft);

              Gamepad[gp]._GamepadReport_PSX.Y = 255 - verticalL_PSX[gp];
              Gamepad[gp]._GamepadReport_PSX.X = 255 - horizontalL_PSX[gp];
              //Gamepad[gp]._GamepadReport_PSX.PoV = 7*lf_star+up_star + (3*rg_star+up_star)/(1+!(up_star^rg_star)) + (3*rg_star+5*dw_star)/(1+!(dw_star^rg_star)) + (7*lf_star+5*dw_star)/(1+!(dw_star^lf_star));
              Gamepad[gp]._GamepadReport_PSX.PoV = 8;
              if (up && lf) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 7;//B00001001;
              } else if (up && rg) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 1;//B00000011;
              } else if (dw && lf) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 5;//B00001100;
              } else if (dw && rg) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 3;//B00000110;
              } else if (up) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 0;//B00000001;
              } else if (lf) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 6;//B00001000;
              } else if (rg) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 2;//B00000010;
              } else if (dw) {
                Gamepad[gp]._GamepadReport_PSX.PoV = 4;//B00000100;
              }
              Gamepad[gp]._GamepadReport_PSX.Z = 255 - horizontalR_PSX[gp];
              Gamepad[gp]._GamepadReport_PSX.RZ = 255 - verticalR_PSX[gp];
            }
            else if (mode == 0x41) //digital
            {
              Gamepad[gp]._GamepadReport_PSX.Y = (((buttons_PSX[gp] & psxDown) >> 1) - ((buttons_PSX[gp] & psxUp) >> 3)) * 127 + 127;
              Gamepad[gp]._GamepadReport_PSX.X = (((buttons_PSX[gp] & psxRight) >> 2) - (buttons_PSX[gp] & psxLeft)) * 127 + 127;

              Gamepad[gp]._GamepadReport_PSX.PoV = 8;
              Gamepad[gp]._GamepadReport_PSX.Z = 127;
              Gamepad[gp]._GamepadReport_PSX.RZ = 127;
            }

            buttonsPrev_PSX[gp] = buttons_PSX[gp];
            horizontalRPrev_PSX[gp] = horizontalR_PSX[gp];
            verticalRPrev_PSX[gp] = verticalR_PSX[gp];
            horizontalLPrev_PSX[gp] = horizontalL_PSX[gp];
            verticalLPrev_PSX[gp] = verticalL_PSX[gp];
            Gamepad[gp].send();
          }
        }
      }
      break;

  }
}

void sendLatch()
{
  // Send a latch pulse to the NES controller(s)
  PORTD |=  B00100000; // Set HIGH
  if (SISTEMA == NES) delayMicroseconds(MICROS_LATCH_NES);
  else if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_LATCH);
  PORTD &= ~B00100000; // Set LOW
  if (SISTEMA == NES) delayMicroseconds(MICROS_PAUSE_NES);
  if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_PAUSE);
}

void sendClock()
{
  // Send a clock pulse to the NES controller(s)
  PORTD |=  B10010000; // Set HIGH
  if (SISTEMA == NES) delayMicroseconds(MICROS_CLOCK_NES);
  if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_CLOCK);
  PORTD &= ~B10010000; // Set LOW
  if (SISTEMA == NES) delayMicroseconds(MICROS_PAUSE_NES);
  if (SISTEMA == SNES) DELAY_CYCLES(CYCLES_PAUSE);
}


//SNES
void detectControllerTypes()
{
  uint8_t buttonCountNew = 0;

  // Read the controllers a few times to detect controller type
  for (uint8_t i = 0; i < 40; i++)
  {
    // Pulse latch
    sendLatch();

    // Read all buttons
    for (uint8_t btn = 0; btn < buttonCount; btn++)
    {
      for (gp = 0; gp < GAMEPAD_COUNT; gp++)
        //(PINF & gpBit[gp]) ? buttons[gp] &= ~btnBits[btn] : buttons[gp] |= btnBits[btn];
        (PIND & gpBit[gp]) ? buttons_SNES[gp] &= ~btnBits_SNES[btn] : buttons_SNES[gp] |= btnBits_SNES[btn];
      sendClock();
    }

    // Check controller types and set buttonCount to max needed
    for (gp = 0; gp < GAMEPAD_COUNT; gp++)
    {
      if ((buttons_SNES[gp] & 0xF3A0) == 0xF3A0) {  // NES
        if (controllerType[gp] != SNES && controllerType[gp] != NTT)
          controllerType[gp] = NES;
        if (buttonCountNew < 8)
          buttonCountNew = 8;
      }
      else if (buttons_SNES[gp] & NTT_CONTROL_BIT) { // SNES NTT Data Keypad
        controllerType[gp] = NTT;
        buttonCountNew = 32;
      }
      else {                                   // SNES Gamepad
        if (controllerType[gp] != NTT)
          controllerType[gp] = SNES;
        if (buttonCountNew < 12)
          buttonCountNew = 12;
      }
    }
  }

  // Set updated button count to avoid unneccesary button reads (for simpler controller types)
  buttonCount = buttonCountNew;

}
