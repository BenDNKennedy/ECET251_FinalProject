/*
 ECET 251 - RF 3D Printed Electronic Boat System [Receiver Code]
 
 This code receives the transmitted data, decides what the data is for
 and acts accordingly. The 3 types of data regard the motor speed, the 
 servo angle, and the checksum.
 
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
 15 Apr 2021

*/


//libraries
#include <Manchester.h>
#include <Servo.h>

//defining pins
#define rx_pin 2
#define enA 10
#define servoPin 3 
#define in1 6
#define in2 7
#define ChkSumPassLED 11
#define ChkSumFailLED 12


//creating servo object
Servo Servo1;

void setup() {
  //setting motors pin states
  pinMode(enA, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  //clockwise rotation
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);

  //servo setup
  Servo1.attach(servoPin);

  //setup manchester receiving
  man.setupReceive(rx_pin, MAN_1200);
  man.beginReceive();

  //reciever sucessfully initialized
  Serial.begin(9600);
  Serial.println("Receiver is initialized");
}

void loop() {

    //when a message is received
    if(man.receiveComplete()) 
    {
      int pwmData;
      int servoData;
      int Calc_ChkSum;
      int Exp_ChkSum;
      int Calc_ChkSumDiff;
      int servoOutput;
      int Exp_ChkSumScaled;
      
      //received message = data_received; begin receiving again
      uint16_t data_received = man.getMessage();
      man.beginReceive();

      //if the received data is between 0-80 (motor)..
      if (data_received <= 80)
      {  
        //configure the pwm for the motor using the received data
        pwmData = data_received;
        
        int pwmOutput = map(pwmData, 0, 80, -255, 255);

        //ignore negative y-axis readings
        if (pwmOutput < 1)
        {
          pwmOutput = 0;
        }

        //write the PWM to the motor
        //Serial.println(pwmOutput);
        analogWrite(enA, pwmOutput);              
      }
      
      //if the received data is between 81-171 (servo)..
      if (data_received > 80 && data_received <= 171)
      {
        //configure the PWM for the servo using the received data
        servoData = data_received;

        servoOutput = map(servoData, 81, 161, 0, 180);

        //if the servo angle is near 90, set it to 90 (pot error)
        if (servoOutput > 85 && servoOutput< 95)
        {
          servoOutput = 90;
        }

        //write the angle to the servo
        Servo1.write(servoOutput);
        //Serial.println(servoOutput);
      }

      //if the received data is between 161-241 (checksum)
      if (data_received > 161 && data_received <= 241)
      {
        //set the expected checksum value to the data that was just received
        Exp_ChkSum = data_received;

        //calculate the current checksum using the pwm and servo data
        Calc_ChkSum = map((pwmData+servoData), 0, 241, 161, 241);

        //reset checksum LEDS
        digitalWrite(ChkSumFailLED, LOW);
        digitalWrite(ChkSumPassLED, LOW);

        //if the expected and calculated checksums are the same..
        if (Exp_ChkSum == Calc_ChkSum)
        {
          //blink "pass" LED
          delay(15);
          digitalWrite(ChkSumPassLED, HIGH);
          delay(15);
        }

        //if they aren't..
        else
        {
          //blink "fail" LED
          delay(15);
          digitalWrite(ChkSumFailLED, HIGH);
          delay(15);
        }
      }
   }
}
