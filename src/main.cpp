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
      if ((source == M_LCM) && (destination == M_ALL) && (message.b(0) == 0x5B))      // Abfrage des Blinkerpfeils mit verschiedenen Lichtbedingungen
      {   
        if (message.b(3) == 0x04) 
        {  
          
          debugSerial.print("BlinkerPfeil li oder re ");
          
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
                turn=1; // Blinker links ein/LICHT ein
                debugSerial.println("send LCM Dimmer Request from switch");
                ibusTrx.write(LCMdimmReq);                                               // Frage den Dimm-Wert im IKE Cluster ab 3F 03 D0 0B (E7)                 
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

        switch (turn) 
        {
          case 1:
            debugSerial.println("Links Blinker ein");
            ibusTrx.write(BlinkerLi);
            //ibusTrx.writeTxt("bis bald Cordula");
            ibusTrx.writefix(LCMdimmReplay, LCMdimmReplaylen);
            //ibusTrx.send(); // Nachricht senden
            //ibusTrx.write(receivedMessage);
            turn = 0;
              
            //delay(1500);
            
            //ibusTrx.write(receivedMessage);
          break;

          case 2:
            debugSerial.println("Rechts Blinker ein");
            ibusTrx.write(BlinkerRe);                                 // 3F 0C D0 0C 00 00 40 00 00 00 00 00 00 AF
            //ibusTrx.write(receivedMessage);
            turn = 0;
            //delay(1500);
            //ibusTrx.write(BlinkerAus);
            //ibusTrx.write(receivedMessage);
          break;  

          default:
            if ((BlinkcountLi > 2) || (BlinkcountRe > 2)) // wenn mehr als 3 mal geblinkt wurde
            {
              BlinkcountLi=0;       // 
              BlinkcountRe=0;
              ibusTrx.write(BlinkerAus);
              debugSerial.println("Blinker AUS");
            } 
          break;
        }

        // Blinker selection
        if (turn == 1 || turn == 2) 
        {
          ibusTrx.write(LCMdimmReq);                                                // Frage den Dimm-Wert im IKE Cluster ab 3F 03 D0 0B (E7) 
          debugSerial.println("send LCM Dimmer Request");
          /*int Zaehler = 0;
          while (Zaehler <= 10) {                                                  //100 Notausstieg wenn nach 100 Bytes keine Diagnose-Antwort vom LCM kommt//// 3F LL 00 0C 10 05 3F CK // 24.7% = 63/255 = 3F/FF
            Zaehler++;
            if ((source == 0xA0) && (length == 0xC1) && (destination == 0xC0)){     // Antwort auf den Request des LCM-Status (A0 C1 C0 00 20 00 00 00 00 00 A0 00 00 88 14 84 E4 + FF 00 //0A 00 00 00 00 00 00 00 00 00 00 FF FF FE)
              for (int i = 0; i < length; ++i) {                                    // diesen ibus code in ein array packen und nach dem blinker-code zurüsckschicken
                  receivedMessage[i] = packet[i];
              }
              receivedMessage[0] = 0x0C;
              byte checksum = 0;
              for (int i = 0; i < length - 1; ++i) {
                checksum ^= receivedMessage[i];
              }
              receivedMessage[length - 1] = checksum;
            }
            return; 
          } */
          
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
  debugSerial.println("-Interrupt-");
  ibusTrx.send(); // Nachricht senden, wenn Interrupt ausgelöst wird
}


