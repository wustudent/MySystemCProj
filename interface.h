#ifndef INTERFACE_H_
#define INTERFACE_H_

enum Activity {
	ACT_CORRECT_PIN,
	ACT_WRONG_PIN,
	ACT_HOME,
	ACT_APP1,
	ACT_APP2,
	ACT_APP3,
	ACT_APP4,
	ACT_APP5,
	ACT_DEFAULT, //Doing Something in current App or Home Menu
	ACT_NOTHING //Doing nothing.
};

enum CmdPrompt {
	CMD_ENTER_PIN,
	CMD_CHOOSE_APP,
};
		
enum Notification {
	NOTI_LOW_BATTERY_LEVEL,
	NOTI_CORRECT_PIN,
	NOTI_WRONG_PIN
};


//Note when the Display is dark, the screen should be turned off.
enum Display {
	DISP_BRIGHT,
	DISP_MEDIUM,
	DISP_DARK
};

enum Screen {
	SCR_TURNED_OFF,
	SCR_PIN_REQUEST,
	SCR_HOME,
	SCR_APP1,
	SCR_APP2,
	SCR_APP3,
	SCR_APP4,
	SCR_APP5
};

#endif
