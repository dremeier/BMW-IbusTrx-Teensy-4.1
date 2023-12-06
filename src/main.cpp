// include the IbusTrx library
#include <IbusTrx.h>
#include <Wire.h> 
#include <BH1750.h>         //GY-302 mit Sensor BH1750
#include "IbusCodes.h"      // hier sind alles Ibus-Codes und Variablen zur Configuration
#include <Snooze.h>
// create a new IbusTrx instance
IbusTrx ibusTrx;
BH1750 lightMeter;

SnoozeUSBSerial usb;
SnoozeDigital digital;                  // this is the pin's wakeup driver
SnoozeBlock config_sleep(usb, digital); // install driver into SnoozeBlock

/*#####################################################
################ SETUP ################################
#######################################################*/
void setup(){
  pinMode(ledPin, OUTPUT);
  pinMode(TH_EN,OUTPUT);
  pinMode(senSta, INPUT_PULLUP);                          // pin 5 des TH3122 für Clear to send, mit einer diode von Teensy pin > TH3122 pin5
  pinMode(TH_RESET, INPUT_PULLUP);                        // Reset pin des TH3122, dieser ist high wenn TH arbeitet, mit einer diode von Teensy pin > TH3122 pin4
  //wakeup Interrupt: pin, mode, type         
  digital.pinMode(TH_RESET, INPUT_PULLUP, RISING);        // SnoozeBlock, wakeup Pin. Wenn TH_RESET high wird, wird der Teensy aufgeweckt

  debugbegin(115200);                                     // Teensy USB port, nicht nötig
  ibusTrx.begin(ibusPort);                                // Hardware Serial Nr. an IbusTrx übergeben

  ibusTrx.senStapin(senSta);                              // senSta Pin an IbusTrx Library übergeben
  attachInterrupt(digitalPinToInterrupt(senSta), ClearToSend, FALLING);   // Interrupt: Wenn senSta LOW wird, gehe zu Funktion ClearToSend
  delay (1000);

  Wire.begin();                                           // I²C init für Lichtsensor BH1750 
  lightMeter.begin(BH1750::CONTINUOUS_LOW_RES_MODE);      // initialise the light Sensor in continius low Resolution Mode 
  
  // TH3122  Reset-Status-Abfrage
    //if (digitalRead(TH_RESET) == LOW)
    //{
      //debugln("TH schläft, wecke ihn auf!");
      //th_reset = true;
    //}  

  // TH3122 einschalten und flags setzen
  digitalWrite(TH_EN, HIGH);      // TH3122 enable pin high
  sysSleep = false;               // SystemSleep =false -> System ist aktiv
  msSleep =millis();              // Sleep Timer reset

  debugln(F("-- IBUS trx on Teensy 4.x --"));
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

    /* the following addresses are defined in the IbusTrx library:
    M_GM5 = 0x00;  // GM5: body control module
    M_NAV = 0x3B;  // NAV: Navigation / Monitor / Bordcomputer
    M_DIA = 0x3F;  // DIA: diagnostic computer
    M_EWS = 0x44;  // EWS: immobilizer
    M_MFL = 0x50;  // MFL: steering wheel controls
    M_IHKA = 0x5B; // IHKA: climate control panel
    M_RAD = 0x68;  // RAD: radio module
    M_IKEC = 0x80; // IKE: instrument cluster
    M_ALL = 0xBF;  // ALL: broadcast message  (GLO = Global)
    M_TEL = 0xC8;  // TEL: telephone module
    M_LCM = 0xD0;  // LCM: light control module
    M_IKET = 0x30; // IKE TextFeld (Großes Display)
    M_BMB = 0xF0;  // BMB: Bordmonitor Buttons
    M_DSP = 0x6A;  // DSP: Digital Sound Processor
    */
    
    // these two functions return the source and destination addresses of the IBUS message:
    unsigned int source = message.source();
    unsigned int destination = message.destination();
    //unsigned int length = message.length();

     // 00 + BF - Funkschlüssel Auf/Zu, Türen Status
    if ((source == M_GM5) && (destination == M_ALL))
    { 
      switch (message.b(0))                                 // erste Nachrichten Byte
      {
        case 0x72:                                          // Funkschlüssel auf und zu // 00 04 BF 72 xx
          heiml =true;
          switch (message.b(1))                             // zweite Nachrichten Byte
          {
            case 0x22:
              ibusTrx.writeTxt("einsteigen Cordula");       // 00 04 BF 72 22 ck  
              break;
            case 0x26:  // Funkschlüssel Andre auf
              ibusTrx.writeTxt("einsteigen Andre");
              break;
            case 0x12:
              ibusTrx.writeTxt("Tschuess Cordula");
              break;
            case 0x16:  // FunkSchlüssel Andre zu
              ibusTrx.writeTxt("Tschuess Andre");
              break;
            default:
              break;
          }
          break;
        
        case 0x7A:                                // status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
          switch (message.b(1))
          {
            case 0x51:                            // 00 05 BF 7A 51 status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
              DvrdoorFr = true;
              break;
            
            default:
              break;
          }
        break;

        default:
          break;
      }
    }

    // Schlüssel im Schloß , Motor aus // 44 05 bf 74 xx xx
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
      }
      if (message.b(1) == 0x04)                                     // 44 05 bf 74 04 xx   Schlüssel wird ins Schloß gesteckt
      {               
        if (message.b(2) == 0x00)
        {
          ibusTrx.writeTxt("gute Fahrt Cordula");                    // 44 05 bf 74 04 00 Cordula 
          IKEclear=true;                                             // bereit um den Text im IKE zu löschen
          msTimer=millis();                                          // aktuelle Zeit in den Timer legen
        }
        if (message.b(2) == 0x05) 
        {
          ibusTrx.writeTxt("gute Fahrt Andre");                      // 44 05 bf 74 04 05 Andre
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
              debugln(BlinkcountLi);
              if (BlinkcountLi < 2) 
              {
                turn=1;                      // Blinker links ein/LICHT ein
                debugln("send LCM Dimmer Request Turn=1");
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
              debugln(BlinkcountRe);
              if (BlinkcountRe < 2) 
              {
                turn=2; // Blinker rechts ein/LICHT ein
                debugln("send LCM Dimmer Request Turn=2");
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
          debugln("Reset Counts");
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
            debugln("Blinker AUS");
            ibusTrx.write (LCMBlinker);    
            LCMdimmOK = 0;    
          }
        }        
      }

      // Abfrag des LCM DimmWertes im KombiInstrument
      if (turn == 1 || turn == 2) 
      {
        if ((source == M_LCM) && (destination == M_DIA) && (message.b(0) == 0xA0)){     // Antwort auf den Request des LCM-Status (D0 xx 3F A0 .....)
          debugln("LCM Antwort erkannt");
          for (uint8_t i = 1; i < 17; ++i) 
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
            debugln("Links Blinker ein");
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
            debugln("Rechts Blinker ein");
            ibusTrx.write (LCMBlinker);
            turn = 0;   // Blinker abfrage erledigt
          }
        }       
      }
    }  

    // Geschwindigkeit, RPM, OutTemp, CoolantTemp
    if ((source == M_IKEC) && (destination == M_ALL))
      {
        switch (message.b(0)) 
        {
          case 0x18:                                // 80 05 BF 18 3F 18 (05),IKE --> GLO: Speed/RPM, Speed 126 km/h  [78 mph], 2,400 RPM
              speed = message.b(1) * 2;
              rpm = message.b(2) * 100;
              debug("Speed: ");
              debugln(speed);
              debug("RPM: ");
              debugln(rpm);
            break;

          case 0x19:                                // 80 05 BF 19 2F 5C (50),IKE --> GLO Temperature Outside 47C Coolant 92C
              outTemp = message.b(1);
              debug("Aussen Temperatur: ");
              debugln(outTemp);

              coolant = message.b(2);
              debug("Kuehlmittel Temperatur: ");
              debugln(coolant);
              Coolant (coolant);                    // Gehe in die Funktion um Die Kühlmitteltemperatur im Bordmonitor anzuzeigen
            break;

          case 0x11:                                // Zündungs Nachricht
            	switch (message.b(1))
              {
                case 0x00:
                    switch (message.b(2))
                    {
                      case 0x2A:                      // Zündung Aus
                          MotorOff = true;
                        break;
                    }
                  break;
                case 0x01:
                    switch (message.b(2))
                    {
                      case 0x2B:                      // Zündung Pos1
                          // noch nix
                        break;
                    }
                  break;
                case 0x03:
                    switch (message.b(2))
                    {
                      case 0x29:                      // Zündung Pos2
                          // noch nix
                        break;
                    }
                  break;
              } 
            break;
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
      //debug(F("Length "));
      //debugln(messageLength);

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
    }
    
    // flags und Timer für TH-Sleep zurücksetzen:
    if (sysSleep)
    {
      //debugln("TH_EN high");
      digitalWrite(TH_EN, HIGH);      // TH3122 enable pin high
      sysSleep = false;               // SystemSleep =false -> System ist aktiv
      debugln("SystemTimer reset und System aktiv");
    }
    msSleep =millis();              // Sleep Timer reset

  // nach Inaktivität TH und Teensy in Sleep mode
  }else if ((millis()-msSleep) >= (SleepTime) && (!sysSleep))   // 600.000 ms = 10 Minuten
  {
    debugln("TH_EN low und System schlaeft");
    digitalWrite(TH_EN, LOW);         // TH3122 enable pin low, TH wird disabled und TH-LDO abgeschaltet
    sysSleep = true;                  // SystemSleep =true -> System schläft
    delay(50);
    Snooze.deepSleep( config_sleep ); // deepSleep ~20mA, sleep ~30mA, in hibernate IBUStrx does not woke up
  }    
  // ############################ if (messageWaiting) ENDE #################################


  // ############################# weiter im Loop ##############
  // Text im IKE löschen
  if (IKEclear)                               // bereit um den Text im IKE zu löschen
  { 
    if (speed > 30)
    {
      ibusTrx.write(cleanIKE);                // lösche den Text
      IKEclear=false;                         // Fertig
    }
    /*
    if ((millis()-msTimer) >= t_clearIKE)     // Zeit abgelaufen?
    {      
      ibusTrx.write(cleanIKE);                // lösche den Text
      IKEclear=false;                         // Fertig
    }*/
  }
  
  if (AutomVerriegeln)                        // Automatisches Verriegln bei Geschwindigkeit > 30Km/h und Entriegeln bei Motor aus
  {
    if (!ZVlocked && (speed > 20))            // wenn speed größer 20 km/h dann ZV verriegeln
    {
      ibusTrx.write(ZV_lock);
      ZVlocked =true;
    }
    if (MotorOff && ZV_lock)                  // Moor aus / Zündung aus dann Entriegeln
    {
      ibusTrx.write(ZV_lock);
      ZVlocked =false;
    }
  }

  Daemmerung();     // gehe zu Dämmerung und messe Helligkeit und ggf schalte Heimleuchten ein

  /* Debug abfrage status TH Reset Pin
  if ((!th_reset) && ( digitalReadFast(TH_RESET)))
  {
    debugln("TH Reset high");
    th_reset = true;
  }
  if ((th_reset) && ( !digitalReadFast(TH_RESET)))
  {
    debugln("TH Reset low");
    th_reset = false;
  }
  */
  
}  // ########################### Ende loop ##############################################



/*#####################################################
################ Funktionen ###########################
#######################################################*/

// Kühlmitteltemperatur im Bordmonitor anzuzeigen
void Coolant (uint8_t coolant)
{
  uint8_t coolantTemp[3];       // Kühlmitteltemperatur zerlegt und in Hex abgelegt z.B. 128 => 31 32 38
  uint8_t zerlegen = coolant;   // Hilfs Variable zum zerlegen des Decimalwertes
  // zerlegen von coolant
  for (uint8_t i = 2; i >= 0; --i) 
  {
    coolantTemp[i] = (zerlegen % 10) + 0x30;  // Die letzte Ziffer der Zahl +30 (in Hex 0x1E) und im Array speichern
    zerlegen /= 10;                           // Eine Ziffer entfernen
  }
  // wenn coolant nur zwei Ziffern hat füge ein Leerzeichen in HEX voran
  if (coolant < 100) 
  {
    coolantTemp[0] = 0x20;    
  }
  /*  for (int i = 0; i < 3; i++) 
  {
    debug(" ");
    debug(coolantTemp[i]);
  }*/
  // Zeichenkette zusammen bauen: 80 0F E7 24 0E 00 20  + coolantTemp + 20 B0 43 20 20 + cksum
  memcpy(BCcool, BCcoolbeginn, sizeof(BCcoolbeginn));                                         // BCcoolbeginn in BCcool speichern
  memcpy(BCcool + sizeof(BCcoolbeginn), coolantTemp, sizeof(coolantTemp));                    // hinzufügen von coolantTemp
  memcpy(BCcool + sizeof(BCcoolbeginn) + sizeof(coolantTemp), BCcoolend, sizeof(BCcoolend));  // hinzufügen von {0x20, 0xB0, 0x43, 0x20, 0x20}
  BCcool[1] = sizeof(BCcool)-1;                                                               // länge der gesammten Zeichenkette berechnen und eintragen
  // sende an den Bordmonitor im BC-Bereich anstelle von Timer 1 oder 2
  ibusTrx.write (BCcool);
}

// Lichtsensor BH1750 und Heimleuchten
void Daemmerung() 
{
  if ((millis()-msTimer) >= 800)     // 500ms Zeit abgelaufen?
    {      
      msTimer = millis();
      // Lese den Wert vom LDR
      //int ldrValue = analogRead(LDR_PIN);

      float ldrValue = lightMeter.readLightLevel();
      
      // Berechne den Durchschnitt
      sum -= ldrValues[ldrindex];  // Subtrahiere den alten Wert
      ldrValues[ldrindex] = ldrValue;  // Speichere den neuen Wert
      sum += ldrValue;  // Addiere den neuen Wert
      ldrindex = (ldrindex + 1) % NUM_READINGS;  // Aktualisiere den ldrindex

      average = sum / NUM_READINGS;  // Berechne den Durchschnitt

      // Entscheide anhand der Hysterese, ob das dunkel flag gesetzt wird
      if (average > UPPER_THRESHOLD && dunkel) {
        dunkel = false;
      } else if (average < LOWER_THRESHOLD && !dunkel) {
        dunkel = true;
      }

      // Ausgabe des Durchschnitts und des dunkel-flags
      //debug("Average: ");
      //debug(average);
      //debug(" - dunkel: ");
      //debugln(dunkel);
      //dunkel = true;  // nur zum testen wenn kein LDR angeschlossen ist
    }
  if ((dunkel && MotorOff && DvrdoorFr) || (dunkel && heiml))
  {
    debug("Heimleuchten gesendet!");
    ibusTrx.write (Heimleuchten);
    MotorOff = false;
    DvrdoorFr = false;
    heiml =false;
  }  
}

// Wenn Interrup dann sende die iBus Codes
void ClearToSend() {
  //debugln("-Interrupt-");
  ibusTrx.send(); // Nachricht senden, wenn Interrupt ausgelöst wird
}