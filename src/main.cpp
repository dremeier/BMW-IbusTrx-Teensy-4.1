/*
example IBUS message:
50 04 68 32 11 1F (volume up button pressed on the steering wheel)
|  |  |  |  |  | 
|  |  |  |  |  checksum (xorsum of all previous bytes)
|  |  |  |  one or more data fields
|  |  |  message type/command type
|  |  destination address
|  length of message (including destination address and checksum)
source address 
*/
// include the IbusTrx library
#include <IbusTrx.h>
#include "IbusCodes.h"      // hier sind alles Ibus-Codes und Variablen zur Configuration
// create a new IbusTrx instance
IbusTrx ibusTrx;


/*#####################################################
################ SETUP ################################
#######################################################*/
void setup(){
  pinMode(ledPin, OUTPUT);
  pinMode(senSta, INPUT_PULLUP);                          // pin 9 des TH3122 für Clear to send, mit einer diode von Teensy pin > TH3122 pin9
  
  debugSerial.begin(115200);                              // Teensy USB port, nicht nötig
  ibusTrx.begin(ibusPort);                                // Hardware Serial Nr. an IbusTrx übergeben
  
  ibusTrx.senStapin(senSta);                              // senSta Pin an IbusTrx Library übergeben
  attachInterrupt(digitalPinToInterrupt(senSta), ClearToSend, FALLING);
  delay (1000);
  
  debugSerial.println(F("-- IBUS trx on Teensy 4.x --"));
}



/*#####################################################
################ LOOP #################################
#######################################################*/
void loop(){
  // available() has to be called repeatedly, with no delay() in between
  // this function returns true if a new message is available for reading
  bool messageWaiting = ibusTrx.available();

  // if there's a message waiting, check it out
  if (messageWaiting) {
    // read the incoming message (this copies the message and clears the receive buffer)
    IbusMessage message = ibusTrx.readMessage();

    // every module on the IBUS has its own 8-bit address.
    // the following addresses are defined in the IbusTrx library:
    // M_GM5: body control module
    // M_DIA: diagnostic computer
    // M_EWS: immobilizer
    // M_MFL: steering wheel controls
    // M_IHKA: climate control panel
    // M_RAD: radio module
    // M_IKE: instrument cluster
    // M_ALL: broadcast message
    // M_TEL: telephone module
    // M_LCM: light control module
    
    // these two functions return the source and destination addresses of the IBUS message:
    unsigned int messageSource = message.source();
    unsigned int messageDestination = message.destination();


    // trigger based on a message from the steering wheel controls
    if (messageSource == 0xF0) {
      
      // if "Push right" is pressed: simulate a press of the dome light button
      if (message.b(1) == 0x05) {
        // write the message to the transmit buffer
        ibusTrx.writeTxt("Tschuess Andre");
      }
    }

    // filtering example: 
    // in this case we're only interested in messages sent by the steering wheel controls to the radio
    if (messageSource == M_MFL && messageDestination == M_RAD) {
      // the length of the message payload, including the checksum
      // this function is rarely needed, 
      // in most cases the number of payload fields is already known based on the type of message
      unsigned int messageLength = message.length();
      //debugSerial.print(F("Length "));
      //debugSerial.println(messageLength);
      // the b(n) function returns the n'th byte of the message payload
      // b(0) will return the first byte, b(1) returns the second byte, etc.
      unsigned int messageCommand = message.b(0); // the first byte usually identifies what type of message it is
      
      // command 0x32 happens to be related to the volume controls
      if (messageCommand == 0x32) {
        // in this case, the least significant bit of the second payload byte tells us whether this is a "volume up" or a "volume down" instruction
        // several fields are often packed into a single byte, playing around with bitwise operators is recommended when working with IBUS data
        if (message.b(1) & 0x01){
          // volume up pressed, turn LED on
          digitalWrite(13, HIGH);
        }
        else{
          // volume down pressed, turn LED off
          digitalWrite(13, LOW);
        }
      }
      // etc.
    }
    // etc.
  }
  // rest of your program goes here...

  // remember to never use a blocking function like delay() in your program,
  // always use millis() or micros() if you have to implement a delay somewhere
}





// Globale Funktion außerhalb der Klasse
void ClearToSend() {
  //debugSerial.print(" Interrupt ");
  ibusTrx.available(); // Versuch, eine Nachricht zu senden, wenn Interrupt ausgelöst wird
}
