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
    //unsigned int length = message.length();

    // Funkschlüssel auf und zu // 00 04 BF 72 xx 
    if ((source == M_GM5) && (destination == M_ALL) && (message.b(0) == 0x72))
    {             
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

    // Schlüssel im Schloß // 44 05 bf 74 xx xx
    if ((source == M_EWS) && (destination == M_ALL) && (message.b(0) == 0x74))
    {
      if (message.b(1) == 0x05)                                     // 44 05 bf 74 05 xx    Motor aus
      {               
        if (message.b(2) == 0x00)
        {
          ibusTrx.writeTxt("bis bald Cordula");                      // schlüssel stellung 1 -> 0, Corula
        }
        if (message.b(2) == 0x05) 
        {
          ibusTrx.writeTxt("bis bald Andre");                        // schlüssel stellung 1 -> 0, Andre
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

    // wenn tippblinken true dann:
    // Blinker: Wenn Blinkerpfeil im IKE links: D0 07 BF 5B 20 00 04 00 17 oder Blinkerpfeil im IKE rechts:  D0 07 BF 5B 40 00 04 00 77
    // D0 07 BF 5B 40 00 04 00 77
    //              |  |  |  |  |
    //              |  |  |  |  Checksum
    //              |  |  |Blinkstatus ist immer 04 , databytes[3]
    //              |  |nil
    //              |Blinkbyte kann sein: links-> 20, 21, 23, 27, 2B, 33, 3B, rechts-> 40, 41, 43, 47, 4B, 53, 5B
    if (Tippblinken == 1) 
    {           
      if ((source == M_LCM) && (destination == M_ALL) && (message.b(0) == 0x5B))      // Abfrage des Blinkerpfeils im IKE mit verschiedenen Lichtbedingungen
      {   
        if (message.b(3) == 0x04) 
        {  
          switch (message.b(1)) 
          {
            case 0x20:
            case 0x21:
            case 0x23:
            case 0x27:
            case 0x2B:
            case 0x33:
            case 0x3B:
              BlinkcountLi++;
              debugSerial.println(BlinkcountLi);
              if (BlinkcountLi < 2) 
              {
                turn=1;                      // Blinker links ein/LICHT ein
                debugSerial.println("send LCM Dimmer Request Turn=1");
                ibusTrx.write(LCMdimmReq);                                               // Anfrage Dimm-Wert des IKE Cluster vom LCM: 3F 03 D0 0B (E7)                 
              }
              break;

            case 0x40:
            case 0x41:
            case 0x43:
            case 0x47:
            case 0x4B:
            case 0x53:
            case 0x5B:
              BlinkcountRe++;
              debugSerial.println(BlinkcountRe);
              if (BlinkcountRe < 2) 
              {
                turn=2; // Blinker rechts ein/LICHT ein
                debugSerial.println("send LCM Dimmer Request Turn=2");
                ibusTrx.write(LCMdimmReq);                                               // Anfrage Dimm-Wert des IKE Cluster vom LCM: 3F 03 D0 0B (E7)  
              } 
              break;

            default:
              // Nichts tun für unbekannte Werte
            break;
          }
        } 
        else if (message.b(3) == 0x00) 
        {     // D0 07 BF 5B 40 00 00 00 73 , byte 3 = 00 =! Indicator_sync       // D0 07 BF 5B 00 = Byte 1=  0x00 = All_Off
          BlinkcountLi = 0;
          BlinkcountRe = 0;
          debugSerial.println("Reset Counts");
        }

        // wenn mehr als 3 mal geblinkt wurde, Blinker Aus
        if ((BlinkcountLi > 2) || (BlinkcountRe > 2)) 
        {
          BlinkcountLi=0;       // 
          BlinkcountRe=0;
          if (LCMdimmOK)
          {
            memcpy(LCMBlinker, BlinkerAus, sizeof(BlinkerAus));                                                 // BlinkerAus in LCMBlinker speichern
            memcpy(LCMBlinker + sizeof(BlinkerAus), LCMdimm, sizeof(LCMdimm));                                 // hinzufügen von LCMdimm
            memcpy(LCMBlinker + sizeof(BlinkerAus) + sizeof(LCMdimm), LCMBlinkerAdd, sizeof(LCMBlinkerAdd));   // hinzufügen von FF 00
            LCMBlinker[1] = sizeof(LCMBlinker)-1;                                                             // Länge des von LCMBlinker ermitteln und einsetzen an Position 2
            debugSerial.println("Blinker AUS");
            ibusTrx.write (LCMBlinker);    
            LCMdimmOK = 0;    
          }
        }        
      }

      // Abfrag des LCM DimmWertes im KombiInstrument
      if (turn == 1 || turn == 2) 
      {
        if ((source == M_LCM) && (destination == M_DIA) && (message.b(0) == 0xA0)){     // Antwort auf den Request des LCM-Status (D0 xx 3F A0 .....)
          debugSerial.println("LCM Antwort erkannt");
          for (int i = 1; i < 17; ++i) 
          {                                               
              LCMdimm[i-1] =message.b(i);         // lese 16 bytes ein = message.b(1-16)  (D0 xx 3F A0 |-> C1 C0 00 20 00 00 00 00 00 A0 00 00 88 14 84 E4 .....)
          }
          LCMdimmOK = 1;
          
          if (turn == 1)    // Blinker links
          {           
            // sende blinker links und die 16 byte der LCMdimm Antwort + FF 00
            memcpy(LCMBlinker, BlinkerLi, sizeof(BlinkerLi));                                                 // BlinkerLi in LCMBlinker speichern
            memcpy(LCMBlinker + sizeof(BlinkerLi), LCMdimm, sizeof(LCMdimm));                                 // hinzufügen von LCMdimm
            memcpy(LCMBlinker + sizeof(BlinkerLi) + sizeof(LCMdimm), LCMBlinkerAdd, sizeof(LCMBlinkerAdd));   // hinzufügen von FF 00
            LCMBlinker[1] = sizeof(LCMBlinker)-1;                                                             // Länge des von LCMBlinker ermitteln und einsetzen an Position 2
            debugSerial.println("Links Blinker ein");
            ibusTrx.write (LCMBlinker);
            turn = 0;   // Blinker abfrage erledigt
          }

          if (turn == 2)    // Blinker rechts
          {           
            // sende blinker rechts und die 16 byte der LCMdimm Antwort + FF 00
            memcpy(LCMBlinker, BlinkerRe, sizeof(BlinkerRe));                                                 // BlinkerRe in LCMBlinker speichern
            memcpy(LCMBlinker + sizeof(BlinkerRe), LCMdimm, sizeof(LCMdimm));                                 // hinzufügen von LCMdimm
            memcpy(LCMBlinker + sizeof(BlinkerRe) + sizeof(LCMdimm), LCMBlinkerAdd, sizeof(LCMBlinkerAdd));   // hinzufügen von FF 00
            LCMBlinker[1] = sizeof(LCMBlinker)-1;                                                             // Länge des von LCMBlinker ermitteln und einsetzen an Position 2
            debugSerial.println("Rechts Blinker ein");
            ibusTrx.write (LCMBlinker);
            turn = 0;   // Blinker abfrage erledigt
          }
        }       
      }
    }  


    // Beispiel: trigger based on a message from the steering wheel controls
    if (source == M_BMB) 
    {
      // if "Push right" is pressed: simulate a press of the dome light button
      if (message.b(1) == 0x05) 
      {
        // write the message to the transmit buffer
        ibusTrx.writeTxt("Hallo BMW iBus");
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


// Wenn Interrup dann sende die iBus Codes
void ClearToSend() {
  //debugSerial.println("-Interrupt-");
  ibusTrx.send(); // Nachricht senden, wenn Interrupt ausgelöst wird
}


