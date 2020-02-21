#ifndef POWERBUTTON_H_
#define POWERBUTTON_H_

#include "systemc.h"
#include "interface.h"
using namespace std;

SC_MODULE(PowerButton){
	//port(s) connected to outside
	sc_in<bool> pt_pressed;
	
	//port(s) inside the phone
	sc_out<bool> shortPress;
	sc_out<bool> longPress;
	
	//internal variables
	sc_time lastTimeStamp;
	sc_time interval_3S;
	
	//constructor
	SC_HAS_PROCESS(PowerButton);
	PowerButton(sc_module_name name){
		shortPress.initialize(false);
		longPress.initialize(false);
		interval_3S=sc_time(3,SC_SEC);
		lastTimeStamp=sc_time_stamp()-interval_3S;
		#if defined(DEBUG_POWERBUTTON) || defined(DEBUG)
			print("powerButton init");
		#endif
		SC_METHOD(run);
		sensitive<<pt_pressed;
	}
	void run(){
		if(pt_pressed.read()==true){
			lastTimeStamp=sc_time_stamp();
			longPress.write(false);
			shortPress.write(false);
		}
		else {
			sc_time now=sc_time_stamp();
			if(now-lastTimeStamp<interval_3S){
				#if defined(DEBUG_POWERBUTTON) || defined(DEBUG)
					print("short press");
				#endif
				shortPress.write(true);
			}else{
				#if defined(DEBUG_POWERBUTTON) || defined(DEBUG)
					print("long press");
				#endif
				longPress.write(true);
			}
		}
	}
	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << name() << "): " << s << endl;
	}
};
#endif