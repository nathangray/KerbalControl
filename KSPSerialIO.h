/**
 * Library for Kerbal Space Program mod KSPSerialIO
 *
 * Plugin by zitroen
 * Library by Nathan Gray
 */

#ifndef KSPSerialIO_h
#define KSPSerialIO_h

#include "Arduino.h"

#define details(name) (uint8_t*)&name,sizeof(name)

struct HandShakePacket
{
  byte id;
  byte M1;
  byte M2;
  byte M3;
};

/**
 * Control the ship
 */
struct ControlPacket {
  byte id;
  byte MainControls;                  //SAS RCS Lights Gear Brakes Precision Abort Stage 
  byte Mode;                          //0 = stage, 1 = docking, 2 = map
  uint16_t ControlGroup;          //action groups 1-10 in 2 bytes
  byte AdditionalControlByte1;        //other stuff
  byte AdditionalControlByte2;
  int16_t Pitch;                          //-1000 -> 1000
  int16_t Roll;                           //-1000 -> 1000
  int16_t Yaw;                            //-1000 -> 1000
  int16_t TX;                             //-1000 -> 1000
  int16_t TY;                             //-1000 -> 1000
  int16_t TZ;                             //-1000 -> 1000
  int16_t Throttle;                       //    0 -> 1000
} __attribute__ ((packed));

/** 
 * Current data about the vessel
 */
struct VesselData
{
  byte id;             //1   packet id
  float AP;            //2   apoapsis (m to sea level)
  float PE;            //3   periapsis (m to sea level)
  float SemiMajorAxis; //4   orbit semi major axis (m)
  float SemiMinorAxis; //5   orbit semi minor axis (m)
  float VVI;           //6   vertical velocity (m/s)
  float e;             //7   orbital eccentricity (unitless, 0 = circular, > 1 = escape)
  float inc;           //8   orbital inclination (degrees)
  float G;             //9   acceleration (Gees)
  int TAp;             //10  time to AP (seconds)
  int TPe;             //11  time to Pe (seconds)
  float TrueAnomaly;   //12  orbital true anomaly (degrees)
  float Density;       //13  air density (presumably kg/m^3, 1.225 at sea level)
  int period;          //14  orbital period (seconds)
  float RAlt;          //15  radar altitude (m)
  float Alt;           //16  altitude above sea level (m)
  float Vsurf;         //17  surface velocity (m/s)
  float Lat;           //18  Latitude (degree)
  float Lon;           //19  Longitude (degree)
  float LiquidFuelTot; //20  Liquid Fuel Total
  float LiquidFuel;    //21  Liquid Fuel remaining
  float OxidizerTot;   //22  Oxidizer Total
  float Oxidizer;      //23  Oxidizer remaining
  float EChargeTot;    //24  Electric Charge Total
  float ECharge;       //25  Electric Charge remaining
  float MonoPropTot;   //26  Mono Propellant Total
  float MonoProp;      //27  Mono Propellant remaining
  float IntakeAirTot;  //28  Intake Air Total
  float IntakeAir;     //29  Intake Air remaining
  float SolidFuelTot;  //30  Solid Fuel Total
  float SolidFuel;     //31  Solid Fuel remaining
  float XenonGasTot;   //32  Xenon Gas Total
  float XenonGas;      //33  Xenon Gas remaining
  float LiquidFuelTotS;//34  Liquid Fuel Total (stage)
  float LiquidFuelS;   //35  Liquid Fuel remaining (stage)
  float OxidizerTotS;  //36  Oxidizer Total (stage)
  float OxidizerS;     //37  Oxidizer remaining (stage)
  uint32_t MissionTime;//38  Time since launch (s)
  float deltaTime;     //39  Time since last packet (s)
  float VOrbit;        //40  Orbital speed (m/s)
  uint32_t MNTime;     //41  Time to next node (s) [0 when no node]
  float MNDeltaV;      //42  Delta V for next node (m/s) [0 when no node]
  float Pitch;         //43  Vessel Pitch relative to surface (degrees)
  float Roll;          //44  Vessel Roll relative to surface (degrees)
  float Heading;       //45  Vessel Heading relative to surface (degrees)
  uint16_t ActionGroups;  //46 status bit order:SAS, RCS, Light, Gear, Brakes, Abort, Custom01 - 10
} __attribute__ ((packed));

//Input Control Group positions
#define SAS 7
#define RCS 6
#define LIGHTS 5
#define GEAR 4
#define BRAKES 3
#define PRECISION 2
#define ABORT 1
#define STAGE 0

// Control / Action group statuses
#define AGSAS      0
#define AGRCS      1       
#define AGLight    2 
#define AGGear     3
#define AGBrakes   4 
#define AGAbort    5 
#define AGCustom01 6
#define AGCustom02 7 
#define AGCustom03 8 
#define AGCustom04 9 
#define AGCustom05 10
#define AGCustom06 11 
#define AGCustom07 12 
#define AGCustom08 13 
#define AGCustom09 14 
#define AGCustom10 15

class KSPSerialIO
{
	public:
                VesselData vessel;
                ControlPacket control;
                
		KSPSerialIO(long baud);
                boolean connected();
                int update();
                boolean controlStatus(byte n);
                void setControl(byte n, boolean value);
	private:
                HandShakePacket handshake_packet;
		void handshake();

                // For status
                boolean _connected;
                int const idle_limit = 5000;
                int const control_refresh = 25;
                long now, last_update_time, last_control_time;
                
                // For reading
                byte packet_id;
                uint8_t rx_len;
                uint8_t * address;         // Pointer to struct to be filled
                byte buffer[256];          // address for temporary storage and parsing buffer
                uint8_t structSize;        // Size of struct currently being filled
                uint8_t rx_array_inx;      // index for RX parsing buffer
                uint8_t calc_CS;	   // calculated Checksum
                
		void initTxPackets();
		boolean receive();
		void send(uint8_t *address, uint8_t length);
};

#endif
