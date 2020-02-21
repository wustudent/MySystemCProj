#ifndef TOUCHSCREEN_H_
#define TOUCHSCREEN_H_

#include "systemc.h"
#include "interface.h"
using namespace std;

SC_MODULE(TouchScreen){
	//port(s) connected to outside
	sc_out<Display> display;
	
	//port(s) inside the phone
	sc_in<bool> shortPress;
	sc_in<bool> onActivity;
	sc_in<bool> powered_on;			//intended to read phone state
	sc_in<bool> pt_pressed;			//to record the display state when button is pressed down, 
											//without it will cause the problem against the user's will:
											//(During the short-press period, display may be turned off automatically,
											//when user then release the button, display SHOULD NOT be open again)
	//internal variable
	sc_time lastActivityTimestamp;
	Display stateWhenPressedDown;
	
	
	//constructor
	SC_HAS_PROCESS(TouchScreen);
    TouchScreen(sc_module_name name){
		display.initialize(Display::DISP_DARK);
		stateWhenPressedDown=Display::DISP_DARK;
		#if defined(DEBUG_SCREEN) || defined(DEBUG)
			print("touchscreen init");
		#endif
		
		SC_THREAD(run);
		
		SC_METHOD(phoneOnOffHandler);
		sensitive<<powered_on;
		
		SC_METHOD(wakeUp);
		sensitive<<onActivity;
		
		SC_METHOD(recordState);
		sensitive<<pt_pressed;
		
		SC_METHOD(shortPressHandler);
		sensitive<<shortPress.pos();
		
		
	}
	void run(){
		while(1){
			wait(1,SC_SEC);
			if(powered_on.read()==true){
				if(sc_time_stamp()-lastActivityTimestamp==sc_time(80,SC_SEC)){
					dim();
				}else if(sc_time_stamp()-lastActivityTimestamp==sc_time(100,SC_SEC)){
					off();
				}
			}
		}
	}
	//phone on/off handler
	void phoneOnOffHandler(){
		#if defined(DEBUG_SCREEN) || defined(DEBUG)
			print("phoneOnOffHandler");
		#endif
		if(powered_on.read()==true){
			on();
		}else{
			off();
		}
	}
	//record state when pressed
	void recordState(){
		if(pt_pressed==true)
			stateWhenPressedDown=display.read();
	}
	
	void shortPressHandler(){		//only handles short press
		if(powered_on.read()==true){
			if(shortPress.read()==true){
				#if defined(DEBUG_SCREEN) || defined(DEBUG)
					print("shortPressHandler");
				#endif
				//Display currentState=display.read();//this is wrong, should not read state when release
				if(stateWhenPressedDown==Display::DISP_DARK){//recorded state should be used
					on();
				}
				else{
					off();
				}
			}
		}
	}
	
	//activity handler
	void wakeUp(){
		#if defined(DEBUG_SCREEN) || defined(DEBUG)
			print("wakeUp");
		#endif
		if(onActivity.read()==true){
			if(display.read()!=Display::DISP_DARK){
				#if defined(DEBUG_SCREEN) || defined(DEBUG)
					print("wakeUp On");
				#endif
				on();
			}
		}
	}
	void change(Display nextState){
		
		Display currentState=display.read();
		bool validFlag=true;
		if(currentState==Display::DISP_DARK&&nextState==Display::DISP_MEDIUM){
			validFlag=false;
		}else if(currentState==nextState){
			validFlag=false;
		}
		if(validFlag==true){
			
			display.write(nextState);
			#if defined(DEBUG_SCREEN) || defined(DEBUG)
				switch(nextState){
					case Display::DISP_BRIGHT:print("change on()");break;
					case Display::DISP_MEDIUM:print("change dim()");break;
					default:print("change dark()");
				}
			#endif
		}
	}
	void on(){
		#if defined(DEBUG_SCREEN) || defined(DEBUG)
			print("on");
		#endif
		change(Display::DISP_BRIGHT);
		lastActivityTimestamp=sc_time_stamp();
	}
	void dim(){
		#if defined(DEBUG_SCREEN) || defined(DEBUG)
			print("dim");
		#endif
		change(Display::DISP_MEDIUM);
	}
	void off(){
		#if defined(DEBUG_SCREEN) || defined(DEBUG)
			print("off");
		#endif
		change(Display::DISP_DARK);
	}
	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << name() << "): " << s << endl;
	}
};
#endif