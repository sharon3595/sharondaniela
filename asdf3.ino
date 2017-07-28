#include "SIM900.h"  //sim900 library
#include <TinyGPS++.h> //GPS library
#include <SoftwareSerial.h> //Arduino included library
#include "inetGSM.h" //POST GET HTTP protocol library
//#include "sms.h"  // SMS Protocol library
//#include "call.h" // Making calls library

InetGSM inet;  //set Inet to use on GSM
//CallGSM call;
//SMSGSM sms;

static const int RXPin1 = 50, TXPin1 = 51;  //Set Rx and Tx for GPS comm
static const uint32_t GPSBaud = 9600; // baud rate for GPS and Arduino
TinyGPSPlus gps; // Set gps variable to use on the sketch
SoftwareSerial ss(RXPin1, TXPin1); //set ss variable as serial port
char HTTP_BODY[100];  //Body of the JSON variables
char msg[50]; // Body of the JSON header
int numdata; //inet buffer for HTTP_BODY and msg
char inSerial[50]; //numdata number of chars
int i=0; // POST or GET variable
boolean started=false; //connection state

char part1[20] = "{\"latitude\": "; //first part of the JSON http body
char part2[20] = ", \"longitude\": "; //second part of the JSON http body
char bufflat[10]; //buffer for convertion between float to char
char bufflon[10]; //buffer for convertion between float to char
int cont=1; //sending state

void setup() 
{
  Serial.begin(9600); //Serial connection.
  ss.begin(GPSBaud);  // GPS serial connection
  Serial.println("GSM TRACKING SYSTEM."); // Print 
  //Start configuration of shield with baudrate.
  //For http uses is raccomanded to use 4800 or slower.
  if (gsm.begin(4800)){
    Serial.println("\nstatus=READY");
    started=true;  
  }
  else Serial.println("\nstatus=IDLE");
  
  
}

void loop() 
{
  if(started){
    //GPRS attach, put in order APN, username and password.
    //If no needed auth let them blank.
    if (inet.attachGPRS("", "", ""))
      Serial.println("status=ATTACHED");
    else Serial.println("status=ERROR");
    delay(1000);
    
    //Read IP address.
    gsm.SimpleWriteln("AT+CIFSR");
    delay(5000);
    //Read until serial buffer is empty.
    gsm.WhileSimpleRead();
  while (ss.available() > 0 && cont==1)  //if connection with GPS is ok 
  if (gps.encode(ss.read()))
  {
  if (gps.location.isValid())
  { 
    Serial.println(dtostrf(gps.location.lat(), 5, 5, bufflat)); //convertion from gps float to char
    Serial.println(dtostrf(gps.location.lng(), 5, 5, bufflon)); //convertion from gps float to char
    sprintf(HTTP_BODY, "%s%s%s%s}",part1,dtostrf(gps.location.lat(), 5, 5, bufflat),part2,dtostrf(gps.location.lng(), 5, 5, bufflon)); //MAKE JSON BODY
    Serial.println(HTTP_BODY);
    numdata=inet.httpPOST("200.110.56.222", 8080, "/api/v1/gpstracker/telemetry", HTTP_BODY, msg, 50); // POST JSON HTTP BODY
    //Print the results.
    Serial.println("\nNumber of data received:");
    Serial.println(numdata);  // No of Data received from the server
    Serial.println("\nData received:");
    Serial.println(msg); //Data received from the server
    cont=0;
  }
  else
  {
    //if theres no valid connection between GPS and Arduino 
    Serial.println(dtostrf(gps.location.lat(), 5, 5, bufflat));
    Serial.println(dtostrf(gps.location.lng(), 5, 5, bufflon));
    Serial.println(gps.date.year());
  }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    // if the GPS is no plugged correctly
    Serial.println(F("No GPS detected: check wiring."));
    while(true);
  }

  }
  delay(15000);
  cont=1;
}


