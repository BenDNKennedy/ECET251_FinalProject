/*
 ECET 251 - Wireless 3D Printed Electric Printed Boat System [Transmit Version]
 
 This program displays motor speed and rudder angle on the LCD. 
 It encrypts the data with Manchester and sent it to the receiving side.
 
  The circuit:
 * TX 5V pin to 5V
 * TX GND pin to common ground
 * TX Data pin to digital pin 2
 * LCD RS pin to digital pin 3
 * LCD Enable pin to digital pin 4
 * LCD D4 pin to digital pin 5
 * LCD D5 pin to digital pin 6
 * LCD D6 pin to digital pin 7
 * LCD D7 pin to digital pin 8
 * LCD R/W pin to ground
 * LCD VSS pin to ground
 * LCD VCC pin to 5V
 * LCD A pin with 10k Ohms to 5V
 * LCD K pin to ground
 * Wiper to LCD VO pin (pin 3)
 * Joystick 5V pin to 5V
 * Joystick X-axis to A1
 * Joystick Y-axis to A0
 * Joystick GND pin to ground
 
 by Ben Kennedy and Patrick Espallardo
 16 Apr 2021

*/

//libraries
#include <Manchester.h>
#include <Servo.h>
#include <LiquidCrystal.h>

//defining pins
#define tx_pin 2                                    
#define motorPin A0
#define servoPin A1                                   

//lcd
const int rs = 3, en = 4, d7 = 5, d6 = 6, d5 = 7, d4 = 8;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
  //setting the button pin to an input
  pinMode(button, INPUT);                           

  lcd.begin(16, 2);

  //setting up transmit using manchester libraries
  man.setupTransmit(tx_pin, MAN_1200);              
  
  //transmitter successfully initialized                                
  Serial.begin(9600);
  Serial.println("Transmitter is initialized");
}

void loop() {  
  //setting up the pot pin and scaling the value range down
  int speed_potValue = analogRead(motorPin);
  int direction_potValue = analogRead(servoPin);
                      
  int pwmOutput = map(speed_potValue, 0, 1023, 0, 80);
  int servoOutput = map(direction_potValue, 0, 1023, 81, 161);
  
  int chkSum_Calc = pwmOutput + servoOutput;
  int chkSumOutput = map(chkSum_Calc, 0, 241, 161, 241);

  //ignore negative motor values
  int lcdMotorStatus = map(speed_potValue, 0, 1023, -100, 100);
  if (lcdMotorStatus < 0)
  {
    lcdMotorStatus = 0;
  }

  //round values for pot innacuracy
  int lcdServoStatus = map(direction_potValue, 0, 1023, 0, 180);
  if (lcdServoStatus > 87 && lcdServoStatus < 93)
  {
    lcdServoStatus = 90;
  }
  
  //Set the position and text in the LCD
  lcd.clear();

  lcd.setCursor(0,0);
  lcd.print("Motor (%): ");
  lcd.setCursor(13,0);
  lcd.print(lcdMotorStatus);

  lcd.setCursor(0, 1);
  lcd.print("Servo (deg): ");
  lcd.setCursor(13,1);
  lcd.print(lcdServoStatus);
  
  //compilation of data to be transmitted
  int data_to_transmit[] = {pwmOutput,servoOutput, chkSumOutput}; 
  //amount of individual items (for ease) 
  char data_to_transmit_SIZE = 3;                   

  //transmit and print the data with a 10ms delay between items
  for (char i = 0; i < data_to_transmit_SIZE; i++)  
  {                                                 
    man.transmit(data_to_transmit[i]);              
    Serial.println(data_to_transmit[i]);
    delay(10);
  }
}
