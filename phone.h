#ifndef PHONE_H_
#define PHONE_H_

#include "systemc.h"
#include "interface.h"
#include "powerButton.h"
#include "touchScreen.h"
#include "battery.h"
#include "phoneOS.h"
#include "notificationManager.h"
#include <string>

using namespace std;

SC_MODULE(Phone){

	sc_fifo_in<Activity> activity;
	
	sc_in<bool> pt_pressed; 
	sc_in<bool> ps_plugged; 

	//out ports for showing the smartphone's status
	sc_fifo_out<CmdPrompt> cmdprompt;
	sc_fifo_out<Notification> notification; 
	
	sc_out<bool> powered_on; 
	sc_out<Display> display; 
	sc_out<Screen> screen;
	sc_out<int> battery_level; 
	
	//internal signals
	sc_signal<bool> shortPress_sig;
	sc_signal<bool> longPress_sig;
	sc_signal<bool> onActivity_sig;
	sc_fifo<Notification> noti_PIN_sig;
	sc_fifo<Notification> noti_Bat_sig;
	
	//internal modules
	Battery battery;
	PowerButton powerButton;
	TouchScreen touchScreen;
	PhoneOS os;
	NotificationManager nm;
	
	//constructor
	SC_CTOR(Phone):
		powerButton("PowerButton"),
		battery("Battery"),
		touchScreen("TouchScreen"),
		os("OS"),
		nm("NotificationManager"){
		
		//connection of outside signals
		powerButton.pt_pressed(pt_pressed);			// ->input
		touchScreen.pt_pressed(pt_pressed);			// ->input
		
		battery.ps_plugged(ps_plugged);				// ->input
		
		
		battery.battery_level(battery_level);		//<- output
		
		battery.powered_on(powered_on);				//<- output
			touchScreen.powered_on(powered_on);			// |->input
			os.powered_on(powered_on);					// |->input
		
		touchScreen.display(display);				//<- output
			battery.display(display);					// |->input
			os.display(display);						// |->input
		
		os.activity(activity);						//<- output
		
		os.screen(screen);							//<- output
		
		os.cmdprompt(cmdprompt);					//<- output
		
		nm.notification(notification);				//<- output
		
		//connection of internal signals
		os.onActivity(onActivity_sig);				// output--
		touchScreen.onActivity(onActivity_sig);		// input<-|
		
		powerButton.shortPress(shortPress_sig);		// output--
		touchScreen.shortPress(shortPress_sig);		// input<-|
		
		powerButton.longPress(longPress_sig);		// output--
		battery.longPress(longPress_sig);			// input<-|
		
		battery.notification(noti_Bat_sig);			// output--
		nm.noti_Bat(noti_Bat_sig);					// input<-|
		
		os.notification(noti_PIN_sig);				// output--
		nm.noti_PIN(noti_PIN_sig);					// input<-|
		
		//SC_THREAD(run);
	}
	
	void run(){
		//cout<<"phone run"<<endl;
	}
	
};
#endif