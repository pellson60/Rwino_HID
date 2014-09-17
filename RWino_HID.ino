/*
Example for RWino_HID withe MPR121 touch sensor IC and Grove_RGB_LCD

 by: Bläckbild, based on the MPR121 Keypad Example by Jim Lindblom
     
 SparkFun Electronics
 created on: 5/12/11
 license: OSHW 1.0, http://freedomdefined.org/OSHW
 
 Pressing a pad will print the corresponding number.
 
 Hardware: 3.3V or 5V Arduino

 Notes: The Wiring library is not used for I2C on MPR121, a default atmel I2C lib
        is used. Be sure to keep the .h files with the project. 
*/

// include the atmel I2C libs
#include "mpr121.h"
#include "i2c.h"
#include "Wire.h"
#include "rgb_lcd.h"

// Match key inputs with electrode numbers
#define OK   3
#define C    2
#define UP   1
#define DOWN 0

//RGB LCD
rgb_lcd lcd;

//LCD Back light - RWino White
const int colorR = 200;  
const int colorG = 235;
const int colorB = 100;

//interupt pin
int irqpin = 2;  // D2 - int0
char digits;

void setup()
{
      // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    
    lcd.setRGB(colorR, colorG, colorB);
    
    // Print a message to the LCD.
    lcd.print("Hello, RWino!");

  //make sure the interrupt pin is an input and pulled high
  pinMode(irqpin, INPUT);
  digitalWrite(irqpin, HIGH);
  
  //configure serial out
  Serial.begin(9600);
  
  digits = '0';  // Start value for button digits
  
  
  //Setup I2C bus
  //output on ADC4 (PC4, SDA)
  DDRC |= 0b00010011;
  // Pull-ups on I2C Bus
  PORTC = 0b00110000; 
  // initalize I2C bus. Wiring lib not used. 
  i2cInit();
  
  delay(100);
  // initialize mpr121
  mpr121QuickConfig();
  
    // Setup the GPIO pins to drive LEDs
  mpr121Write(GPIO_EN, 0xFF);       // 0x77 is GPIO enable
  mpr121Write(GPIO_DIR, 0xFF);      // 0x76 is GPIO Dir
  mpr121Write(GPIO_CTRL0, 0xFF);    // Set to LED driver
  mpr121Write(GPIO_CTRL1, 0xFF);    // GPIO Control 1
  mpr121Write(GPIO_CLEAR, 0xFF);    // GPIO Data Clear
  mpr121Write(PWM_0, 0xA4);         // PWM_0 dimm LED 0 & 1
  mpr121Write(PWM_1, 0x88);         // PWM_0 dimm LED 2 & 3
  
  // Blink all LEDs to begin
 for (int i = 0; i < 2; i++)
  {
     mpr121Write(GPIO_SET, 0xFF);
    delay(200);
    mpr121Write(GPIO_CLEAR, 0xFF);
    delay(200);
    
  }
 // Create and interrupt to trigger when a button
  // is hit, the IRQ pin goes low, and the function getNumber is run. 
  attachInterrupt(0,getNumber,LOW);
  
    // prints 'Ready...' when you can start hitting numbers
  Serial.println("Ready...");
}

void loop()
{
 
  if (digits > '!'){
     // set the cursor to column 0, line 1
    // (note: line 1 is the second row, since counting begins with 0)
   lcd.setCursor(0, 1);
    // print the button number:
   lcd.print(digits);
    digits = '!'; // Reset Electrod pressed status 
  }

}

//////////////// Interrupt rutin for touch and relas of electrod
void getNumber()
{
  int touchNumber = 0;
  uint16_t touchstatus;
  
  touchstatus = mpr121Read(0x01) << 8;
  touchstatus |= mpr121Read(0x00);
  
  for (int j=0; j<4; j++)  // Check how many electrods were pressed
  {
    if ((touchstatus & (1<<j)))
      touchNumber++;
  }
  
  if (touchNumber == 1) // Touch of electrod
  {
    if (touchstatus & (1<<OK)){
      digits = '*';
      mpr121Write(GPIO_SET, 0x02);}
    else if (touchstatus & (1<<DOWN)){
      digits = '>';
      mpr121Write(GPIO_SET, 0x04);}
    else if (touchstatus & (1<<UP)){
      digits = '<';
      mpr121Write(GPIO_SET, 0x04);}
    else if (touchstatus & (1<<C)){
      digits = 'C';
     mpr121Write(GPIO_SET, 0x01);}
    }
  if (digits == '!'){ // Relas of electrod
  mpr121Write(GPIO_CLEAR, 0xFF);
  }
}

