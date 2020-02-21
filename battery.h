#ifndef BATTERY_H_
#define BATTERY_H_

#include "systemc.h"
#include "interface.h"
using namespace std;

SC_MODULE(Battery){
	//port(s) connected to outside
	sc_in<bool> ps_plugged;
	sc_out<int> battery_level;
	sc_out<bool> powered_on;	//(according to reference manual, reading from out port internally is allowed)
	
	//port(s) inside the phone
	sc_in<Display> display;
	sc_in<bool> longPress;
	sc_fifo_out<Notification> notification;
	
	//internal events
	sc_event lowPower;
	sc_event lowPowerEnd;
	sc_event inversePower;
	
	//constructor
	SC_HAS_PROCESS(Battery);
    Battery(sc_module_name name){
		battery_level.initialize(100);			//start with 100%
		powered_on.initialize(false);
		#if defined(DEBUG_BATTERY) || defined(DEBUG)
			print("battery init");
		#endif
		
		SC_METHOD(longPressHandler);
		sensitive<<longPress.pos();
		
		SC_METHOD(inversePowerState);
		sensitive<<inversePower;
		
		SC_THREAD(notify);
		SC_THREAD(run);
		
	}
	//main process
	void run(){
		while(1){
			wait(60,SC_SEC);
			if(ps_plugged.read()==true){
				charge();
			}else{
				if(powered_on.read()==true){
					drain();
					int nowCapacity=battery_level.read();
					
					if(nowCapacity<=5){
						#if defined(DEBUG_BATTERY) || defined(DEBUG)
							print("no power");
							cout<<nowCapacity<<endl;
						#endif
						//powered_on.write(false);	//turn off phone
						inversePower.notify();
					}
					
				}
			}	
		}
	}
	void longPressHandler(){
		if(longPress.read()==true){
			#if defined(DEBUG_BATTERY) || defined(DEBUG)
				print("long press");
			#endif
			
			if(powered_on.read()==true){
				//powered_on.write(false);
				inversePower.notify();
				#if defined(DEBUG_BATTERY) || defined(DEBUG)
					print("long press(power off)");
				#endif
			}else{
				if(battery_level.read()>10){
					//powered_on.write(true);
					inversePower.notify();
					#if defined(DEBUG_BATTERY) || defined(DEBUG)
						print("long press(power on)");
					#endif
				}
			}
				
		}
	}
	void inversePowerState(){
		bool currentState=powered_on.read();
		powered_on.write(!currentState);
	}
	void drain(){
		int currentCapacity=battery_level.read();
		
		if(display.read()==true){
			currentCapacity-=2;
		}else{
			currentCapacity-=1;
		}
		#if defined(DEBUG_BATTERY) || defined(DEBUG)
			print("drain");
			cout<<currentCapacity<<endl;
		#endif
		if(currentCapacity<=10)
			lowPower.notify();
		battery_level.write(currentCapacity);
	}
	void charge(){
		int currentCapacity=battery_level.read();
		if(powered_on.read()==false)
			currentCapacity+=3;
		else{
			if(display.read()==Display::DISP_DARK){
				currentCapacity+=2;
			}else{
				currentCapacity+=1;
			}
		}
		if(currentCapacity>100){
			currentCapacity=100;
		}
		if(currentCapacity>=10)
			lowPowerEnd.notify();
		#if defined(DEBUG_BATTERY) || defined(DEBUG)
			print("charge");
			cout<<currentCapacity<<endl;
		#endif
		battery_level.write(currentCapacity);
	}
	void notify(){
		while(1){
			wait(lowPower);
			#if defined(DEBUG_BATTERY) || defined(DEBUG)
				print("notify");
			#endif
			notification.write(Notification::NOTI_LOW_BATTERY_LEVEL);
			wait(lowPowerEnd);		//after notification sent, should wait
		}
	}
	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << name() << "): " << s << endl;
	}
};
#endif