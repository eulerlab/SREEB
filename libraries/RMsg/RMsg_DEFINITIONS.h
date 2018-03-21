/*--------------------------------------------------------------------------------
  Message and host communication related definitions

  GENERAL:
  -------
  - Remark, as a simple text message
    </>REM ArbitraryText;

  - Information about software version (V) and free space in SRAM (M) in bytes
    >VER;
    <VER V=100 M=1234;

  - Returns error code regarding the last message
    with x,      command index, 255=not recognized
         y,      error code
         z,      error value (further specifies the error, e.g. an I2C error)
    <ERR C=x E=y,z;

  - Acknowledges that command has been executed; when no error occurred and the
    command has no specific response message defined (other than e.g. >ver;)
    with x,      command index
    </>ACK C=x;



  Hardware-specific:
  -----------------

  * Define a servo port, two servo positions and a port that serves as input
    to toggle between these two servo positions
	  with s,      servo port index (1...8), output to servo
	       i,      servo port index (1...8), input to toggle servo
	       o,      servo port index (1...8), output to indicate servo position a
		     a,b     two servo positions (0...255) for i=0 and i=1
    >SDT P=s,i,o S=a,b;

  * Define I/O mode of up to 8 digital pins (=servo ports of the Watterott
    Robot Controller).
    with [x,..]  servo port index (1...8)
         [y,..]  mode, mode, 0=input, 1=input_low, 2=output, 3=servo
		             "input"    requires an external pulldown resistor
				         "input_lo" uses the internal 2k pullup resistor
				          (=> closed == LOW!)
    >SDM P=x1,x2... M=y1,y2...;

  * Set up to 8 digital pin (=servo ports of the Watterott Robot Controller)
    values simultanously
    with [x,..]  servo port index (1...8)
         [y,..]  value, for output pins : 0=low, 1=high
                        for servo pins  : 0..255 as angle (not degrees)
    >SDV P=x1,x2... V=y1,y2...;

  * Clear all function entries
    >CLR;

  * Start/stop sampling vom analog inputs #0 and 1
    with rate,   0=stop, >0 rate in [us]
         range,  1=1.1V, analogReference(INTERNAL)
                 3=2.3V, analogReference(EXTERNAL), with Aref->3.3V
                 5=5.0V, analogReference(DEFAULT)
    >REC r=rate,range;


  --------------------------------------------------------------------------------*/
#ifndef RMsg_COMMANDS_h
#define RMsg_COMMANDS_h

#define TOK_MaxParams          3
#define TOK_MaxData            8
#define MSG_MaxInLen         127
#define MSG_MaxOutLen        127

#define TOK_NONE             255
#define TOK_REM                0
#define TOK_VER                1
#define TOK_ERR                2
#define TOK_ACK                3
#define TOK_STA                4
#define TOK_DUM                5

// ===============================================================================
// USER DEFINED ==>

/*--------------------------------------------------------------------------------
  Command tokens
  --------------------------------------------------------------------------------*/
#define TOK_SDM                6
#define TOK_SDV                7
#define TOK_SDT                8
#define TOK_CLR				         9
#define TOK_I2W                10
#define TOK_I2R                11
#define TOK_REC                12
#define TOK_LastIndex          12

/*--------------------------------------------------------------------------------
  Status codes
  --------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------
  String resources
  (Stored in flash memory to save space in SRAM)
  --------------------------------------------------------------------------------*/
#define STR_MaxLength         32
#define STR_Ready              0
#define STR_Done               1

extern  prog_char const _STR0[]  PROGMEM;
extern  prog_char const _STR1[]  PROGMEM;
extern  PGM_P     const _Strs[]  PROGMEM;

// <==
// ===============================================================================
#endif
