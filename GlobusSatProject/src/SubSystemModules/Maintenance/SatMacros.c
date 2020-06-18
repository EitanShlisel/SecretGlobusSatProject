#include "SatMacros.h"
#include "hal/boolean.h"

Boolean8bit macro_write_enabled[MACRO_NUM_OF_MACROS] = {0};


macro_err MacroExecuteMacro(MacroTypes macro_type,int index){
	return 0;
}

macro_err MacroAllowEdit(MacroTypes macro_type,int index){
	return 0;
}

macro_err MacroEditMacro(MacroTypes macro_type,int index,
		sat_packet_t *macro_cmd,unsigned int length){

	return 0;
}
