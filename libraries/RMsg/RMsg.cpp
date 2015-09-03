/*--------------------------------------------------------------------------------
  Project:  RMsg -- another lightweight protocol to communicate with an Arduino
            via a serial port using a user-defined set of simple sting commands
  Module:   RMsg.cpp
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

  History:  see header 
  --------------------------------------------------------------------------------*/
#include <Stream.h>
#include "RMsg.h"
#include "RString.h"
  
#include "RMsg_RESOURCES.h"

//================================================================================
// Class RMsg - Methods
//--------------------------------------------------------------------------------
RMsgClass::RMsgClass () 
{
  msgOutBuf[0]  = '\0';
  iMsgOutBuf    = 0;
  isMsgStarted  = false;
  isClient		  = true;
  chStartClient = MSG_StartChr_Client;
  chStartHost   = MSG_StartChr_Host;
  cmdStream     = &Serial;
  debugStream   = NULL;
}

//--------------------------------------------------------------------------------
void  RMsgClass::setStream (Stream *StreamCmd, Stream *StreamDebug)
// Set input/output stream and, if required, an extra output stream for debug
// messages (currently only messages of the REM-type)
{
	cmdStream = StreamCmd;
	if (StreamDebug != NULL)
		debugStream  = StreamDebug;
	else
		debugStream  = cmdStream;
}

//--------------------------------------------------------------------------------
void  RMsgClass::setIsHost(bool _isHost)
// Set if client or host role
{
	isClient	= !_isHost;
	if (!isClient) {
		chStartClient = MSG_StartChr_Host;
		chStartHost   = MSG_StartChr_Client;
	}
}

//--------------------------------------------------------------------------------
void  RMsgClass::beginMsg (token_t token)
// Starts a message to the host; an already started message is discarded
{
  RString  MsgOutStr(msgOutBuf, MSG_MaxOutLen, 0);

  if(isMsgStarted) {
    // Discard previous message
    //
    // ...
    isMsgStarted  = false;
  }
  if((token >= 0) && (token <= TOK_LastIndex)) {
    sprintf_P(convStrBuf, PSTR("%c%s"), chStartClient, msgTokens[token]);
    MsgOutStr     = convStrBuf;
    iMsgOutBuf    = MsgOutStr.length();
    isMsgStarted  = true;  
  }
}

//--------------------------------------------------------------------------------
void  RMsgClass::appendDataToMsg (char sKey[], char  cFormat, int nData, int data[])
// Appends a data package to the current message
//   sKey[]    := string, parameter key
//   cFormat   := character, determines the representation format
//                MSG_DecFormatChr  '=', 12,3456(,79...)
//                MSG_WordFormatChr ':', FF(FF...)
//                MSG_ByteFormatChr '.', FFFF(FFFF...)
//   nData     := number of data elements to append
//   data      := data to append
{
  RString  MsgOutStr(msgOutBuf, MSG_MaxOutLen, iMsgOutBuf);

  if((isMsgStarted) && (strlen(sKey) > 0) &&
     ((cFormat == MSG_DecFormatChr) || (cFormat == MSG_WordFormatChr) || 
      (cFormat == MSG_ByteFormatChr))) 
  {
    MsgOutStr += MSG_SpacerChr; 
    MsgOutStr += sKey;
    MsgOutStr += cFormat;
    switch (cFormat) {
      case MSG_DecFormatChr:
        for(int i=0; i<nData; i+=1) {
          itoa(data[i], convStrBuf, 10);
          MsgOutStr += convStrBuf;
          if(i < (nData-1)) { 
            MsgOutStr += ',';
          }  
        } 
        break;
        
      case MSG_WordFormatChr:
        for(int i=0; i<nData; i+=1) {
          sprintf_P(convStrBuf, PSTR("%4X"), data[i]);
          MsgOutStr += convStrBuf;
        } 
        break;
        
      case MSG_ByteFormatChr:
        for(int i=0; i<nData; i+=1) {
          sprintf_P(convStrBuf, PSTR("%2X"), constrain(data[i], 0, 255));
          MsgOutStr += convStrBuf;
        } 
        break;
    }
    iMsgOutBuf = MsgOutStr.length();
  }
}

//--------------------------------------------------------------------------------
char* RMsgClass::finalizeMsg ()
// Finalizes started message
{
  RString  MsgOutStr(msgOutBuf, MSG_MaxOutLen, iMsgOutBuf);

  if(isMsgStarted) {
    MsgOutStr   += MSG_EndChr;
    isMsgStarted = false;
    return msgOutBuf;
  }
  else
    return NULL;
}

//--------------------------------------------------------------------------------
char* RMsgClass::convertMsgToStr (Msg_t msg)
// Converts a message structure into a string message
{
  byte  j;
  char  s[2] = " ";

  beginMsg(msg.tok);
  for(j=0; j<msg.nParams; j++) {
    s[0] = char(msg.paramCh[j]);
    appendDataToMsg(s, MSG_DecFormatChr, msg.nData[j], msg.data[j]);
  }
  finalizeMsg();
  return msgOutBuf;
}

//--------------------------------------------------------------------------------
void RMsgClass::clearMsg (Msg_t* msg)
// Clears a message srructure
{
  memset(msg, 0, sizeof(Msg_t));
}

//--------------------------------------------------------------------------------
char* RMsgClass::composeRemMsg (int strCode)
// Compose a remark message
{
  char     strBuf[STR_MaxLength];    
  RString  MsgOutStr(msgOutBuf, MSG_MaxOutLen, 0);

  MsgOutStr    = chStartClient;
  MsgOutStr   += msgTokens[TOK_REM];
  MsgOutStr   += MSG_SpacerChr;
  strcpy_P(strBuf, (char*)pgm_read_word(&(_Strs[strCode])));
  MsgOutStr   += strBuf;
  MsgOutStr   += MSG_EndChr;
  isMsgStarted = false;

  return msgOutBuf;
}

//--------------------------------------------------------------------------------
void RMsgClass::beginRemMsg ()
{
	(*debugStream).print(chStartClient);
	(*debugStream).print(msgTokens[TOK_REM]);
	(*debugStream).print(MSG_SpacerChr);
}

void RMsgClass::appendStrToRemMsg (char *s)
{
	(*debugStream).print(s);
}

void RMsgClass::sendRemMsg ()
{
	(*debugStream).println(MSG_EndChr);
}

//--------------------------------------------------------------------------------
void RMsgClass::sendMsg()
{
  char* msgStr = finalizeMsg();
	if (msgStr != NULL) {
		(*cmdStream).println(msgStr);
	}
}

void RMsgClass::sendMsg(Msg_t msg)
{
	char* msgStr = convertMsgToStr(msg);
	if (msgStr != NULL) {
		(*cmdStream).println(msgStr);
	}
}

//--------------------------------------------------------------------------------
void RMsgClass::sendConfirmMsg (token_t tok, int errCode, int errValue)
// Depending on error code, it sends an error message or an acknowledgement 
// to the host
{
	int   data[2] = {byte(tok), 0};
	char* msgStr;

	if (errCode == ERR_None) {
		beginMsg(TOK_ACK);
		appendDataToMsg("C", MSG_DecFormatChr, 1, data);
	}
	else {
		beginMsg(TOK_ERR);
		appendDataToMsg("C", MSG_DecFormatChr, 1, data);
		data[0] = errCode;
		data[1] = errValue;
		appendDataToMsg("E", MSG_DecFormatChr, 2, data);
	}
	msgStr = finalizeMsg();
	if (msgStr != NULL) {
		(*cmdStream).println(msgStr);
	}
}

//--------------------------------------------------------------------------------
void RMsgClass::sendRemMsg (int strCode)
{
	(*debugStream).println(composeRemMsg(strCode));
}

void RMsgClass::sendRemMsg(char *s)
{
	(*debugStream).print(chStartClient);
	(*debugStream).print("REM ");
	(*debugStream).print(s);
	(*debugStream).println(MSG_EndChr);
}

//--------------------------------------------------------------------------------
void RMsgClass::sendVerMsg (int ver, int freeRAM)
{
	int data[1];

	beginMsg(TOK_VER);
	data[0] = ver;
	appendDataToMsg("V", MSG_DecFormatChr, 1, data);
	data[0] = freeRAM;
	appendDataToMsg("M", MSG_DecFormatChr, 1, data);
	sendMsg();
}

//--------------------------------------------------------------------------------
token_t RMsgClass::readMsgFromStream (Msg_t* msg)
// Check of data is available on the stream connected to the host and parse
// the data. Returns a command token, if a complete message was recognized, and
// the message data, including command and parameter fields, in "msg". Returns 
// immediately of no data is available.
//
// Except for checking the validity of the token and the message format, this
// routine does not check if the parameter fields match the command. This needs
// to be taken care of by the caller.
// For message structure see class RMsg
//
{
	char    ch;
	char    *pTok, *pCh, *pBuf, *pErrCh;
//char    Buf[MSG_MaxInLen];
//int     nBuf;
	byte    i;
	int     j;
	int     convRes;
	boolean isMsgComplete = false;

	if ((msg == NULL) || (!(*cmdStream).available()))
		return TOK_NONE;

	// Initialize message
	//
	(*msg).tok = TOK_NONE;
	(*msg).nParams = 0;
	for (i = 0; i<TOK_MaxParams; i++)
		(*msg).nData[i] = 0;

	// Read bytes, if any, from host and process ...
	//
	while ((*cmdStream).available()) {
		ch = (*cmdStream).read();

		if (ch == chStartHost) {
			// Start of message found, read more data until message end character 
			// is detected or a time-out occurs ...
			//
			nBuf = 0;
			nBuf = (*cmdStream).readBytesUntil(MSG_EndChr, Buf, MSG_MaxInLen);
			isMsgComplete = (nBuf >= MSG_MinInLen) && (nBuf < MSG_MaxInLen);
			if (isMsgComplete) {
				// Success
				// 
				Buf[nBuf++] = 0;
			}
			break;
		}
	}
	if (isMsgComplete) {
		// Identify token ...
		//
		(*msg).tok = TOK_NONE;
		ch = Buf[TOK_StrLength];
		Buf[TOK_StrLength] = 0;
		for (j = 0; j <= TOK_LastIndex; j++) {
			if (strcasecmp(msgTokens[j], Buf) == 0) {
				(*msg).tok = j;
				break;
			}
		}
		Buf[TOK_StrLength] = ch;
		if ((*msg).tok == TOK_NONE) {
			// Token could not be identified, discard message ...
			//
			sendConfirmMsg(TOK_NONE, ERR_CmdNotRecognized, 0);
		}
		else {
			// Check if the message contains parameter
			//
			strupr(Buf);
			if (nBuf >= (TOK_StrLength + TOK_MinParamStrLength + 1)) {
				// Parse message parameters ...
				//
				pBuf = &Buf[TOK_StrLength + 1];
				pTok = strtok_r(pBuf, MSG_SpacerChr, &pCh);

				while (pTok != NULL) {
					if (strlen(pTok) >= TOK_MinParamStrLength) {
						// String of sufficient length for parameter found
						//
						(*msg).paramCh[(*msg).nParams] = pTok[0];
						switch (pTok[1]) {
						case MSG_DecFormatChr:
							// Parse comma separated decimal parameters ...
							//
							pTok += 2;
							convRes = 0;
							do {
								i = (*msg).nData[(*msg).nParams];
								(*msg).data[(*msg).nParams][i] = strtol(pTok, &pErrCh, 10);
								if (pErrCh == pTok) {
									// Nothing to convert, abort ...
									// 
									convRes = -1;
								}
								else {
									// Conversion was successful
									//
									(*msg).nData[(*msg).nParams]++;
									if ((*msg).nData[(*msg).nParams] == TOK_MaxData)
										break;

									// Check whether more data entries are in the list or not
									//
									if (*pErrCh == 0)
										convRes = 1;
									else {
										pTok = pErrCh + 1;
									}
								}
							} while (convRes == 0);
							break;

						case MSG_WordFormatChr:
						case MSG_ByteFormatChr:
							//***************
							//**** TODO *****
							//***************
							break;
						}
						(*msg).nParams++;
					}
					if ((*msg).nParams == TOK_MaxParams)
						pTok = NULL;
					else
						pTok = strtok_r(NULL, MSG_SpacerChr, &pCh);
				}
			}
		}
	}
	return (*msg).tok;
}

char* RMsgClass::getPtrToInBuf()
{
	return Buf;
}

//--------------------------------------------------------------------------------
bool  RMsgClass::checkMsg (Msg_t* msg, bool asCmd)
// Checks if the parameters are complete and fit to the message token; two cases
// are distinguished: 1) token as command and 2) token as reply, if required
{
  bool res = false;
  char ch;
  
	switch ((*msg).tok) {
		case TOK_REM:
		case TOK_NONE:
		case TOK_STA:
			res = ((*msg).nParams == 0);
			break;

		case TOK_DUM:
			res = true;
			break;

		case TOK_VER:
			if (asCmd)
				res = ((*msg).nParams == 0);
			else
				res = (((*msg).nParams == 2) &&
							((*msg).paramCh[0] == 'V') && ((*msg).paramCh[1] == 'M') &&
							((*msg).nData[0] == 1) && ((*msg).nData[1] == 1));
			break;

		case TOK_ERR:
		  res = (((*msg).nParams == 2) &&
						((*msg).paramCh[0] == 'C') && ((*msg).nData[0] == 1) &&
						((*msg).paramCh[1] == 'E') && ((*msg).nData[1] == 2));
		  break;

		case TOK_ACK:
			res = (((*msg).nParams == 1) &&
						((*msg).paramCh[0] == 'C') && ((*msg).nData[0] == 1));
			break;
  }
  return res;
}  

//--------------------------------------------------------------------------------
// Preinstantiate Object
// 
#ifndef RMsg_NoPreinstantiatedObject
RMsgClass RMsg = RMsgClass();
#endif

//--------------------------------------------------------------------------------



