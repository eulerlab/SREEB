/*--------------------------------------------------------------------------------
 Project:  RMsg -- another lightweight protocol to communicate with an Arduino
            via a serial port using a user-defined set of simple sting commands
  Module:   RMsg.h
  Purpose:  Common message handling routines (library)
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

  History   v0.1 File created
            v0.2 2010-08-12, made a library
            v0.3 2013-06-08, expanded message to 4x12 int parameters
                             -> 106 bytes/message
            v0.4 2014-02-28, moved more general routines from hostCom to this
                             library. Defined "stream" as general link to host
            v0.5 2014-10-01, no preinstantiated RMsg objects any longer
                             (#define RMsg_NoPreinstantiatedObject)
                             Allow to define a second stream (serial) exclusively
                             for REM-type messages
            v0.6 2015-08-28, small changes for "RMsg_generalIOExtension"
            v0.7 2016-01-07, expanded message size to 4x18 int parameters
                 2017-08-13, moved message size definition to RMsg_DEFINITIONs.h


  Class "RMsgClass" (only object "RMsg")
  --------------------------------------
  void  setStream (Stream *StreamCmd, Stream *StreamDebug);
    Set the stream that is connected to the host/client for all further
    communications. Default stream is "Serial" and that it is connected to the
    host. If StreamDebug==NULL, then StreamHost is used for all REM-type messages

  void  RMsgClass::setIsHost(bool _isHost)
    Change role of object owner and connected party
    _isHost == true            : object owner is host and connected to client
    _isHost == false (default) : object owner is client and connected to host

  void  beginMsg (token_t token)
    Starts a message to the host; an already started message is discarded

  void  appendDataToMsg (char sKey[], char  cFormat, int nData, int data[])
    Appends a data package to the current message
      sKey[]    := string, parameter key
      cFormat   := character, determines the representation format
                   MSG_DecFormatChr  '=', 12,3456(,79...)
                   MSG_WordFormatChr ':', FFFF(FFFF...)
                   MSG_ByteFormatChr '.', FF(FF...)
      nData     := number of data elements to append
      data      := data to append

  char* finalizeMsg ()
    Finalizes started message

  void  sendMsg()
  void  sendMsg (Msg_t msg)
    Sends the last finalized message or "msg"

  char* convertMsgToStr (Msg_t msg)
    Converts a message structure into a string message

  void  clearMsg (Msg_t* msg);
    Clear message structure

  char* composeRemMsg (int strCode)
    Compose a remark message

  void  beginRemMsg ()
  void  appendStrToRemMsg (char *s)
  void  sendRemMsg ()

  void  sendMsg ()
    Send the last composed message

  void  sendMsg (Msg_t msg)
  Send the message contained in the message structure

  void  sendConfimMsg (token_t tok, int errCode, int errValue)
    Depending on error code, it sends an error message or an acknowledgement
    to the host

  void  sendRemMsg(int strCode);
  void  sendVerMsg(int ver, int freeRAM);
  Shortcuts for different kinds of messages.

  token_t readMsgFromStream(Msg_t* msg)
    Check of data is available on the comm port connected to the host and parse
    the data. Returns a command token, if a complete message was recognized, and
    the message data, including command and parameter fields, in "msg". Returns
    immediately of no data is available.
    Except for checking the validity of the token and the message format, this
    routine does not check if the parameter fields match the command. This needs
    to be taken care of by the caller.
    For message structure see class RMsgClass.

  char* getPtrToInBuf ()

  bool  checkMsg (Msg_t* msg, bool asCmd)
    Checks if the parameters are complete and fit to the message token; two cases
    are distinguished: 1) token as command and 2) token as reply, if required

  --------------------------------------------------------------------------------*/
#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif
#if defined(R6bot_SafeSpace)
#else
  #include <avr/pgmspace.h>
#endif
#ifndef  RMsg_h
#define  RMsg_h

#define  prog_char  char PROGMEM
#define  RMsg_NoPreinstantiatedObject

#include "RMsg_DEFINITIONS.h"

/*--------------------------------------------------------------------------------
  Command-related definitions
  --------------------------------------------------------------------------------*/
#define         TOK_StrLength          3
#define         TOK_MinParamStrLength  3
/* Moved to RMsg_DEFINITIONS.h
#define         TOK_MaxParams          4
#define         TOK_MaxData            18    // 12
*/
#define         TOK_MaxMsgLen_bytes    2 +2*TOK_MaxParams +2*TOK_MaxParams*TOK_MaxData
#define         TOK_isCommand          true
#define         TOK_isReply            true

extern char     msgTokens[TOK_LastIndex+1][TOK_StrLength+1];

typedef byte    token_t;
typedef struct  {
  token_t       tok;                                //  1
  byte          nParams;                            //  1
  char          paramCh[TOK_MaxParams];             //  4  = 1*4
  byte          nData[TOK_MaxParams];               //  4  = 1*4
  int           data[TOK_MaxParams][TOK_MaxData];   // 144 = 2*4*18
                } Msg_t;                            // 154
typedef byte    MsgBuf_t[TOK_MaxMsgLen_bytes];

/*--------------------------------------------------------------------------------
  Error codes
  --------------------------------------------------------------------------------*/
#define         ERR_None                            0
#define         ERR_CmdNotRecognized                1
#define         ERR_AtLeastOneInvalidParam          3
#define         ERR_InvalidOrTooFewParams           4
#define         ERR_CmdNotImplemented               5
#define         ERR_DeviceNotReady                  6
#define         ERR_I2C_Error                       20
                /* 1, data too long to fit in transmit buffer
                   2, received NACK on transmit of address
                   3, received NACK on transmit of data
                   4, other error
                */
/*--------------------------------------------------------------------------------
  Host-Controller communication message definitons
  --------------------------------------------------------------------------------*/
/* Moved to RMsg_DEFINITIONS.h
#define         MSG_MaxInLen           512 // 127
#define         MSG_MaxOutLen          512 // 127
*/
#define         MSG_MinInLen           3
#define         MSG_MaxConvBufLen      10
#define         MSG_StartChr_Client    '<'
#define         MSG_StartChr_Host      '>'
#define         MSG_SpacerChr          " "
#define         MSG_SepChr             ","
#define         MSG_EndChr             ';'

#define         MSG_DecFormatChr       '='
#define         MSG_WordFormatChr      ':'
#define         MSG_ByteFormatChr      '.'

//--------------------------------------------------------------------------------
// Class RMsg
//--------------------------------------------------------------------------------
class RMsgClass
{
  public:
    RMsgClass();
    void    setStream(Stream *StreamCmd, Stream *StreamDebug);
    void    setIsHost(bool _isHost);

    void    beginMsg(token_t token);
    void    appendDataToMsg(char sKey[], char  cFormat, int nData, int data[]);
    char*   finalizeMsg();
    char*   convertMsgToStr(Msg_t msg);
    char*   composeRemMsg(int strCode);
    void    clearMsg(Msg_t* msg);

    token_t readMsgFromStream(Msg_t* msg);
    char*   getPtrToInBuf();
    bool    checkMsg(Msg_t* msg, bool asCmd);

    void    sendMsg();
    void    sendMsg(Msg_t msg);
    void    sendConfirmMsg(token_t tok, int errCode, int errValue);
    void    sendRemMsg(int strCode);
    void    sendRemMsg(char *s);
    void    beginRemMsg();
    void    appendStrToRemMsg(char *s);
    void    sendRemMsg();
    void    sendVerMsg(int ver, int freeRAM);

  private:
    char    msgOutBuf[MSG_MaxOutLen +1];
    int     iMsgOutBuf;
    char    Buf[MSG_MaxInLen +1];
    int     nBuf;
    char    convStrBuf[MSG_MaxConvBufLen];
    Stream* cmdStream;
    Stream* debugStream;
    boolean isMsgStarted;
    bool    isClient;
    char    chStartClient, chStartHost;
};

#ifndef RMsg_NoPreinstantiatedObject
extern RMsgClass  RMsg;
#endif

//--------------------------------------------------------------------------------
#endif
