//#define DEBUG					//for tracing all internal prints, following are module tracing
	//#define DEBUG_BATTERY	
	//#define DEBUG_OS
	//#define DEBUG_SCREEN
	//#define DEBUG_POWERBUTTON
#include "systemc.h"
#include "interface.h"
#include "user.h"
#include "phone.h"

int sc_main(int argc, char* argv[])
{
	// generating the sc_signal
	sc_fifo<Activity> act_fifo;
	sc_fifo<CmdPrompt> cmdprompt_fifo;
	sc_fifo<Notification> noti_fifo;
	
	sc_signal<bool> pt_sig("pt_sig");
	sc_signal<bool> ps_sig("ps_sig");
	sc_signal<bool> pow_sig("pow_sig");
	sc_signal<Display, SC_MANY_WRITERS> disp_sig("disp_sig");	//use SC_MANY_WRITERS to avoid check
	sc_signal<Screen, SC_MANY_WRITERS> scr_sig("scr_sig");
	sc_signal<int> batt_sig("batt_sig"); 

	//add any internal signals here
		//internal signals are inside the phone and SHOULD NOT be placed here!
	// generating the modules
	User user1("User1");
	// add smartphone modules here
	Phone phone("Phone");
	// connecting moduleports
	user1.activity(act_fifo);
	user1.cmdprompt(cmdprompt_fifo);
	user1.notification(noti_fifo); 
	
	user1.pt_pressed(pt_sig);
	user1.ps_plugged(ps_sig);
	user1.powered_on(pow_sig); 
	user1.display(disp_sig); 
	user1.screen(scr_sig);
	user1.battery_level(batt_sig);
	
	//phone port
	phone.activity(act_fifo);
	phone.cmdprompt(cmdprompt_fifo);
	phone.notification(noti_fifo); 
	
	phone.pt_pressed(pt_sig);
	phone.ps_plugged(ps_sig);
	phone.powered_on(pow_sig); 
	phone.display(disp_sig); 
	phone.screen(scr_sig);
	phone.battery_level(batt_sig);
	
	
	sc_start(9000,SC_SEC);
	return 0;
}
