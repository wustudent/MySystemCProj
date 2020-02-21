#ifndef NOTIFICATIONMANAGER_H_
#define NOTIFICATIONMANAGER_H_

#include "systemc.h"
#include "interface.h"
#include <string>

SC_MODULE(NotificationManager){
	//port(s) connected to outside
	sc_fifo_out<Notification> notification;
	
	//port(s) inside the phone
	sc_fifo_in<Notification> noti_PIN;
	sc_fifo_in<Notification> noti_Bat;
	
	//constructor
	SC_CTOR(NotificationManager) {
		SC_METHOD(send);
		sensitive<<noti_Bat.data_written()<<noti_PIN.data_written();
	}
	void send(){
		Notification n;
		if(noti_Bat.nb_read(n))
			notification.write(n);
		Notification n1;
		if(noti_PIN.nb_read(n1))
			notification.write(n1);
	}

	//print given string on cout
	void print(string s){
		cout << "[" << sc_time_stamp() << " / " << sc_delta_count() << "](" << name() << "): " << s << endl;
	}
};

#endif
