
#ifndef TELEMETRYFILES_H_
#define TELEMETRYFILES_H_

//	---general
#define FILENAME_WOD_TLM				"wod"

//	---eps
#define FILENAME_EPS_ENG_TLM			"eEng"

//	---Solar
#define	FILENAME_SOLAR_PANELS_TLM		"slrPnl"

//	---trxvu
#define FILENAME_TX_REVC				"tx_revc"
#define FILENAME_RX_REVC				"rx_revC"

//  --- ants
#define FILENAME_ANTENNA_SIDE_A_TLM			"antSideA"
#define FILENAME_ANTENNA_SIDE_B_TLM			"antSideB"

typedef enum {
	tlm_wod,
	tlm_eps_eng,
	tlm_solar,
	tlm_tx_revc,
	tlm_rx_revc,
	tlm_antenna_side_a,
	tlm_antenna_side_b
}tlm_type;

#endif /* TELEMETRYFILES_H_ */
