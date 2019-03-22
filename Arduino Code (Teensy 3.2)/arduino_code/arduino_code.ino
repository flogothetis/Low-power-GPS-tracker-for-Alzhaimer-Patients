


/*
 * Alzhaimer Finder
 * Project in Embedded Systems
 * Authors :
 * Logothetis Fragkoulis
 * Apostolos Bailakis
 *
 * Electrical and Computer Engineering
 * Technical University of Crete
 * Department :HMMY
 * Lesson: Embedded -Cyber Phusical Systems
 *
 

************************************************************************************************
 * For this project we used Teensy 3.2 Development Bord ,which it has 3
 * hardware serial inputs/outputs  using UART (Universal Asychronous Receiver Trasmitter).
 * We used RX1,TX1( Pins 0 ,1) and RX2,TX2 (Pins 9,10).
 *
 * We Control each devise with the adove:
 * RX1,TX1( Pins 0 ,1)-->Gsm Sim900
 *
 * RX2,TX2 (Pins 9,10)--->Gps GY-NEO6MV2
 *
 *
 * ***********************************************************************************************
 * 
 
*/
//___________________________________Libraries and Member variables_________________________//
//Library for GPS module
#include <TinyGPS++.h>
//Library to handle strings
#include <stdlib.h>
//Global member variables

char incoming_char;
TinyGPSPlus gps;
int counter=0;
const int GPSen = 5 ;
const int ledPin = 13;
String textForSMS;
double lat_gps,lng_gps ;
int val;
float input_voltage = 0.0;

char key_char ='@';
int terminal_cond=56;

//Sampling Period
unsigned long sampling_period = 3600000;
unsigned long sampling_period_in_low_battery = 1600000;
//Current Milliseconds
unsigned long present_millis=millis();


//___________________________________Arduino Setup Function________________________________//
void setup()
{

  //Teensy 3.2 works great with usb baudrates

  Serial.begin(38400);
  //Default Baudrate for GPS

  Serial2.begin(9600);
  //Baudrate of SIM900

  Serial1.begin(19200);
  //Enable the GPS with that PIN

  pinMode(GPSen, OUTPUT);
  //Open the LED when the process in running

  pinMode(ledPin, OUTPUT);
  pinMode(6, OUTPUT);
  //Power in the SIM900

  SIM900power();
  
}
//___________________________________Arduino Loop Function__________________________________//
void loop()
{
  

  while(GsmHasNewData() || (millis() - present_millis >= sampling_period ) || (millis()-present_millis <0 ))
   {
      //Auto-sampling
      if((millis() - present_millis >= sampling_period ))
      {
        present_millis=millis();
      }
      if((millis()-present_millis <0 ))
      {
        present_millis=millis();
        continue;
      }
    
    
      //Enable the GPS
      EnableGps();
      
      counter=0;
     
      while(GpsHasNewData())
     {
        GetGpsData();
        counter++;
       if( counter > terminal_cond) break;
        
     }
  
      DisableGps();
      ReadBattery();
      SendSms(CreateSms());
     
   }
    //Change the sampling rate when the battery is low 
    if(input_voltage < 50)
      sampling_period= sampling_period_in_low_battery ;
}



bool GsmHasNewData()
{
  if( Serial1.available()>0 )
  {
    incoming_char=Serial1.read();
    Serial.print(incoming_char);
    //Recognize the key ('@') to enable the locating process
    if(incoming_char==key_char) return true;
  }
  return false;
}



void DisableGps()
{
digitalWrite(GPSen, HIGH);
digitalWrite(ledPin, LOW);    // set the LED off
}


void EnableGps()
{
    //Enabe the GPS

      digitalWrite(GPSen, LOW);
    // set the LED on

      digitalWrite(ledPin, HIGH);
    //Cold Start of the GPS

      delay(17000);

      
      Serial.println(counter);

}




double ReadBattery()
{

 //Read the batteries level
          int val;
          val = analogRead(0);
          Serial.print("analog 0 is: ");
          Serial.println(val);
          
          input_voltage = ((1024.0-(float)val) / 1024.0)*100;
          input_voltage = 100.0-input_voltage;
          return input_voltage;
          
}

String CreateSms()
{
  //Prepare the message for sending.

  String textForSMS = "Alzhaimer Finder: http://www.google.com/maps/place/";
  char arg1[1];
  dtostrf(lng_gps, 4, 6, arg1);
  textForSMS.concat(arg1);
  textForSMS = textForSMS + ",";
   char  arg2[1];
  dtostrf(lng_gps, 4, 6, arg2);
  textForSMS.concat(arg2);

  textForSMS = textForSMS + "  **Battery  : ";
  textForSMS.concat(input_voltage);
  textForSMS = textForSMS + " %";
   Serial.println("");
  Serial.println(textForSMS);
  return textForSMS;

}





bool GpsHasNewData()
{

    if(Serial2.available() > 0 )
    {
      return true;
    }
     return false;
}



void GetGpsData()
{
  
    if (gps.encode(Serial2.read())) {
    //Read the GPS longitude and latitude
     Serial.print(F("Location: "));
    if (gps.location.isValid())
    {
      Serial.print(gps.location.lat(), 6);
      lat_gps=(gps.location.lat());
      Serial.print(F(","));
      Serial.print(gps.location.lng(), 6);
      lng_gps=(gps.location.lng());
    }
    else
    {
      lat_gps=0.000;
      lng_gps=0.000;
      Serial.print(F("INVALID"));
    }

    Serial.print(F("  Date/Time: "));
    if (gps.date.isValid())
    {
      Serial.print(gps.date.month());
      Serial.print(F("/"));
      Serial.print(gps.date.day());
      Serial.print(F("/"));
      Serial.print(gps.date.year());
    }
    else
    {
      Serial.print(F("INVALID"));
    }

    Serial.print(F(" "));
    if (gps.time.isValid())
    {
      if (gps.time.hour() < 10) Serial.print(F("0"));
      Serial.print(gps.time.hour());
      Serial.print(F(":"));
      if (gps.time.minute() < 10) Serial.print(F("0"));
      Serial.print(gps.time.minute());
      Serial.print(F(":"));
      if (gps.time.second() < 10) Serial.print(F("0"));
      Serial.print(gps.time.second());
      Serial.print(F("."));
      if (gps.time.centisecond() < 10) Serial.print(F("0"));
      Serial.print(gps.time.centisecond());
    }
    else
    {
      Serial.print(F("INVALID"));
    }

    Serial.println();
    }
}


//__________Software equivalent of pressing the GSM shield "power" button_____//
void SIM900power()
{
digitalWrite(6, HIGH);
delay(1000);
digitalWrite(6, LOW);
delay(7000);
}


///___________________________________Sms Sent____________________________//

void SendSms(String message)
{
// AT command to send SMS message
Serial1.print("AT+CMGF=1\r");
delay(100);
// recipient's mobile number, in international format
Serial1.println("AT + CMGS = \"+306936460870\"");
delay(100);
// message to send
Serial1.println(message);
delay(100);
// End AT command with a ^Z, ASCII code 26
Serial1.println((char)26);
delay(100);
Serial1.println();
delay(5000);
}




