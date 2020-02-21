#ifndef USER_H_
#define USER_H_

#include "systemc.h"
#include "interface.h"
#include <string>
using namespace std;

/* Please check interface.h. There you will find all possbile values.*/

SC_MODULE(User) {
	//output ports for smartphone inputs
	sc_fifo_out<Activity> activity;
	
	sc_out<bool> pt_pressed; //for powering_on, lock/unlock-screen, powering_off
	sc_out<bool> ps_plugged; //power supply plugged in/off true/false


	//input ports for monitoring the smartphone's status
	sc_fifo_in<CmdPrompt> cmdprompt;
	sc_fifo_in<Notification> notification; 
	
	sc_in<bool> powered_on; 
	sc_in<Display> display; 
	sc_in<Screen> screen;
	sc_in<int> battery_level; 
	
	bool userChanged;

	SC_CTOR(User) {
		userChanged=false;
		SC_THREAD(fst_seq);
	}


	void fst_seq()
	{
		//turn smartphone on:
		turn_on();
		bool isPowered = powered_on.read();
		
		CmdPrompt currentPrompt = cmdprompt.read();
		Display displayState = display.read();
		Screen currentScreen = screen.read();

		if (isPowered == false) {
			print("Smartphone should be powered on by now. Thread will be closed...");
			return;
		}
		if (displayState != Display::DISP_BRIGHT) {
			print("Smartphone  display should be shnining bright. Thread will be closed...");
		} 
		if (currentPrompt != CmdPrompt::CMD_ENTER_PIN) {
			print("Smartphone should be giving a PIN request. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_PIN_REQUEST) {
			print("Smartphone Screen doesn't show a PIN-request. Thread will be closed...");
			return;
		}
		
		//enter wrong pin
		print("Entering wrong PIN..");
		activity.write(Activity::ACT_WRONG_PIN);
		wait(1,SC_SEC);
		Notification noti = notification.read();
		currentScreen = screen.read();
		if (noti != Notification::NOTI_WRONG_PIN) {
			print("Smartphone doesn't state that the entered PIN was wrong. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_PIN_REQUEST) {
			print("Smartphone Screen doesn't show a PIN-request. Thread will be closed...");
			return;
		}
		
		//enter correct pin
		print("Entering correct PIN..");
		activity.write(Activity::ACT_CORRECT_PIN);
		wait(1,SC_SEC);
		noti = notification.read();
		currentScreen = screen.read();
		if (noti != Notification::NOTI_CORRECT_PIN) {
			print("Smartphone doesn't state that the entered PIN was correct. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home screen. Thread will be closed...");
			return;
		}
		
		//choose app
		print("Choosing APP 3...");
		activity.write(Activity::ACT_APP3);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP3) {
			print("Smartphone Screen doesn't show APP 3. Thread will be closed...");
			return;
		}		
		
		//wait smartphone to be dimmed
		print("Waiting for the Smartphone to be dimmed...");
		activity.write(Activity::ACT_NOTHING);
		wait(80,SC_SEC);
		displayState = display.read();
		if (displayState != Display::DISP_MEDIUM) {
			print("Smartphone display should be dimmed by now. Thread will be closed...");
			return;
		}
		
		//simple activity
		print("Doing Something in current APP...");
		activity.write(Activity::ACT_DEFAULT);
		wait(1,SC_SEC);
		displayState = display.read();
		if (displayState != Display::DISP_BRIGHT) {
			print("Smartphone display should be bright by now. Thread will be closed...");
			return;
		}
		
		//press pt shortly
		click_pt_button();
		displayState = display.read();
		currentScreen = screen.read();
		if (displayState != Display::DISP_DARK) {
			print("Smartphone display should be dark by now. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_TURNED_OFF) {
			print("Smartphone screen should be turned off by now. Thread will be closed...");
			return;
		}
			
		//wait a moment
		print("Just waiting and doing nothing...For 10 Seconds");
		activity.write(Activity::ACT_NOTHING);
		wait(10,SC_SEC);
		
		//press pt shortly
		click_pt_button();
		displayState = display.read();
		currentScreen = screen.read();
		if (displayState != Display::DISP_BRIGHT) {
			print("Smartphone display should be bright by now. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_PIN_REQUEST) {
			print("Smartphone screen should be showing a PIN request. Thread will be closed...");
			return;
		}
		
		//enter correct pin
		print("Entering correct PIN..");
		activity.write(Activity::ACT_CORRECT_PIN);
		wait(1,SC_SEC);
		noti = notification.read();
		currentScreen = screen.read();
		if (noti != Notification::NOTI_CORRECT_PIN) {
			print("Smartphone doesn't state that the entered PIN was correct. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_APP3) {
			print("Smartphone Screen doesn't show APP3. Thread will be closed...");
			switch(currentScreen){
				case Screen::SCR_HOME : print("home");break;
				case Screen::SCR_PIN_REQUEST : print("pin");break;
				case Screen::SCR_TURNED_OFF : print("off");break;
				default: print("app");
			}
			return;
		}
		
		//press home_button,
		print("Going back to home menu...");
		activity.write(Activity::ACT_HOME);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home menu. Thread will be closed...");
			return;
		}		
		

		//let display timeout
		print("Waiting for the Smartphone to be timed out...");
		activity.write(Activity::ACT_NOTHING);
		wait(101,SC_SEC);
		displayState = display.read();
		currentScreen = screen.read();
		if (displayState != Display::DISP_DARK) {
			print("Smartphone display should be dimmed by now. Thread will be closed...");
			return;
		}
		if (currentScreen != SCR_TURNED_OFF) {
			print("Smartphone Screen should be turned off by now. Thread will be closed...");
			return;
		}
		
		//wait until battery is low (auto shutdown)
		print("waiting for low battery auto shutdown");
		int batt = battery_level.read();
		while (batt > 10) {
			wait(1,SC_SEC);
			batt = battery_level.read();
		}
		int num_noti = notification.num_available();
		bool infoLowBattery = false;
		while (num_noti > 0 && !(infoLowBattery)) {
			noti = notification.read();
			if (noti == Notification::NOTI_LOW_BATTERY_LEVEL) {
				infoLowBattery = true;
			}
			num_noti--;
		}
		if (infoLowBattery == false) {
			print("Smartphone has not informed about Low Battery Level. Thread will be closed...");
			return;
		}
		batt = battery_level.read();
		isPowered = powered_on.read();
		if (!(isPowered) && batt > 7) {
			print("Smartphone's battery level is high enough. Smartphone shouldn't be powered off. Thread will be closed...");
		}
		if (isPowered) {
			int battOld = 0;
			while (isPowered || (batt != battOld)) {
				wait(3,SC_SEC);
				battOld = batt;
				batt = battery_level.read();
				isPowered = powered_on.read();
			}
		}
		if (batt < 4) {
			print("Smartphone's battery level is lower than allowed and shouldn't get that low. Thread will be closed...");
		}
		if (batt > 7) {
			print("Smartphone's battery level is high enough. Smartphone shouldn't be powered off. Thread will be closed...");
		}
		isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone has low battery level and shouldn't be on now. Thread will be closed...");
			return;
		}
		if (batt < 4) {
			print("Smartphone's battery level is lower than allowed and shouldn't get that low. Thread will be closed...");
			return;
		}
		//try to power up smartphone
		print("Trying to power-up the smartphone...");
		turn_on();
		wait(1,SC_SEC);
		isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone has low battery level and shouldn't be able to run. Thread will be closed...");
			return;
		}
		
		//state: "Mist, Ladekabel vergessen! ;-)"
		print("What a pitty. I must have forgotten my Smartphone's charging cable. END");

		//add notification for 2nd user process here
		scnd_seq();
	}
	void scnd_seq(){
		userChanged=true;
		cout<<endl;
		print("***************************** User2 came **********************************");
		cout<<endl;
		wait(1,SC_SEC);
		print("Trying to power-up the smartphone when battery level<10%...");
		turn_on();
		wait(1,SC_SEC);
		bool isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone has low battery level and shouldn't be able to run. Thread will be closed...");
			return;
		}
		
		//here charge the phone
		ps_plugged.write(true);
		
		wait(120,SC_SEC);//10%
		print("Trying to power-up the smartphone when battery level=10%...");
		turn_on();
		wait(1,SC_SEC);
		isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone has low battery level and shouldn't be able to run. Thread will be closed...");
			return;
		}
		cout<<"battery level now:"<<battery_level.read()<<endl;
		wait(60,SC_SEC);//13%
		print("Trying to power-up the smartphone when battery level>10%...");
		turn_on();
		wait(1,SC_SEC);
		isPowered = powered_on.read();
		
		CmdPrompt currentPrompt = cmdprompt.read();
		Display displayState = display.read();
		Screen currentScreen = screen.read();
		if (!isPowered) {
			print("Smartphone should be powered on by now. Thread will be closed...");
			cout<<battery_level.read()<<endl;
			return;
		}
		cout<<"battery level now:"<<battery_level.read()<<endl;
		click_pt_button();//close screen
		print("Stop charging");
		ps_plugged.write(false);	//unplug
		wait(180,SC_SEC);//10%
		cout<<"battery level now:"<<battery_level.read()<<endl;
		turn_off();
		wait(3,SC_SEC);
		print("Trying to power-up the smartphone when battery level=10%...");
		turn_on();
		isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone has low battery level and shouldn't be able to run. Thread will be closed...");
			return;
		}
		
		ps_plugged.write(true);
		print("Charge 200s to make it possible to open the phone");
		wait(200,SC_SEC);//in 200s the battery level should be over 10%
		print("Short press to see whether phone will be opened");
		click_pt_button();
		isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone was short pressed and shouldn't be able to run. Thread will be closed...");
			return;
		}
		wait(1,SC_SEC);
		turn_on();
		
		isPowered = powered_on.read();
		currentPrompt = cmdprompt.read();
		displayState = display.read();
		currentScreen = screen.read();
		
		if (isPowered == false) {
			print("Smartphone should be powered on by now. Thread will be closed...");
			return;
		}
		if (displayState != Display::DISP_BRIGHT) {
			print("Smartphone  display should be shining bright. Thread will be closed...");
		} 
		if (currentPrompt != CmdPrompt::CMD_ENTER_PIN) {
			print("Smartphone should be giving a PIN request. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_PIN_REQUEST) {
			print("Smartphone Screen doesn't show a PIN-request. Thread will be closed...");
			return;
		}
		
		//enter correct pin
		print("Entering correct PIN...");
		activity.write(Activity::ACT_CORRECT_PIN);
		wait(1,SC_SEC);
		Notification noti = notification.read();
		currentScreen = screen.read();
		
		
		print("Wait 1 min to see whether phone charge correctly when screen on");
		activity.write(Activity::ACT_DEFAULT);
		//remember the battery_level
		int oldBattery1 = battery_level.read();
		
		//watch, if the phone ist correctly charged with screen on
		wait(60,SC_SEC);
		int newBattery1 = battery_level.read();
		int chargeScreenOn = newBattery1-oldBattery1;
		if (chargeScreenOn != 1){
			print("Smartphone is not correctly charged with screen on. Thread will be closed...");
			cout<<chargeScreenOn<<endl;
			return;
		}
		
		//watch, if the phone ist correctly charged with screen off
		print("Closing the screen...");
		wait(1,SC_SEC);
		click_pt_button();//press pt shortly
		displayState = display.read();
		currentScreen = screen.read();
		if (displayState != Display::DISP_DARK) {
			print("Smartphone display should be dark by now. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_TURNED_OFF) {
			print("Smartphone screen should be turned off by now. Thread will be closed...");
			return;
		}
		
		print("Wait 1 min to see whether phone charge correctly when screen off");
		int oldBattery2 = battery_level.read();
		wait(60,SC_SEC);
		int newBattery2 = battery_level.read();
		int chargeScreenOff = newBattery2-oldBattery2;
		if (chargeScreenOff != 2){
			print("Smartphone is not correctly charged with screen off. Thread will be closed...");
			cout<<chargeScreenOff<<endl;
			return;
		} 
		
		//watch, if the phone ist correctly charged when turned off
		//wait(1,SC_SEC);
		print("Turning off the phone...");
		int oldBattery3 = battery_level.read();
		//Turn the smartphone off
		turn_off();
		print("Wait 1 min to see whether phone charge correctly when itself is turned off");
		wait(60,SC_SEC);
		turn_on();
		isPowered = powered_on.read();
		
		currentPrompt = cmdprompt.read();
		displayState = display.read();
		currentScreen = screen.read();
		print("Entering correct PIN..");
		activity.write(Activity::ACT_CORRECT_PIN);
		wait(1,SC_SEC);
		int newBattery3 = battery_level.read();
		int chargeTurnOff = newBattery3-oldBattery3;
		if (chargeTurnOff != 3){
			print("Smartphone is not correctly charged when turned off. Thread will be closed...");
			return;
		} 
		
		//App wechsel
		
		print("Choosing APP 1...");
		activity.write(Activity::ACT_APP1);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP1) {
			print("Smartphone Screen doesn't show APP 1. Thread will be closed...");
			return;
		}	
		//press home_button,
		print("Going back to home menu...");
		activity.write(Activity::ACT_HOME);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home menu. Thread will be closed...");
			return;
		}	
		
		print("Choosing APP 2...");
		activity.write(Activity::ACT_APP2);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP2) {
			print("Smartphone Screen doesn't show APP 2. Thread will be closed...");
			return;
		}	
		 
		print("Going back to home menu...");
		activity.write(Activity::ACT_HOME);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home menu. Thread will be closed...");
			return;
		}
		
		print("Choosing APP 3...");
		activity.write(Activity::ACT_APP3);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP3) {
			print("Smartphone Screen doesn't show APP 3. Thread will be closed...");
			return;
		}	
		 
		print("Going back to home menu...");
		activity.write(Activity::ACT_HOME);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home menu. Thread will be closed...");
			return;
		}
		
		print("Choosing APP 4...");
		activity.write(Activity::ACT_APP4);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP4) {
			print("Smartphone Screen doesn't show APP 4. Thread will be closed...");
			return;
		}	
		 
		print("Going back to home menu...");
		activity.write(Activity::ACT_HOME);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home menu. Thread will be closed...");
			return;
		}
		
		print("Choosing APP 5...");
		activity.write(Activity::ACT_APP5);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP5) {
			print("Smartphone Screen doesn't show APP 5. Thread will be closed...");
			return;
		}	
		 
		print("Going back to home menü...");
		activity.write(Activity::ACT_HOME);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home menu. Thread will be closed...");
			return;
		}
		
		//Ausschalten bei App-Nutzung
		print("Choosing APP 5...");
		activity.write(Activity::ACT_APP5);
		wait(1,SC_SEC);
		currentScreen = screen.read();
		if (currentScreen != Screen::SCR_APP5) {
			print("Smartphone Screen doesn't show APP 5. Thread will be closed...");
			return;
		}	
		print("Turning off when APP 5 opened...");
		turn_off();
		//after the turn off
		wait(10,SC_SEC);
		turn_on();
		isPowered = powered_on.read();
		
		currentPrompt = cmdprompt.read();
		displayState = display.read();
		currentScreen = screen.read();
		print("Entering correct PIN..");
		activity.write(Activity::ACT_CORRECT_PIN);
		wait(1,SC_SEC);
		noti = notification.read();
		currentScreen = screen.read();
		if (noti != Notification::NOTI_CORRECT_PIN) {
			print("Smartphone doesn't state that the entered PIN was correct. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home screen. Thread will be closed...");
			return;
		}
		
		print("Turning off in Haupt Menü");
		turn_off();//in Haupt Menü
		//after the turn off
		wait(10,SC_SEC);
		print("Turning on again");
		turn_on();
		isPowered = powered_on.read();
		
		currentPrompt = cmdprompt.read();
		displayState = display.read();
		currentScreen = screen.read();
		print("Entering correct PIN..");
		activity.write(Activity::ACT_CORRECT_PIN);
		wait(1,SC_SEC);
		noti = notification.read();
		currentScreen = screen.read();
		if (noti != Notification::NOTI_CORRECT_PIN) {
			print("Smartphone doesn't state that the entered PIN was correct. Thread will be closed...");
			return;
		}
		if (currentScreen != Screen::SCR_HOME) {
			print("Smartphone Screen doesn't show home screen. Thread will be closed...");
			return;
		}
		
		//press to close screen shortly before it closing automatically, to see whether screen will open again
		activity.write(Activity::ACT_NOTHING);
		wait(1,SC_SEC);
		print("wait 100.5s");
		activity.write(Activity::ACT_DEFAULT);
		wait(99500,SC_MS);
		//wait(500,SC_MS);
		print("press to close screen shortly before it closing automatically");
		click_pt_button();
		displayState = display.read();
		if (displayState != Display::DISP_DARK) {
			print("Smartphone display should not be opened again. Thread will be closed...");
			return;
		}
		
		wait(100,SC_SEC);
		print("Turning off when Screen is dark");
		turn_off();//in Haupt Menü
		isPowered = powered_on.read();
		if (isPowered) {
			print("Smartphone should not be powered on by now. Thread will be closed...");
			return;
		}
		cout<<endl;
		print("########################### Test Succeeded! ###########################");
		cout<<endl;
	}
	/*
	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << name() << "): " << s << endl;
	}
	*/
	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << (userChanged==false?"User1":"User2") << "): " << s << endl;
	}
	// Some possible actions
	//Turn the Smartphone on.
	void turn_on()
	{
		print("turn_on()");
		pt_pressed.write(true);
		//print("pt has been pressed and held..");
		wait(3, SC_SEC);
		pt_pressed.write(false);
		//print("pt has been released");
		wait(1,SC_SEC);
	}

	//Turn the smartphone off
	void turn_off()
	{
		print("turn_off()");
		pt_pressed.write(true);
		//print("pt has been pressed");
		wait(3, SC_SEC);
		pt_pressed.write(false);
		//print("pt has been released");
		wait(1,SC_SEC);
	}
	
	//lock or unlock screen
	void click_pt_button()
	{
		print("click_pt_button()");
		pt_pressed.write(true);
		//print("pt has been pressed");
		wait(1, SC_SEC);
		pt_pressed.write(false);
		//print("pt has been released");
		wait(1,SC_SEC);		
	}

};
#endif
