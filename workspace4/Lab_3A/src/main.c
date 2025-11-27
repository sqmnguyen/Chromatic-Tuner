/*
 * Copyright (c) 2009-2012 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "xil_cache.h"
#include <mb_interface.h>

#include "xparameters.h"
#include <xil_types.h>
#include <xil_assert.h>

#include <xio.h>
#include "xtmrctr.h"
#include "fft.h"
#include "note.h"
#include "stream_grabber.h"
#include "xintc.h"
#include "trig.h"
#define RESET_VALUE 100000
#define SAMPLES 128 // AXI4 Streaming Data FIFO has size 512
#define M 7 //2^m=samples
#define CLOCK 100000000.0 //clock speed

int int_buffer[SAMPLES];
static float q[SAMPLES];
static float w[SAMPLES];
int count = 0;
int read_count = 0;
int fft_count = 0;
int find_count = 0;
int misc_count = 0;
int trig_count = 0;
int complex_count=0;
int computation_count = 0;
int interrupt_count = 0;
int print_count = 0;
unsigned seqf, seql, seq_old=0;
XIntc sys_intc;
XTmrCtr perfTimer;
//void print(char *str);
void Timer_ISR(void *CallbackRef)
{
	uint32_t ControlStatusReg;

	ControlStatusReg =
				XTimerCtr_ReadReg(perfTimer.BaseAddress, 0, XTC_TCSR_OFFSET);
    uint32_t ra;
    asm("add %0, r0, r14" : "=r"(ra));
    //xil_printf("ra: %d\r\n", ra);

    if (ra >= 0x80002fb8 && ra < 0x80003110){
        read_count++;
	}
    else if (ra >= 0x800022d0 && ra < 0x80002dc8){
    	fft_count++;
    }

    else if (ra >= 0x800034d0 && ra < 0x80003738 ){
        find_count++;
    }
    else if (ra>=0x80003114 && ra <0x800034d4){
       misc_count++;
    }
    else if(ra>=0x80000000 && ra < 0x800021d0){
    	computation_count++;
    }
    else if(ra>=0x800021d0 && ra < 0x800022cc){
    	complex_count++;
    }
    else if(ra>=0x80003f70 && ra < 0x8000405c){
        trig_count++;
    }

    else{
    	interrupt_count++;
    }


	count++;

    XTmrCtr_WriteReg(perfTimer.BaseAddress, 0,
                     XTC_TCSR_OFFSET, ControlStatusReg | XTC_CSR_INT_OCCURED_MASK);
}

void read_fsl_values(float* q, int n) {
    unsigned int x;
    unsigned int sample;
    int i;

    // Start microphone grabber
    stream_grabber_start();

    // Read sequence counters (for timing info)
    stream_grabber_wait_enough_samples(SAMPLES*4);
    seql = stream_grabber_read_seq_counter();
    seqf = stream_grabber_read_seq_counter_latched();



    for (i = 0; i < n; i++) {
        q[i] = 3.3 *stream_grabber_read_sample(i * 4) / 67108864.0;
    }

}

int main() {

   float sample_f;
   int l;
   int ticks; //used for timer
   uint32_t Control;
   float frequency;

   Xil_ICacheInvalidate();
   Xil_ICacheEnable();
   Xil_DCacheInvalidate();
   Xil_DCacheEnable();
   XIntc_Initialize(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
      //set up timer

   XTmrCtr timer;
   XTmrCtr_Initialize(&timer, XPAR_AXI_TIMER_1_DEVICE_ID);
   Control = XTmrCtr_GetOptions(&timer,0) | XTC_CAPTURE_MODE_OPTION | XTC_INT_MODE_OPTION;
   XTmrCtr_SetOptions(&timer,0,Control);
   XTmrCtr_Start(&timer,0);

   //perf timer
   XIntc_Connect(&sys_intc,
                       XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR,
                       (XInterruptHandler)Timer_ISR,
                       &perfTimer);
   XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);

   XTmrCtr_Initialize(&perfTimer, XPAR_AXI_TIMER_0_DEVICE_ID);

   XTmrCtr_SetOptions(&perfTimer, 0,
   				XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);


   XTmrCtr_SetResetValue(&perfTimer, 0,0xFFFFFFFF - RESET_VALUE);

   XIntc_Start(&sys_intc,XIN_REAL_MODE);
   XTmrCtr_Start(&perfTimer, 0);











   microblaze_register_handler((XInterruptHandler)XIntc_DeviceInterruptHandler,
		   (void*)XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
   microblaze_enable_interrupts();







   init_fft_tables(SAMPLES);

   while(1) {
	   if(count >= 10000 ){
		    xil_printf("READ  COUNT: %d / 10000\r\n", read_count);
		    xil_printf("FFT   COUNT: %d / 10000\r\n", fft_count);
		    xil_printf("FIND  COUNT: %d / 10000\r\n", find_count);
		    xil_printf("MAIN COUNT: %d / 10000\r\n", misc_count);
		    xil_printf("TRIG COUNT: %d / 10000\r\n", trig_count);
		    xil_printf("COMPLEX COUNT: %d / 10000\r\n", complex_count);
		    xil_printf("COMPUTATION COUNT: %d / 10000\r\n", computation_count);
		    xil_printf("OTHER COUNT: %d / 10000\r\n", interrupt_count);


	   		read_count = 0;
	   		fft_count = 0;
	   		find_count = 0;
	   		misc_count = 0;
	   		trig_count = 0;
	   		complex_count=0;
	   		computation_count = 0;
	   		interrupt_count = 0;
	   		print_count = 0;
	   		count=0;

	   }


      //Read Values from Microblaze buffer, which is continuously populated by AXI4 Streaming Data FIFO.
      read_fsl_values(q, SAMPLES);


      //printf("Time %d, Seqf %d, Seql %d, Seq_off %d\n", ticks, seqf, seql, seqf-seq_old);
      seq_old = seql;


      sample_f = (100*1000*1000/2048);
      //xil_printf("sample frequency: %d \r\n",(int)sample_f);

      //zero w array
      for(l=0;l<SAMPLES;l++)
         w[l]=0;
      int start = XTmrCtr_GetValue(&timer, 0);



      //xil_printf("TIMER: %d\r\n", perfhTimer);
      //microblaze_disable_interrupts();
      frequency = fft(q, w, SAMPLES, M, sample_f/4);   //divide both by 4
      //microblaze_enable_interrupts();
      int end = XTmrCtr_GetValue(&timer, 0);


      float elapsed = (float)(end - start) / 100000000.0f;
      //printf("FFT time: %.6f seconds\n", elapsed);
      //xil_printf("frequency: %d Hz\r\n", (int)(frequency+.5));
      findNote(frequency);


   }


   return 0;
}
