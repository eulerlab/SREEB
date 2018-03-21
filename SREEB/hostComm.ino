/*--------------------------------------------------------------------------------
  Project:  SEB - Simple Extension Box
            Control external equippment using the Arduino-based Robot Controller
            Shield by Watterott
  Module:   hostComm
  Purpose:  Communication with host via serial port "SerHost"
  Author:   Copyright (c) 2015 Thomas Euler, CIN University of Tübingen. 
            All right reserved.
  History   v0.1 File created 
            v0.2 General parts moved to class RMsgClass, only host communication
                 related routines kept here
            v0.3 Now alternatively via ROS service and publisher     
            v0.4 Back to pure serial communication via a propriatory protocol
            v0.5 Moved most functionality to the RMsg class
            v0.6 First release
  --------------------------------------------------------------------------------*/  
  
//--------------------------------------------------------------------------------  
void COM_init ()
{
  // Initialize serial port
  //
  SerHost.begin(baudSerHost);
  SerHost.setTimeout(toutSerHost_ms);
  RMsg.setStream(&SerHost, NULL);
  delay(100);
}

/*--------------------------------------------------------------------------------
  Check message syntax (hardware-specific tokens)
  --------------------------------------------------------------------------------*/
bool  COM_checkMsg (Msg_t* msg, bool asCmd)
// Checks if the parameters are complete and fit to the message token; two cases
// are distinguished: 1) token as command and 2) token as reply, if required
{
  bool res = false;
  char ch;
  
  if(RMsg.checkMsg(msg, asCmd))
    return true;

  switch ((*msg).tok) {
    case TOK_SDM :
    case TOK_SDV :
      res = (((*msg).nParams == 2) && 
             ((*msg).paramCh[0] == 'P') && 
             ( (((*msg).paramCh[1] == 'M') && ((*msg).tok == TOK_SDM)) || 
               (((*msg).paramCh[1] == 'V') && ((*msg).tok == TOK_SDV))) &&
             ((*msg).nData[0] > 0) && 
             ((*msg).nData[0] < TOK_MaxData) &&
             ((*msg).nData[1] > 0) &&
             ((*msg).nData[1] < TOK_MaxData) &&
             ((*msg).nData[0] == (*msg).nData[1]));    
      break;

    case TOK_SDT :
      res = (((*msg).nParams == 2) && 
             ((*msg).paramCh[0] == 'P') && 
             ((*msg).paramCh[1] == 'S') &&
             ((*msg).nData[0] == 3) && 
             ((*msg).nData[1] == 2));    
      break;

    case TOK_CLR :
      res = ((*msg).nParams == 0);    
      break;
      
  }
  return res;
}  

/*--------------------------------------------------------------------------------
  Handling messages
  --------------------------------------------------------------------------------*/
boolean COM_handleMsg (Msg_t* msg)
// Handles some messages, can be called by the main loop to deal with standard
// messages. The result reflects if the message was not/could not be handled.
{
  boolean res   = true;
  byte    nErrs = 0;
  int     p1, p2, p3, pin, mode, j, val;
  
  switch ((*msg).tok) {
    case TOK_REM :
    case TOK_NONE:
  //case TOK_ACK :
  //case TOK_ERR :
      return res;
      
    case TOK_VER :
      RMsg.sendVerMsg(ModuleVer, getFreeSRAM());
      return res;

    case TOK_SDM :
      // Define I/O mode of up to 8 digital pins (=servo ports of the 
      // Watterott Robot Controller). 
      // with [x,..]  servo port index (1...8)
      //      [y,..]  mode, 0=input, 1=input_low, 2=output, 3=servo
      //              "input"    requires an external pulldown resistor
      //              "input_lo" uses the internal 2k pullup resistor 
      //              (=> closed == LOW!)
      // >SDM P=2,3 M=1,0
      //
      for(j=0; j<(*msg).nData[0]; j+=1) {
        p1   = (*msg).data[0][j] -1;
        mode = (*msg).data[1][j];
        if((p1 < 0) || (p1 >= RCS_maxServoPorts) || 
           (mode < 0) || (mode > MODE_last)) {
          nErrs += 1;
        }  
        else {  
          SPortList[p1].mode = mode;
          switch (mode) {
            case MODE_triggerIn : 
              pinMode(RobotCS.getArduinoPin(p1), INPUT);
              break;

            case MODE_triggerIn_Lo : 
              pinMode(RobotCS.getArduinoPin(p1), INPUT_PULLUP);
              break;

            case MODE_triggerOut : 
              pinMode(RobotCS.getArduinoPin(p1), OUTPUT);
              break;
            
            case MODE_servoOut : 
              SPortList[p1].pos1 = 90;
              SPortList[p1].pos2 = 90;        
              RobotCS.initServo(p1);
              break;
          }
        }
      }
      break;

    case TOK_SDV :
      // Set up to 8 digital pin (=servo ports of the Watterott Robot 
      // Controller) values simultanously
      // with [x,..]  servo port index (1...8)
      //      [y,..]  value, for output pins : 0=low, 1=high
      //                     for servo pins  : 0..255 as angle (not degrees)      
      // >SDV P=2,3 V=1,0
      //
      for(j=0; j<(*msg).nData[0]; j+=1) {
        p1   = (*msg).data[0][j] -1;
        val  = (*msg).data[1][j];
        if((p1 < 0) || (p1 >= RCS_maxServoPorts) || 
           (SPortList[p1].mode == MODE_unused) || (val < 0) || (val > 255)) {
          nErrs += 1;
        }  
        else {  
          switch (SPortList[p1].mode) {
            case MODE_triggerIn : 
            case MODE_triggerIn_Lo :
              break;

            case MODE_triggerOut : 
              digitalWrite(RobotCS.getArduinoPin(p1), val);
              break;
            
            case MODE_servoOut : 
              RobotCS.writeServo_Position(p1, val);
              break;
          }
        }
      }
      break;

    case TOK_SDT :
      // Define a servo port, two servo positions and a port that serves as 
      // input trigger to toggle between these two servo positions
      // with s,      servo port index (1...8), output to servo
      //      i,      servo port index (1...8), input to toggle servo
      //      o,      servo port index (1...8), output to indicate servo position a
      //      a,b     two servo positions (0...255) for i=0 and i=1
      // >SDT P=s,i,o S=a,b
      //
      p1   = (*msg).data[0][0] -1;
      p2   = (*msg).data[0][1] -1;
      p3   = (*msg).data[0][2] -1;      
      if((p1 < 0) || (p1 >= RCS_maxServoPorts) || 
         (p2 < 0) || (p2 >= RCS_maxServoPorts) || (p1 == p2) || 
         (p3 <-1) || (p3 >= RCS_maxServoPorts) || (p3 == p2) || (p3 == p1)) {
        nErrs += 1;
      }  
      else {   
        SPortList[p1].mode = MODE_servoOut;
        SPortList[p1].pos1 = (*msg).data[1][0];
        SPortList[p1].pos2 = (*msg).data[1][1];        
        RobotCS.initServo(p1);

        if(p3 >= 0) {
          SPortList[p3].mode = MODE_triggerOut;
          pinMode(RobotCS.getArduinoPin(p3), OUTPUT);
        }  
        SPortList[p2].mode = MODE_triggerIn;
        SPortList[p2].linkedTriggerOut = p3;        
        SPortList[p2].linkedServoOut   = p1; 
        pin = RobotCS.getArduinoPin(p2);
        pinMode(pin, INPUT_PULLUP);

        delay(10);
        val  = RobotCS.readDigitalDebounced(pin);
        if(val == HIGH)
          RobotCS.writeServo_Position(p1, SPortList[p1].pos1);
        else {
          RobotCS.writeServo_Position(p1, SPortList[p1].pos2);
        }
        if(p3 >= 0)
          digitalWrite(RobotCS.getArduinoPin(p3), val);
      }
      break;

    case TOK_CLR :
      // Clear all function entries
      // >CLR
      //
      for(j=0; j<RCS_maxServoPorts; j+=1) {
        SPortList[j].mode = MODE_unused;
      }
      RobotCS.reset();
      break;

    default      :
      res = false;
  }
  if(nErrs > 0)
    RMsg.sendConfirmMsg((*msg).tok, ERR_AtLeastOneInvalidParam, nErrs);
  else {
    RMsg.sendConfirmMsg((*msg).tok, ERR_None, 0);      
  }
  return res; 
}  

//--------------------------------------------------------------------------------
//
//--------------------------------------------------------------------------------  
extern int __bss_end;
extern int *__brkval;

int getFreeSRAM ()
{
  int free_memory;
  
  if((int)__brkval == 0)
    free_memory = ((int)&free_memory)  -((int)&__bss_end);
  else
    free_memory = ((int)&free_memory) -((int)__brkval);  
  return free_memory;
}
//--------------------------------------------------------------------------------

