/*--------------------------------------------------------------------------------
  Project:  Watterott Robot Controller Shield
  Module:   RobotRS.h
  Purpose:  basic library to simplify use of Watterott's Robot Controller Shield.
            NOTE: So far only partial support of the shield's features.
  Author:   Copyright (c) 2015 Thomas Euler. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  History:  v0.1 File created, very rudimentary support so far

  --------------------------------------------------------------------------------*/
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#ifndef    RobotCS_h
#define    RobotCS_h
#include   "Servo.h"

//--------------------------------------------------------------------------------
#define    RCS_maxMotorPorts   4
#define    RCS_maxServoPorts   8

#define    RCS_M1              0
#define    RCS_M2              1
#define    RCS_M3              2
#define    RCS_M4              3

#define    RCS_S1              0
#define    RCS_S2              1
#define    RCS_S3              2
#define    RCS_S4              3
#define    RCS_S5              4
#define    RCS_S6              5
#define    RCS_S7              6
#define    RCS_S8              7

//--------------------------------------------------------------------------------
// Class RobotCSClass
//--------------------------------------------------------------------------------
class RobotCSClass
{
  public:
    RobotCSClass(); 
    
    void    reset ();
	  int     initMotor(int _iPort);
	  int     writeMotor_LEDDutyCycle(int _iPort, uint8_t _val);
	
	  int     initServo(int _iPort);
	  int     writeServo_Position(int _iPort, int _pos);

	  int     getArduinoPin(int _iServoPort);
	  int     readDigitalDebounced(int _iServoPort);

  private: 
    bool    isReady;    
    uint8_t MPorts[RCS_maxMotorPorts];
    uint8_t SPorts[RCS_maxServoPorts];
};    

extern RobotCSClass  RobotCS;

//--------------------------------------------------------------------------------
#endif
