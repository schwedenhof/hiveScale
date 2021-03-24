

#ifndef __COMPONENTS_TERMINALHANDLER_H_
#define __COMPONENTS_TERMINALHANDLER_H_

/** TERMINAL_MAX_CMD_LEN characters for the command string*/
#define TERMINAL_MAX_CMD_LEN 5
/** TERMINAL_MAX_CMD_HELP_LEN characters for the help message*/
#define TERMINAL_MAX_CMD_HELP_LEN 35
/** TERMINAL_MAX_CMDLINE_LEN characters for command length*/
#define TERMINAL_MAX_CMDLINE_LEN 40



/*****************************************************************************/
/*! ENUMERATIONS of terminal commands                                        */
/*****************************************************************************/
enum  {
  TERM_UNKNOWN      = 0x00,
  TERM_HELP,
  TERM_BLINK_ON,
  TERM_BLINK_OFF,
  TERM_RTC,
  TERM_ALARM,
  TERM_PUSH,
  TERM_STARTAPP,
  TERM_SETPARAM,
  TERM_SETTIME,
  TERM_AT,
  TERM_BAT,
  TERM_BAT_LOW,
  TERM_BAT_HIGH,
  TERM_HX,
  TERM_TARA,
  TERM_CALI,
  TERM_SIM800_ON,
  TERM_SIM800_OFF,
  TERM_BUFLIST,
  TERM_BUFFLUSH,
  TERM_BUFCLR,
  TERM_BUFINFO,
  TERM_REMINFO,
  TERM_REMLOAD,
  TERM_REMSTORE,
  TERM_LAST,     /** just for determining number of commands in this list */
  TERM_STOP,    /* command which cannot be called from console */
};


enum{
	PARAM_WAKEUP_PERIOD   = 1,

	PARAM_ALARM_HRS       = 10,
	PARAM_ALARM_MIN       = 11,
	PARAM_ALARM_INCREMENT = 12,

	PARAM_TARA            = 20,
	PARAM_CALI            = 21,

	PARAM_ADC_LOW         = 30,
	PARAM_ADC_HIGH        = 31,
	PARAM_VOLT_LOW        = 32,
	PARAM_VOLT_HIGH       = 33,

	PARAM_APN             = 40,
	PARAM_URL             = 41,

};

/*****************************************************************************/
/*! typedef terminal command structure                                       */
/*****************************************************************************/
typedef struct TERM_TERMINAL_CMD_Ttag {
  uint32_t iCode;
  char szString[TERMINAL_MAX_CMD_LEN];
//disabled to save memory//  char szHelp[TERMINAL_MAX_CMD_HELP_LEN];
} __attribute__((__packed__)) TERM_TERMINAL_CMD_T;


/*****************************************************************************/
/*! FUNCTION PROTOTYPES                                                      */
/*****************************************************************************/

void terminalHandler(uint8_t* abBuffer, int iLength);
void Term_TerminalCommandHandler(int uiTerminalCommandCode, int argc, char** argv );

void terminalInitCmdList(void);


#endif /* __COMPONENTS_TERMINALHANDLER_H_ */
