
#ifndef SPL_H_
#define SPL_H_

typedef enum __attribute__ ((__packed__)) spl_command_type_t {
	trxvu_cmd_type,
	eps_cmd_type,
	telemetry_cmd_type,
	filesystem_cmd_type,
	obc_cmd_type
}spl_command_type_t;

//TODO: set all sub-types to be different from one another


//----------TRXVU SUB-TYPES
#define ACK_SUBTYPE			0x00
#define BEACON_SUBTYPE		0X01

//-----------------


//----------EPS SUB-TYPES

//-----------------

//----------TELEMETRY SUB-TYPES

//-----------------

//----------OBC SUB-TYPES

//-----------------




#endif /* SPL_H_ */
