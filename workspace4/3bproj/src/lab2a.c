/*****************************************************************************
* lab2a.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 23,2014
*****************************************************************************/

#define AO_LAB2A

#include "qpn_port.h"
#include "bsp.h"
#include "lab2a.h"
#include <stdio.h>		// Used for printf()
#include "xparameters.h"	// Contains hardware addresses and bit masks
#include "xil_cache.h"		// Cache Drivers
#include "xintc.h"		// Interrupt Drivers
#include "xtmrctr.h"		// Timer Drivers
#include "xtmrctr_l.h" 		// Low-level timer drivers
#include "xil_printf.h" 	// Used for xil_printf()
//#include "extra.h" 		// Provides a source of bus contention
#include "xgpio.h" 		// LED driver, used for General purpose I/i
#include "xspi.h"
#include "xspi_l.h"
#include "lcd.h"



typedef struct Lab2ATag  {               //Lab2A State machine
	QActive super;
	u8 volume;
	int time;
	char label[32];


}  Lab2A;

/* Setup state machines */
/**********************************************************************/
static QState Lab2A_initial (Lab2A *me);
static QState Lab2A_on      (Lab2A *me);
static QState Lab2A_stateA  (Lab2A *me);
static QState Lab2A_stateB  (Lab2A *me);


/**********************************************************************/


Lab2A AO_Lab2A;


void Lab2A_ctor(void)  {
	Lab2A *me = &AO_Lab2A;
	QActive_ctor(&me->super, (QStateHandler)&Lab2A_initial);
}

int mute_flag = 0;


QState Lab2A_initial(Lab2A *me) {
	xil_printf("\n\rInitialization");
	Static_Background();
    return Q_TRAN(&Lab2A_on);
}

QState Lab2A_on(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("\n\rOn");
			}
			
		case Q_INIT_SIG: {
			return Q_TRAN(&Lab2A_stateA);
			}
	}
	
	return Q_SUPER(&QHsm_top);
}


/* Create Lab2A_on state and do any initialization code if needed */
/******************************************************************/
// TODO: **WILL HAVE TO CHANGE THIS CODE AFTER UI**
QState Lab2A_stateA(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("Startup State A\n");
			Clear_overlay();
			Draw_overlay(me);
			me->time = 0;
			return Q_HANDLED();
		}
		
		case ENCODER_UP: {
			xil_printf("Encoder Up from State A\n");
			if (!mute_flag){
				me->volume+=4;
				if(me->volume > 100) me->volume = 100;
			}
			Draw_Volume(me);
			me->time = 0;
			return Q_HANDLED();
		}

		case ENCODER_DOWN: {
			xil_printf("Encoder Down from State A\n");
			if (!mute_flag){
				if(me->volume > 0){
					me->volume-= 4;
				}
			}
			Draw_Volume(me);
			me->time = 0;
			return Q_HANDLED();
		}

		case ENCODER_CLICK:  {
			xil_printf("reset volume\n");
			mute_flag = !mute_flag;
			Draw_Volume(me);
			me->time = 0;
			return Q_HANDLED();
		}
		case TICK_SIG: {

			me->time++;
			if(me->time == 3){
				Clear_overlay(me);
				me->time = 0;
				xil_printf("Change state\n");
				return Q_TRAN(&Lab2A_stateB);

			}

			return Q_HANDLED();

		}
		case MODE_UP: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label, "Mode:1", sizeof(me->label) - 1);
			Draw_Mode(me);
			me->time = 0;
			return Q_HANDLED();

		}
		case MODE_DOWN: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label ,"Mode:3", sizeof(me->label) - 1);
			Draw_Mode(me);
			me->time = 0;
			return Q_HANDLED();

		}

		case MODE_LEFT: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label, "Mode:4", sizeof(me->label) - 1);
			Draw_Mode(me);
			me->time = 0;
			return Q_HANDLED();

		}

		case MODE_RIGHT: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label, "Mode:2", sizeof(me->label) - 1);
			Draw_Mode(me);
			me->time = 0;
			return Q_HANDLED();

		}

	}

	return Q_SUPER(&Lab2A_on);

}

QState Lab2A_stateB(Lab2A *me) {
	switch (Q_SIG(me)) {
		case Q_ENTRY_SIG: {
			xil_printf("Startup State B\n");
			return Q_TRAN(&Lab2A_stateA);
		}
		
		case ENCODER_UP: {
			xil_printf("Encoder Up from State B\n");
			if(!mute_flag){
				me->volume+=4;
				if(me->volume > 100) me->volume = 100;
			}
			//Draw_overlay(me);
			return Q_TRAN(&Lab2A_stateA);
		}

		case ENCODER_DOWN: {
			xil_printf("Encoder Down from State B\n");
			if(!mute_flag){
				if(me->volume > 0){
					me->volume-=4;
				}
			}
			//Draw_overlay(me);
			return Q_TRAN(&Lab2A_stateA);
		}

		case ENCODER_CLICK:  {
			xil_printf("Changing State\n");
			mute_flag = !mute_flag;
			// reset volume to 0
			return Q_TRAN(&Lab2A_stateA);
		}
		case MODE_UP: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label, "Mode:1", sizeof(me->label) - 1);
			//Draw_overlay(me);

			return Q_TRAN(&Lab2A_stateA);

		}

		case MODE_DOWN: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label, "Mode:3", sizeof(me->label) - 1);
			//Draw_overlay(me);
			return Q_TRAN(&Lab2A_stateA);

		}

		case MODE_LEFT: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label, "Mode:4", sizeof(me->label) - 1);
			//Draw_overlay(me);
			return Q_TRAN(&Lab2A_stateA);

		}

		case MODE_RIGHT: {
			memset(me->label, 0, sizeof(me->label));
			strncpy(me->label , "Mode:2", sizeof(me->label) - 1);
			//Draw_overlay(me);
			return Q_TRAN(&Lab2A_stateA);
		}

	}

	return Q_SUPER(&Lab2A_on);

}

