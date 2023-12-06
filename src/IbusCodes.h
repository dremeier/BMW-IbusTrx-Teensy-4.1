/*
  iBus Codes and setup for the main.cpp
*/
#ifndef IbusCodes_h
#define IbusCodes_h
#include <Arduino.h>

/*------------------------ DEBUG Serial.print -------------------------------------------------------------*/
#define DEBUG Serial     // USB Port on Teensy   // debugging aktivieren, if comment in then you have debugging over serial.print
//  my debug.h ;                        '#define DEBUG Serial' auskommentieren um den print abzuschalten
  #ifdef DEBUG
    #define debug(...) DEBUG.print(__VA_ARGS__)
    #define debugln(...) DEBUG.println(__VA_ARGS__)
    #define debugbegin(...) DEBUG.begin(__VA_ARGS__)
    #define debugf(...) DEBUG.printf(__VA_ARGS__)
  #else
    #define debug(...)
    #define debugln(...)
    #define debugbegin(...)
    #define debugf(...)
  #endif
// anstatt: serial.print wird: debug verwendet
// ende debug.h
// #############################################################################################################

//#define debugSerial Serial        // USB Port on Teensy
#define ibusPort Serial2          // Serial für Ibus = Serial2 = RxPIN 7 und TxPin 8 , CleartoSent = Pin 5
const byte ledPin = 13;           // buildin LED
const byte senSta = 5;            // sen/Sta output from Melexis TH3122 PIN9.
const byte TH_EN = 6;             // EN input from Melexis TH3122 PIN2.
const byte TH_RESET = 4;          // Reset output from Melexis TH3122 PIN14.

bool IKEclear = false;            // flag um das IKE zu löschen
long msTimer = 0;                 // used for cycle timing
long msSleep = 0;                 // zeit für System-Schlaf
bool sysSleep = 1;                // flag für SleepTimer
unsigned long SleepTime = 60000; // Zeit bis zum schlafen legen des TH3122 (60.000 = 1 Minute)
unsigned int t_clearIKE = 10000;  // zeit in milis bis der Text im IKE gelöscht wird
bool Tippblinken = true;          // flag zum ein und aus schalten der Tipp-Blinker Funktion
bool AutomVerriegeln = true;      // Automatisches Verriegln bei Geschwindigkeit > 30Km/h und Entriegeln bei Motor aus
bool ZVlocked =false;             // flag für geschwindigkeit war größer 30 usw.
uint8_t BlinkcountLi = 0;         // flag um die Anzahl der Blinker zu zählen
uint8_t BlinkcountRe = 0;
byte LCMdimm[16];                 // ausgelesener Dimmwert
byte LCMBlinker[31];              // Array für das zusammenbauen des Blinker-Befehls mit LCMdimmwerten
byte BCcool[15];                  // Array für die Kühlmitteltemperatur im Bordmonitor
uint8_t turn;                     // flag für Blinker li und re
uint8_t LCMdimmOK;                // flag für LCMdimmWerte erfolgreich ausgelesen  
uint8_t speed;                    // Geschwindigkeit in Km/h
uint16_t rpm;                     // Motordrehzahl
uint8_t outTemp;                  // Außen Temperatur
uint8_t coolant;                  // Kühlmittel Temperatur

const int LDR_PIN = A0;  // Analog-Pin, an dem der LDR angeschlossen ist
const int NUM_READINGS = 10;  // Anzahl der Messungen für den Durchschnitt
int ldrValues[NUM_READINGS];  // Array für Messwerte
int ldrindex = 0;  // Index für das Speichern der Werte im Array
int sum = 0;  // Variable zur Berechnung der Summe
int average = 0;  // Variable für den Durchschnittswert
bool dunkel = false;  // Flag zur Entscheidung über Grenzwerte
bool heiml = false; // Funkschlüssel ZV auf, für Heimleuchten
bool MotorOff =false; // Schlüssel aus dem Zündschloß gezogen für Heimleuchten und Zentralverriegelung auf
bool DvrdoorFr = false; // Fahrertür auf

// Ober- und Untergrenze für den LDR-Wert, Heimleuchten
/*Natürliches Tageslicht erreicht an wolkenfreien Sommertagen bis zu 100.000 Lux, ein bewölkter Himmel 20.000 Lux. Im Winter erreicht ein bedeckter Himmel etwa 3.500 Lux. Bei Dämmerung herrschen ca. 750 Lux.*/
const int UPPER_THRESHOLD = 500;
const int LOWER_THRESHOLD = 400;

void Coolant (uint8_t temp);
void Daemmerung() ;

/*
// Example: define the message that we want to transmit
// the message must be defined as an array of uint8_t's (unsigned 8-bit integers)
uint8_t toggleDomeLight[6] = {
  M_DIA, // sender ID (diagnostic interface)
  0x05,  // length of the message payload (including destination ID and checksum)
  M_GM5, // destination ID (body control module)
  GM5_SET_IO, // the type of message (IO manipulation)
  GM5_BTN_DOME_LIGHT, // the first parameter (the IO line that we want to manipulate)
  0x01 // second parameter
  // don't worry about the checksum, the library automatically calculates it for you
};*/
/*
// IBUS message definitions. Add/Remove as needed.
uint8_t KEY_IN [7] PROGMEM = { 
  0x44 , 0x05 , 0xBF , 0x74 , 0x04 , 0x00 , 0x8E }; // Ignition key in
uint8_t KEY_OUT [7] PROGMEM = { 
  0x44 , 0x05 , 0xBF , 0x74 , 0x00 , 0xFF , 0x75 }; // Ignition key out
uint8_t MFL_VOL_UP [6] PROGMEM = { 
  0x50 , 0x04 , 0x68 , 0x32, 0x11 , 0x1F }; // Steering wheel Volume Up
uint8_t MFL_VOL_DOWN [6] PROGMEM = { 
  0x50 , 0x04 , 0x68 , 0x32, 0x10 , 0x1E }; // Steering wheel Volume Down
uint8_t MFL_TEL_VOL_UP [6] PROGMEM = { 
  0x50 , 0x04 , 0xC8 , 0x32, 0x11 , 0xBF }; // Steering wheel Volume Up - Telephone
uint8_t MFL_TEL_VOL_DOWN [6] PROGMEM = { 
  0x50 , 0x04 , 0xC8 , 0x32, 0x10 , 0xBE }; // Steering wheel Volume Down - Telephone
uint8_t MFL_SES_PRESS [6] PROGMEM = { 
  0x50 , 0x04 , 0xB0 , 0x3B, 0x80 , 0x5F }; // Steering wheel press and hold phone button
uint8_t MFL_SEND_END_PRESS [6] PROGMEM = { 
  0x50 , 0x04 , 0xC8 , 0x3B, 0x80 , 0x27 }; // Steering wheel send/end press
uint8_t MFL_RT_PRESS [6] PROGMEM = { 
  0x50 , 0x04 , 0x68 , 0x3B , 0x02, 0x05 }; // MFL R/T press
uint8_t CD_STOP [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x01 , 0x00 , 0x4C }; // CD Stop command
uint8_t CD_PLAY [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x03 , 0x00 , 0x4E }; // CD Play command
uint8_t CD_PAUSE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x02 , 0x00 , 0x4F }; // CD Pause command
uint8_t CD_STOP_STATUS [12] PROGMEM = { 
  0x18 , 0x0A , 0x68 , 0x39 , 0x00 , 0x02 , 0x00 ,  0x3F , 0x00 , 0x07 , 0x01 , 0x78 }; // CD stop request
uint8_t CD_PLAY_STATUS [12] PROGMEM = { 
  0x18 , 0x0A , 0x68 , 0x39 , 0x02 , 0x09 , 0x00 ,  0x3F , 0x00 , 0x07 , 0x01 , 0x71 }; // CD play request
uint8_t INCOMING_CALL [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x05 , 0x02 }; // Incoming phone call
uint8_t PHONE_ON [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x10 , 0x17 }; // Phone On
uint8_t HANDSFREE_PHONE_ON [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x11 , 0x16 }; // Hands Free Phone On
uint8_t ACTIVE_CALL [6] PROGMEM = { 
  0xC8 , 0x04 , 0xE7 , 0x2C , 0x33 , 0x34 }; // Active phone call
uint8_t IGNITION_OFF [6] PROGMEM = { 
  0x80 , 0x04 , 0xBF , 0x11 , 0x00 , 0x2A }; // Ignition Off
uint8_t IGNITION_POS1 [6] PROGMEM = { 
  0x80 , 0x04 , 0xBF , 0x11 , 0x01 , 0x2B }; // Ignition Acc position - POS1
uint8_t IGNITION_POS2 [6] PROGMEM = { 
  0x80 , 0x04 , 0xBF , 0x11 , 0x03 , 0x29 }; // Ignition On position - POS2
uint8_t REMOTE_UNLOCK [6] PROGMEM = { 
  0x00 , 0x04 , 0xBF , 0x72 , 0x22 , 0xEB }; // Remote control unlock
uint8_t DSP_STATUS_REQUEST [5] PROGMEM = { 
  0x68 , 0x03 , 0x6A , 0x01 , 0x00 }; // DSP status request
uint8_t DSP_STATUS_REPLY [6] PROGMEM = { 
  0x6A , 0x04 , 0xFF , 0x02 , 0x00 , 0x93 }; // DSP status reply
uint8_t DSP_STATUS_REPLY_RST [6] PROGMEM = { 
  0x6A , 0x04 , 0xFF , 0x02 , 0x01 , 0x92 }; // DSP status ready after reset to LOC
uint8_t DSP_VOL_UP_1 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x11 , 0x25 }; // Rotary Volume Up 1 step
uint8_t DSP_VOL_UP_2 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x21 , 0x15 }; // Rotary Volume Up 2 step
uint8_t DSP_VOL_UP_3 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x31 , 0x05 }; // Rotary Volume Up 3 step
uint8_t DSP_VOL_DOWN_1 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x10 , 0x24 }; // Rotary Volume Down 1 step
uint8_t DSP_VOL_DOWN_2 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x20 , 0x14 }; // Rotary Volume Down 2 step
uint8_t DSP_VOL_DOWN_3 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x32, 0x30 , 0x04 }; // Rotary Volume Down 3 step
uint8_t DSP_FUNC_0 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0x30 , 0x00 }; // DSP_Function 0
uint8_t DSP_FUNC_1 [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xE1 , 0xD1 }; // DSP_Function 1
uint8_t DSP_SRCE_OFF [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xAF , 0x9F }; // DSP Source = OFF
uint8_t DSP_SRCE_CD [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xA0 , 0x90 }; // DSP Source = CD
uint8_t DSP_SRCE_TUNER [6] PROGMEM = { 
  0x68 , 0x04 , 0x6A , 0x36, 0xA1 , 0x91 }; // DSP Source = Tuner
uint8_t GO_TO_RADIO [6] PROGMEM = { 
  0x68 , 0x04 , 0xFF , 0x3B, 0x00 , 0xA8 }; // Go  to radio - I think
uint8_t REQUEST_TIME [7] PROGMEM = { 
  0x68 , 0x05 , 0x80 , 0x41, 0x01 , 0x01 , 0xAC}; // Request current time from IKE
uint8_t CLOWN_FLASH [7] PROGMEM = { 
  0x3F , 0x05 , 0x00 , 0x0C , 0x4E , 0x01 , 0x79 }; // Turn on clown nose for 3 seconds
uint8_t BACK_ONE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x08 , 0x00 , 0x45 }; // Back
uint8_t BACK_TWO [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x08 , 0x01 , 0x44 }; // Back
uint8_t LEFT [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x0A , 0x01 , 0x46 }; // Left
uint8_t RIGHT [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x0A , 0x00 , 0x47 }; // Right
uint8_t SELECT [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x07 , 0x01 , 0x4B }; // Select
uint8_t BUTTON_ONE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x01 , 0x4A }; // Button 1
uint8_t BUTTON_TWO [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x02 , 0x49 }; // Button 2
uint8_t BUTTON_THREE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x03 , 0x48 }; // Button 3
uint8_t BUTTON_FOUR [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x04 , 0x4F }; // Button 4
uint8_t BUTTON_FIVE [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x05 , 0x4E }; // Button 5
uint8_t BUTTON_SIX [7] PROGMEM = { 
  0x68 , 0x05 , 0x18 , 0x38 , 0x06 , 0x06 , 0x4D }; // Button 6
uint8_t CDC_STATUS_REPLY_RST [6] PROGMEM = { 
  0x18 , 0x04 , 0xFF , 0x02 , 0x01 , 0xE0 }; // CDC status ready after reset to LOC
uint8_t CDC_STATUS_REQUEST [5] PROGMEM = { 
  0x68 , 0x03 , 0x18 , 0x01 , 0x72 }; // CDC status request
uint8_t CDC_STATUS_REPLY [6] PROGMEM = { 
  0x18 , 0x04 , 0xFF , 0x02 , 0x00 , 0xE1 }; // CDC status reply
uint8_t CD_STATUS [16] PROGMEM = { 
  0x18 , 0x0E , 0x68 , 0x39 , 0x00 , 0x82 , 0x00 , 0x3F , 0x00 , 0x07 , 0x00 , 0x00 , 0x01 , 0x01 , 0x01 , 0xFC }; // CD status
uint8_t BUTTON_PRESSED [6] PROGMEM = {
  0x68 , 0x04 , 0xFF , 0x3B ,  0x00 , 0xA8 }; // Radio/Telephone control, No_buttons_pressed 
uint8_t VOL_INCREMENT [64] PROGMEM = { 
  0,68,70,72,74,76,78,80,82,84,86,88,90,92,94,96,98,100,102,104,106,108,110,112,114,116,118,120,122,124,126,
  128,130,132,134,136,138,140,142,144,146,148,150,152,154,156,158,160,162,164,166,168,170,172,174,176,
  178,180,182,184,186,188,190,192}; // Volume increments
*/

// die checksumme muss nicht mit angegeben werden
uint8_t cleanIKE [6] PROGMEM = {
  0x30, 0x05, 0x80, 0x1A, 0x30, 0x00}; // IKE Anzeige wird gelöscht, wichtig sonst bleibt sie immer an
uint8_t BlinkerRe [13] PROGMEM = {
  0x3F, 0x0C, 0xD0, 0x0C, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //3F0FD00C000040000000000000
uint8_t BlinkerLi [13] PROGMEM = {
  0x3F, 0x0c, 0xD0, 0x0C, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; // 3F 0B D0 0C 00 00 80 00 00 00 00 00
uint8_t BlinkerAus [13] PROGMEM = {
  0x3F, 0x0C, 0xD0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //3F0FD00C000000000000000000
uint8_t LCMdimmReq [] PROGMEM ={0x3F, 0x03, 0xD0, 0x0B}; // Diagnoseanfrage ans LCM um Helligkeitswert für IKE zu finden
//uint8_t LCMdimmReplay [32] PROGMEM = {0xA0, 0xC1, 0xC0, 0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x00, 0x00, 0x88, 0x14, 0x84, 0xE4, 0xFF, 0x00, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFE};
//size_t LCMdimmReplaylen = 31; // die Größe von LCMdimmReplay, komischerweise ist das Array 32 byte groß aber es funktioniert nur mit 31
uint8_t LCMBlinkerAdd [2]  PROGMEM = {0xFF, 0x00}; // Anhängsel nach dem LCMBlinker zusammen bau
uint8_t BCcoolbeginn [7]  PROGMEM = {0x80, 0x0E, 0xE7, 0x24, 0x0E, 0x00, 0x20}; // Kühlmitteltemperatur im Bordmonitor anzeigen Anfangs-Kette
uint8_t BCcoolend [5] PROGMEM = {0x20, 0xB0, 0x43, 0x20, 0x20}; // Kühlmitteltemperatur im Bordmonitor anzeigen Schluß-Kette (_°C__)
// Heimleuchten Nebel und Bremslichter: 3F0FD00C000000001844000000E5FF00AA
//Fahrertür auf
uint8_t door_open_driver[] PROGMEM = {0x00, 0x05, 0xBF, 0x7A, 0x51};  // status Fahrertür ist auf (wenn Tür auf geht Heimleuchten einschalten)
//00 05 BF 7A 51
uint8_t ZV_lock[] PROGMEM = {0x3F, 0x05, 0x00, 0x0C, 0x00, 0x0B}; // Zentralverriegelung öffnen / schließen 3F05000C000B(3D)

uint8_t Heimleuchten[] PROGMEM = 
  {0x3F, 0x0F, 0xD0, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x18, 0x44, 0x00, 0x00, 0x00, 0xE5, 0xFF, 0x00}; // Lichter für Heimleuchten: Bremslicht und Nebelleuchten: 3F0FD00C000000001844000000E5FF00

#endif
