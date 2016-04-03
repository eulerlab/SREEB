/*--------------------------------------------------------------------------------
  Project:  SREEB - Simple Research Equipment Extension Box
            Control external scientific equippment using the Arduino-based Robot 
            Controller Shield from Watterott
            https://github.com/watterott/Robot-Controller/
  Module:   SREEB.ino 
  Author:   Copyright (c) 2015 Thomas Euler, CIN University of Tübingen. 
            All right reserved.
  History:  v0.6 (2015-08-31) first release
            v0.7 (2016-02-16) Added routines to read i2c data by Andre M. Chagas
  --------------------------------------------------------------------------------*/
#include <RString.h>
#include <RMsg.h>
#include <Wire.h>
#include "Servo.h"
#include "RobotCS.h"


//--------------------------------------------------------------------------------
#define  SerHost           Serial
#define  ModuleVer         3
#define  baudSerHost       57600
#define  toutSerHost_ms    500
#define  toutLastCmd_ms    2000

#define  MODE_unused       -1
#define  MODE_triggerIn    0  // external pulldown resistor needed!!
#define  MODE_triggerIn_Lo 1  // using internal pullup resistor
#define  MODE_triggerOut   2
#define  MODE_servoOut     3
#define  MODE_last         3

/*--------------------------------------------------------------------------------
  Global general variables
  --------------------------------------------------------------------------------*/
boolean         isReady;
int             val, p, pOut;
int          c1;
// Related to messaging
//
RMsgClass       RMsg = RMsgClass();
Msg_t           currMsg, currRpl;
token_t         currTok; 

// Related to implementing control functions
//
typedef struct  {
  int           mode;
  int           pos1, pos2;
  int           linkedServoOut, linkedTriggerOut;
                } SPortEntry_t;
SPortEntry_t    SPortList[RCS_maxServoPorts];

//================================================================================
// METHODS
//--------------------------------------------------------------------------------
void setup() 
{
  // Initialize
  //
  isReady = false;
  for(int j=0; j<RCS_maxServoPorts; j+=1) {
    SPortList[j].mode = MODE_unused;
  }
  
  // Initialize modules
  //
  COM_init();
  I2C_init();
  // ...
  
  isReady = true;
  RMsg.sendRemMsg(STR_Ready);
}

//================================================================================
void loop() 
{
  // Check for message from host (via serial/USB) 
  //
  currTok = RMsg.readMsgFromStream(&currMsg);
  if(currTok != TOK_NONE) {
    //Serial.println(RMsg.getPtrToInBuf());
  
    if(!RMsg.checkMsg(&currMsg, TOK_isCommand) &&
       !COM_checkMsg(&currMsg, TOK_isCommand)) 
    {
      // Error: Command recognized but parameters invalid/incomplete
      //
      RMsg.sendConfirmMsg(TOK_NONE, ERR_AtLeastOneInvalidParam, 0);
      currMsg.tok = TOK_NONE;
    }    
    else {
      // Received message from host ...
      //
      COM_handleMsg(&currMsg);
    }  
  }
  // Execute user-defined functions 
  //
  for(p=0; p<RCS_maxServoPorts; p+=1) {
    switch (SPortList[p].mode) {
      case MODE_unused       :
      case MODE_triggerOut   :
      case MODE_servoOut     :
        break;

      case MODE_triggerIn    : 
      case MODE_triggerIn_Lo :
        val  = RobotCS.readDigitalDebounced(p);
        pOut = SPortList[p].linkedServoOut;
        if(val == HIGH)
          RobotCS.writeServo_Position(pOut, SPortList[pOut].pos1);
        else {
          RobotCS.writeServo_Position(pOut, SPortList[pOut].pos2);
        }
        pOut = SPortList[p].linkedTriggerOut;
        if(pOut >= 0)
          digitalWrite(RobotCS.getArduinoPin(pOut), val);
        break;
    }
  }
}
//--------------------------------------------------------------------------------

