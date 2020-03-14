#ifndef SATMACROS_H_
#define SATMACROS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"

#define MACRO_NUM_OF_MACROS 30
#define	MACRO_MAX_NUM_OF_COMMANDS_PER_MACRO 16

typedef enum{
	MACRO_SUCCESS,
	MACRO_FAIL,
	MACRO_CANNOT_EDIT_WAKUP,	// the wakup macro is a protected macro and cannot be overwritten
	MACRO_NOT_A_VALID_CMD,		// if a command is not a valid command, i.e. does not exist or with non-valid parameters
	MACRO_MACRO_TOO_LONG		// will only write a maximum of MACRO_MAX_NUM_OF_COMMANDS_PER_MACRO commands per macros
}macro_err;

typedef enum{
	MACRO_WAKEUP,
	MACRO_RESET,
	MACRO_AT_INDEX
}MacroTypes;

macro_err MacroExecuteMacro(MacroTypes macro_type,int index);

macro_err MacroAllowEdit(MacroTypes macro_type,int index);

macro_err MacroEditMacro(MacroTypes macro_type,int index,
		sat_packet_t *macro_cmd,unsigned int length);


#endif /* SATMACROS_H_ */
