/**
 * Library for Kerbal Space Program mod KSPSerialIO
 *
 * Plugin by zitroen
 * Library by Nathan Gray
 */

#include "Arduino.h"
#include "KSPSerialIO.h"

// Change this if you need to talk to something else
#define Serial Serial


KSPSerialIO::KSPSerialIO(long baud)
{
  Serial.begin(baud);
  initTxPackets();
}
void KSPSerialIO::handshake()
{
  handshake_packet.id = 0;
  handshake_packet.M1 = 3;
  handshake_packet.M2 = 1;
  handshake_packet.M3 = 4;

  send(details(handshake_packet));
}

// Check the flag to see if we're properly connected
boolean KSPSerialIO::connected()
{
  return _connected;
}
// Get the status of a control flag
boolean KSPSerialIO::controlStatus(byte n)
{
  return ((vessel.ActionGroups >> n) & 1) == 1;
}

// Set a control flag
void KSPSerialIO::setControl(byte n, boolean on)
{
  if(on)
    control.MainControls |= (1 << n);       // forces nth bit of x to be 1.  all other bits left alone.
  else
    control.MainControls &= ~(1 << n);      // forces nth bit of x to be 0.  all other bits left alone.
}

// See if we can recieve data
int KSPSerialIO::update()
{
  int returnValue = -1;
  now = millis();
  
  if(receive())
  {
    // Yay, we have data
    last_update_time = now;
    returnValue = packet_id;
    
    if(packet_id == 0)
    {
      handshake();
    }
    _connected = true;
  }
  else if (now - last_update_time > idle_limit)
  {
    _connected = false;
  }
  if(_connected && now - last_control_time > control_refresh)
  {
    last_control_time = now;
    send(details(control));
  }
  return returnValue;
}

void KSPSerialIO::initTxPackets() {
  handshake_packet.id = 0;  
  control.id = 101;
}



/**
 * Receive packets over serial
 * Copied from the KSPSerialIO example
 */
boolean KSPSerialIO::receive()
{
  if ((rx_len == 0)&&(Serial.available()>3)){
    while(Serial.read()!= 0xBE) {
      if (Serial.available() == 0)
        return false;  
    }
    if (Serial.read() == 0xEF){
      rx_len = Serial.read();   
      packet_id = Serial.read(); 
      rx_array_inx = 1;

      switch(packet_id) {
        case 0:
          structSize = sizeof(handshake_packet);   
          address = (byte*)&handshake_packet;
          break;
        case 1:
          structSize = sizeof(vessel);   
          address = (byte*)&vessel;     
          break;
      }
    }

    //make sure the binary structs on both Arduinos are the same size.
    if(rx_len != structSize){
      rx_len = 0;
      return false;
    }   
  }

  if(rx_len != 0)
  {
    while(Serial.available() && rx_array_inx <= rx_len){
      buffer[rx_array_inx++] = Serial.read();
    }
    buffer[0] = packet_id;

    if(rx_len == (rx_array_inx-1)){
      //seem to have got whole message
      //last uint8_t is CS
      calc_CS = rx_len;
      for (int i = 0; i<rx_len; i++){
        calc_CS^=buffer[i];
      } 

      if(calc_CS == buffer[rx_array_inx-1]){//CS good
        memcpy(address,buffer,structSize);
        rx_len = 0;
        rx_array_inx = 1;
        return true;
      }
      else{
        //failed checksum, need to clear this out anyway
        rx_len = 0;
        rx_array_inx = 1;
        return false;
      }
    }
  }

  return false;
}

void KSPSerialIO::send(uint8_t * address, uint8_t len)
{
  uint8_t CS = len;
  Serial.write(0xBE);
  Serial.write(0xEF);
  Serial.write(len);
  
  for(int i = 0; i<len; i++){
    CS^=*(address+i);
    Serial.write(*(address+i));
  }
  Serial.write(CS);
}

