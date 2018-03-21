// ===============================================================================
// USER DEFINED ==>

/*--------------------------------------------------------------------------------
  Command token strings
  --------------------------------------------------------------------------------*/
extern char     msgTokens[TOK_LastIndex+1][TOK_StrLength+1]
                = {"REM", "VER", "ERR", "ACK", "STA", "DUM",
                   "SDM", "SDV", "SDT", "CLR", "I2W", "I2R",
                   "REC"
                  };

/*--------------------------------------------------------------------------------
  String resources
  (Stored in flash memory to save space in SRAM)
  --------------------------------------------------------------------------------*/
extern prog_char const _STR0[]  PROGMEM   = "Ready";
extern prog_char const _STR1[]  PROGMEM   = "...done";

extern PGM_P     const _Strs[]  PROGMEM   = {_STR0, _STR1};

// <==
// ===============================================================================
