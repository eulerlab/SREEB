/*--------------------------------------------------------------------------------
  Project:  Watterott Robot Controller Shield
  Module:   RobotRS.cpp
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

  History:  see header file
  --------------------------------------------------------------------------------*/
#include "RobotCS.h"

//--------------------------------------------------------------------------------
#define  M_SLEEP      12

//--------------------------------------------------------------------------------
uint8_t  M_portPins[RCS_maxMotorPorts][2]
		     = {{2,3}, {4,5}, {7,6}, {8,9}};

int      S_portPins[RCS_maxServoPorts]
		     = {13, A0, A1, A2, A3, A6, A7, 11};
Servo    S_objs[RCS_maxServoPorts];

//================================================================================
// Class RobotCSClass - Methods
//--------------------------------------------------------------------------------
RobotCSClass::RobotCSClass () 
{
  reset();
}

//--------------------------------------------------------------------------------
void  RobotCSClass::reset ()
{
  int j;

  for(j=0; j<RCS_maxMotorPorts; j+=1) 
	  MPorts[j] = 0;
  for(j=0; j<RCS_maxServoPorts; j+=1) {
	  SPorts[j] = 0;
	  pinMode(S_portPins[j], INPUT);
  }
  isReady = true;
}

//--------------------------------------------------------------------------------
int   RobotCSClass::initMotor(int _iPort)
{
  if((!isReady) || (_iPort < 0) || (_iPort > RCS_maxMotorPorts))
    return -1;
  
  // Initialize motor port
  //
  pinMode(M_portPins[_iPort][0], OUTPUT);
  pinMode(M_portPins[_iPort][1], OUTPUT);  
  pinMode(M_SLEEP, OUTPUT);  
  
  digitalWrite(M_SLEEP, HIGH);   
  digitalWrite(M_portPins[_iPort][0], LOW);      
  MPorts[_iPort] = 1;

  return 0;
}

//--------------------------------------------------------------------------------
int   RobotCSClass::initServo(int _iPort)
{
  if((_iPort < 0) || (_iPort > RCS_maxServoPorts)) 
    return -1;

  if(S_objs[_iPort].attached()) {
    S_objs[_iPort].detach();
  }
  S_objs[_iPort].attach(S_portPins[_iPort]);
  SPorts[_iPort] = 1;
}

//--------------------------------------------------------------------------------
int   RobotCSClass::writeMotor_LEDDutyCycle(int _iPort, uint8_t _val)
{
  if((_iPort < 0) || (_iPort > RCS_maxMotorPorts) || (MPorts[_iPort] == 0)) 
    return -1;

  analogWrite(M_portPins[_iPort][1], _val);  
  return _val;
}

//--------------------------------------------------------------------------------
int   RobotCSClass::writeServo_Position(int _iPort, int _pos)
{
  if((_iPort < 0) || (_iPort > RCS_maxServoPorts) || (SPorts[_iPort] == 0)) 
    return -1;

  S_objs[_iPort].write(_pos);
  return _pos;  
}

//--------------------------------------------------------------------------------
int   RobotCSClass::getArduinoPin(int _iServoPort)
{
  return S_portPins[_iServoPort];
}

//--------------------------------------------------------------------------------
int   RobotCSClass::readDigitalDebounced(int _iServoPort)
{
  int val, j;

  val = 0;
  for(int j=0; j<5; j+=1) {
	  val += 10*digitalRead(S_portPins[_iServoPort]);
    delay(2);
  }
  if(val > 40)
    return 1;

  return 0;
}

//--------------------------------------------------------------------------------
// Preinstantiate Object
// 
RobotCSClass RobotCS = RobotCSClass();

//--------------------------------------------------------------------------------



