/*
  IbusTrx (v2.4.0)
  Arduino library for sending and receiving messages over the BMW infotainment bus (IBUS).
  Author: D. van Gent
  More info: https://0x7b.nl/ibus

  THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
*/

#ifndef IbusTrx_h
#define IbusTrx_h

#include "Arduino.h"
#include "IbusMessage.h"
#include "IbusNames.h"


class IbusTrx{
  public:
    void begin(HardwareSerial &userPort);
    void end();
    void write(uint8_t message[]);        // senden der Ibus Message
    void writeTxt(const char txtmessage[]);     // senden ener Text Nachricht an das IKE
    void senStapin(uint8_t pin);
    bool available();
    bool transmitWaiting();
    uint8_t length();
    IbusMessage readMessage();
    
  private:
    HardwareSerial* serialPort;
    void clearBuffer();
    bool checkMessage();
    bool tx_msg_waiting = false; // message waiting in transmit buffer
    bool rx_msg_waiting = false; // message waiting in receive buffer
    uint8_t rx_buffer[0xFF] = {0x00}; // receive bufer
    uint8_t tx_buffer[0xC1] = {0x00}; // transmit buffer, hier wie die maximale Größe angegeben, 0x20 =32 bytes also 32 hex werte können maximal gesendet werden, 0xC1=193
    uint8_t rx_bytes = 0; // number of bytes in receive buffer
    uint8_t tx_bytes = 0; // number of bytes in transmit buffer
    uint32_t t_last_rx_byte = 0; // timestamp of last byte received
    uint8_t senSta; // Die Variable senSta als private Variable der Klasse
};

void ClearToSend();

#endif
