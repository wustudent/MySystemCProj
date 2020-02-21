#ifndef PHONEOS_H_
#define PHONEOS_H_

#include "systemc.h"
#include "interface.h"
using namespace std;

SC_MODULE(PhoneOS){
	//port(s) connected to outside
	sc_fifo_in<Activity> activity;
	sc_out<Screen> screen;
	sc_fifo_out<CmdPrompt> cmdprompt;
	
	//port(s) inside the phone
	sc_in<Display> display;
	sc_in<bool> powered_on;
	sc_out<bool> onActivity;
	sc_fifo_out<Notification> notification;
	
	//internal variable
	Display lastDisplayState;
	Screen lastScreenState;
	
	//constructor
	SC_HAS_PROCESS(PhoneOS);
	PhoneOS(sc_module_name name){
		screen.initialize(SCR_TURNED_OFF);
		lastDisplayState=Display::DISP_DARK;
		lastScreenState=Screen::SCR_TURNED_OFF;
		onActivity.initialize(false);
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("os init");
		#endif
		
		SC_METHOD(displayChangeHandler);
		sensitive<<display;
		
		SC_METHOD(activityHandler);
		sensitive<<activity.data_written();
		
		SC_METHOD(phoneOffReseter);
		sensitive<<powered_on.neg();
		
	}
	void displayChangeHandler(){
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("displayChangeHandler");
		#endif
		Display currentState=display.read();
		if(currentState==Display::DISP_BRIGHT){
			if(lastDisplayState==Display::DISP_DARK){
				askForPIN();
			}
		}else if(currentState==Display::DISP_DARK){
			off();
		}
		lastDisplayState=currentState;
	}
	void activityHandler(){
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("activityHandler");
		#endif
		if(powered_on.read()==true){//react only when phone is on
			Activity currentActivity=activity.read();//here takes out an activity from the fifo
			if(currentActivity==Activity::ACT_NOTHING)
				onActivity.write(false);
			else{
				onActivity.write(true);
				#if defined(DEBUG_OS) || defined(DEBUG)
					print("onActivity");
				#endif
			}
			Screen currentScreen=screen.read();
			if(currentScreen==Screen::SCR_PIN_REQUEST){
				if(currentActivity==Activity::ACT_WRONG_PIN){
					notification.write(Notification::NOTI_WRONG_PIN);
					askForPIN();
				}else if(currentActivity==Activity::ACT_CORRECT_PIN){
					notification.write(Notification::NOTI_CORRECT_PIN);
					if(lastScreenState==Screen::SCR_TURNED_OFF){
						home();
					}else{
						screen.write(lastScreenState);
					}
				}
			}else if(currentScreen==Screen::SCR_HOME){
				if(isOpenAppActivity(currentActivity)){
					openApp(currentActivity);
				}
			}else if(currentScreen>=Screen::SCR_APP1&&currentScreen<=Screen::SCR_APP5){
				if(currentActivity==Activity::ACT_HOME){
					home();
				}
			}
		}
	}
	void phoneOffReseter(){
		if(powered_on.read()==false){
			lastDisplayState=Display::DISP_DARK;
			lastScreenState=Screen::SCR_TURNED_OFF;
		}
	}
	void askForPIN(){
	
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("askForPIN");
		#endif
		screen.write(Screen::SCR_PIN_REQUEST);
		cmdprompt.write(CmdPrompt::CMD_ENTER_PIN);
			
	}
	void home(){
		
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("home");
		#endif
		screen.write(Screen::SCR_HOME);
		cmdprompt.write(CmdPrompt::CMD_CHOOSE_APP);
		lastScreenState=Screen::SCR_HOME;
			
	}
	void off(){
		
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("off");
		#endif
		screen.write(Screen::SCR_TURNED_OFF);
	}
	bool isOpenAppActivity(Activity act){
		return (act>=Activity::ACT_APP1&&act<=Activity::ACT_APP5);
	}
	void openApp(Activity act){
		Screen now;
		switch(act){
			case Activity::ACT_APP1 : now=Screen::SCR_APP1;break;
			case Activity::ACT_APP2 : now=Screen::SCR_APP2;break;
			case Activity::ACT_APP3 : now=Screen::SCR_APP3;break;
			case Activity::ACT_APP4 : now=Screen::SCR_APP4;break;
			case Activity::ACT_APP5 : now=Screen::SCR_APP5;break;
			default: now=Screen::SCR_APP1;
		}
		
		#if defined(DEBUG_OS) || defined(DEBUG)
			print("openApp");
		#endif
		screen.write(now);
		lastScreenState=now;
	}
	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << name() << "): " << s << endl;
	}
};
#endif