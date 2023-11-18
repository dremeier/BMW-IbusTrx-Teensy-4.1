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
  attachInterrupt(digitalPinToInterrupt(senSta), ClearToSend, FALLING);   // Interrupt: Wenn senSta LOW wird, gehe zu Funktion ClearToSend
  delay (1000);
  
  debugSerial.println(F("-- IBUS trx on Teensy 4.x --"));
}



/*#####################################################
################ LOOP #################################
#######################################################*/
void loop()
{
  // available() has to be called repeatedly, with no delay() in between
  // this function returns true if a new message is available for reading
  bool messageWaiting = ibusTrx.available();

  // if there's a message waiting, check it out
  if (messageWaiting) 
  {
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
    unsigned int source = message.source();
    unsigned int destination = message.destination();

    // Funkschlüssel auf und zu
    if ((source == 0x00) && (destination == 0xBF) && (message.b(0) == 0x72))
    {             // 00 04 BF 72 xx 
      if (message.b(1) == 0x22) {
        ibusTrx.writeTxt("einsteigen Cordula");
      }
      if (message.b(1) == 0x26) {                       // Funkschlüssel Andre auf
        ibusTrx.writeTxt("einsteigen Andre");
      }
      if (message.b(1) == 0x12) {
        ibusTrx.writeTxt("Tschuess Cordula");
      }
      if (message.b(1) == 0x16) {                       // FunkSchlüssel Andre zu
        ibusTrx.writeTxt("Tschuess Andre");
      }
      // goto Heimleuchten: LDR auslesen und bei Dunkelheit Licht einschalten
    }
    /* // Alternative:
    if(memcmp_P(packet, Key1FunkAuf, 6) == 0 ) {
      ibusTrx.writeTxt("einsteigen Andre");
    }  */

    // Schlüssel im Schloß
    if ((source == 0x44) && (destination == 0xBF) && (message.b(0) == 0x74))
    {
      if (message.b(1) == 0x05)                                     // 44 05 bf 74 05 xx    Motor aus
      {               
        if (message.b(2) == 0x00)
        {
          ibusTrx.writeTxt("bis bald Cordula");                      // schlüssel stellung 1 -> 0, Corula
        }
        if (message.b(2) == 0x05) 
        {
          ibusTrx.writeTxt("bis bald Andre");                        // schlüssel stellung 1 -> 0, Corula
        }
      // goto Heimleuchten: LDR auslesen und bei Dunkelheit Licht einschalten
      }
      if (message.b(1) == 0x04)                                     // 44 05 bf 74 04 xx   Schlüssel wird ins Schloß gesteckt
      {               
        if (message.b(2) == 0x00)
        {
          ibusTrx.writeTxt("gute Fahrt Cordula");                     //  Corula
          IKEclear=true;                                             // bereit um den Text im IKE zu löschen
          msTimer=millis();                                          // aktuelle Zeit in den Timer legen
        }
        if (message.b(2) == 0x05) 
        {
          ibusTrx.writeTxt("gute Fahrt Andre");                       // Andre
          IKEclear=true;                                             // bereit um den Text im IKE zu löschen
          msTimer=millis();                                          // aktuelle Zeit in den Timer legen
        }
      } 
    }



    // trigger based on a message from the steering wheel controls
    if (source == 0xF0) 
    {
      // if "Push right" is pressed: simulate a press of the dome light button
      if (message.b(1) == 0x05) 
      {
        // write the message to the transmit buffer
        ibusTrx.writeTxt("Tschuess Andre");
      }
    }

    // filtering example: 
    // in this case we're only interested in messages sent by the steering wheel controls to the radio
    if (source == M_MFL && destination == M_RAD) {
      // the length of the message payload, including the checksum
      // this function is rarely needed, 
      // in most cases the number of payload fields is already known based on the type of message
    //unsigned int length = message.length();
      //debugSerial.print(F("Length "));
      //debugSerial.println(messageLength);

      // the b(n) function returns the n'th byte of the message payload
      // b(0) will return the first byte, b(1) returns the second byte, etc.  
      // command 0x32 happens to be related to the volume controls
      if (message.b(0) == 0x32) {
        // in this case, the least significant bit of the second payload byte tells us whether this is a "volume up" or a "volume down" instruction
        // several fields are often packed into a single byte, playing around with bitwise operators is recommended when working with IBUS data
        // prüft, ob das letzte Bit des zweiten Bytes (b(1)) des Objekts message gesetzt ist. Der Ausdruck & 0x01 verwendet die bitweise AND-Operation mit der Maske 0x01, um nur das letzte Bit zu überprüfen. 
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
  } // ############################ if (messageWaiting) ENDE #################################
  





  // ############################# weiter im Loop ##############
  // Text im IKE löschen
  if (IKEclear)                               // bereit um den Text im IKE zu löschen
  {                                  
    if ((millis()-msTimer) >= t_clearIKE)     // Zeit abgelaufen?
    {      
      ibusTrx.write(cleanIKE);                // lösche den Text
      IKEclear=false;                         // Fertig
    }
  }
}


// Globale Funktion außerhalb der Klasse
void ClearToSend() {
  //debugSerial.print(" Interrupt ");
  ibusTrx.available(); // Versuch, eine Nachricht zu senden, wenn Interrupt ausgelöst wird
}


