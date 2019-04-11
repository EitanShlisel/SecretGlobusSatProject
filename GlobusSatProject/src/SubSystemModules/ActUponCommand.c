#include "ActUponCommand.h"
#include "CommandDictionary.h"

/*
 * 	trxvu_cmd_type,
 eps_cmd_type,
 telemetry_cmd_type,
 filesystem_cmd_type,
 obc__cmd_type
 */
int ActUponCommand(sat_command_t *cmd) {
	int err = 0;
	switch ((spl_command_type_t) cmd->cmd_type) {
	case trxvu_cmd_type:
		err = trxvu_command_router(cmd->cmd_subtype, cmd->data);
		break;
	case eps_cmd_type:
		err = eps_command_router(cmd->cmd_subtype, cmd->data);
		break;
	case telemetry_cmd_type:
		err = telemetry_command_router(cmd->cmd_subtype, cmd->data);
		break;
	case filesystem_cmd_type:
		err = filesystem_command_router(cmd->cmd_subtype, cmd->data);
		break;
	case obc_cmd_type:
		err = obc_command_router(cmd->cmd_subtype, cmd->data);
		break;
	}

	//TODO: ack logic
	return 0;
}
