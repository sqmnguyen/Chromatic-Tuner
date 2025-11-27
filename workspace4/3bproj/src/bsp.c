/*****************************************************************************
* bsp.c for Lab2A of ECE 153a at UCSB
* Date of the Last Update:  October 27,2019
*****************************************************************************/

/**/
#include "xparameters.h"
#include "qpn_port.h"
#include "bsp.h"
#include "lab2a.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_cache.h"		// Cache Drivers
#include "lcd.h"
#include "xspi.h"
#include "xtmrctr.h"		// Timer Drivers
#include "xtmrctr_l.h" 		// Low-level timer drivers

/*****************************/

/* Define all variables and Gpio objects here  */

XGpio twist_Gpio;
XGpio sw_Gpio;
XGpio GpioLED; //The 16 LEDs that need to shift
void Encoder_Handler(void);
void Button_Handler(void);
#define GPIO_CHANNEL1 1
volatile int i_cnt;
static XTmrCtr axiTimer;
void debounceInterrupt(); // Write This function

// Create ONE interrupt controllers XIntc
XIntc sys_intc;

// Create two static XGpio variables
static XGpio isr_btn;
static XGpio isr_rot;
// Suggest Creating two int's to use for determining the direction of twist
int fsm_state = 0;
int dire = 0;
// 8====D
//timer counter handler:
void TimerCounterHandler(void *CallBackRef, u8 TmrCtrNumber)
{
	xil_printf("CALL ME\r\n");
	QActive_postISR((QActive *)&AO_Lab2A,TICK_SIG);
	return;
}
// LCD setup + display:
void LCD_display(void){
	xil_printf("continuous call \n");
	static XSpi spi;
	XSpi_Config *spiConfig;
	static XGpio dc;

	u32 status;
	u32 controlReg;

	Xil_ICacheEnable();
	Xil_DCacheEnable();
	status = XIntc_Initialize(&sys_intc, XPAR_INTC_0_DEVICE_ID);
	if (status != XST_SUCCESS) {
		xil_printf("Initialize interrupt controller fail!\n");
		return XST_FAILURE;
	}

	if (status != XST_SUCCESS) {
		xil_printf("Connect IHR fail!\n");
		return XST_FAILURE;
	}
	status = XIntc_Start(&sys_intc, XIN_REAL_MODE);
	if (status != XST_SUCCESS) {
		xil_printf("Start interrupt controller fail!\n");
		return XST_FAILURE;
	}

	// Enable interrupt
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR);
	microblaze_enable_interrupts();

	/*
	 * Initialize the GPIO driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	status = XGpio_Initialize(&dc, XPAR_SPI_DC_DEVICE_ID);
	if (status != XST_SUCCESS)  {
		xil_printf("Initialize GPIO dc fail!\n");
		return;
	}

	/*
	 * Set the direction for all signals to be outputs
	 */
	XGpio_SetDataDirection(&dc, 1, 0x0);



	/*
	 * Initialize the SPI driver so that it is  ready to use.
	 */
	spiConfig = XSpi_LookupConfig(XPAR_SPI_DEVICE_ID);
	if (spiConfig == NULL) {
		xil_printf("Can't find spi device!\n");
		return XST_DEVICE_NOT_FOUND;
	}

	status = XSpi_CfgInitialize(&spi, spiConfig, spiConfig->BaseAddress);
	if (status != XST_SUCCESS) {
		xil_printf("Initialize spi fail!\n");
		return XST_FAILURE;
	}

	/*
	 * Reset the SPI device to leave it in a known good state.
	 */
	XSpi_Reset(&spi);

	/*
	 * Setup the control register to enable master mode
	 */
	controlReg = XSpi_GetControlReg(&spi);
	XSpi_SetControlReg(&spi,
			(controlReg | XSP_CR_ENABLE_MASK | XSP_CR_MASTER_MODE_MASK) &
			(~XSP_CR_TRANS_INHIBIT_MASK));

	// Select 1st slave device
	XSpi_SetSlaveSelectReg(&spi, ~0x01);

	initLCD();

	clrScr();

	/*setColor(255, 0, 0);
	fillRect(20, 20, 220, 300);

	setColor(0, 50, 100);
	fillRect(40, 180, 200, 250);

	setColor(0, 255, 0);
	setColorBg(255, 0, 0);
	lcdPrint("Hello !!!!!", 40, 60);

	setFont(BigFont);
	lcdPrint("<# WORLD #>", 40, 80);

	setFont(SevenSegNumFont);
	setColor(238, 64, 0);
	setColorBg(0, 50, 100);*/

	xil_printf("End\n");
}
/*..........................................................................*/
void BSP_init(void) {
	XStatus Status;
	Status = XST_SUCCESS;
/* Setup LED's, etc */
	LCD_display();

	static XGpio dc;
/* Setup interrupts and reference to interrupt handler function(s)  */
	Status = XIntc_Initialize(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		if (Status == XST_DEVICE_NOT_FOUND) {
			xil_printf("XST_DEVICE_NOT_FOUND...\r\n");
		} else {
			xil_printf("a different error from XST_DEVICE_NOT_FOUND...\r\n");
		}
		xil_printf("Interrupt controller driver failed to be initialized...\r\n");

	}
	xil_printf("Interrupt controller driver initialized!\r\n");
	Status = XGpio_Initialize(&dc, XPAR_SPI_DC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialize GPIO dc fail!\n");
		return XST_FAILURE;
	}

	Status = XTmrCtr_Initialize(&axiTimer, XPAR_AXI_TIMER_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialize timer fail!\n");
		return XST_FAILURE;
	}


	Status = XIntc_Connect(&sys_intc,
				XPAR_MICROBLAZE_0_AXI_INTC_AXI_TIMER_0_INTERRUPT_INTR,
				(XInterruptHandler)XTmrCtr_InterruptHandler,
				(void *)&axiTimer);
	if (Status != XST_SUCCESS) {
		xil_printf("Connect IHR fail!\n");
		return XST_FAILURE;
	}

	XTmrCtr_SetHandler(&axiTimer, TimerCounterHandler, &axiTimer);

	/*
	 * Enable the interrupt of the timer counter so interrupts will occur
	 * and use auto reload mode such that the timer counter will reload
	 * itself automatically and continue repeatedly, without this option
	 * it would expire once only
	 */
	XTmrCtr_SetOptions(&axiTimer, 0,
				XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION);
	u32 resetVal = 0xFFFFFFFFu - (XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ - 1u);
	XTmrCtr_SetResetValue(&axiTimer, 0, resetVal);


	/*
	 * Start the timer counter such that it's incrementing by default,
	 * then wait for it to timeout a number of times
	 */
	XTmrCtr_Start(&axiTimer, 0);
	xil_printf("Timer start!\n");

	/*
	 * Initialize the interrupt controller driver so that it's ready to use.
	 * specify the device ID that was generated in xparameters.h
	 *
	 * Initialize GPIO and connect the interrupt controller to the GPIO.
	 *
	 */
	XGpio_Initialize(&isr_btn, XPAR_AXI_GPIO_BTN_DEVICE_ID);
	XGpio_Initialize(&isr_rot, XPAR_ENCODER_DEVICE_ID);
	XIntc_Connect(&sys_intc,
	              XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR,
	              (XInterruptHandler)GpioHandler,
	              &isr_btn);

	XIntc_Connect(&sys_intc,
	              XPAR_MICROBLAZE_0_AXI_INTC_ENCODER_IP2INTC_IRPT_INTR,
	              (XInterruptHandler)TwistHandler,
	              &isr_rot);

	// Press Knob
	XGpio_InterruptEnable(&isr_btn, 1);        // Enable channel 1 interrupts (for button)
	XGpio_InterruptGlobalEnable(&isr_btn);     // Enable global interrupts for that GPIO

	// Twist Knob
	XGpio_InterruptEnable(&isr_rot, 1);        // Enable channel 1 interrupts (for encoder)
	XGpio_InterruptGlobalEnable(&isr_rot);     // Enable global interrupts for that GPIO
		
}
/*..........................................................................*/
void QF_onStartup(void) {                 /* entered with interrupts locked */

/* Enable interrupts */
	xil_printf("\n\rQF_onStartup\n"); // Comment out once you are in your complete program



	/* Enable GPIO interrupt sources */


	/* If A/B generate IRQs on the Encoder GPIO, enable those bits too */
	XGpio_InterruptEnable(&isr_rot, /* ROT_A_MASK | ROT_B_MASK */ 0xFFFFFFFFu);
	XGpio_InterruptGlobalEnable(&isr_rot);

	// Global enable of interrupt
	XGpio_InterruptGlobalEnable(&isr_btn);
	// Enable interrupt on the GPIO
	XGpio_InterruptEnable(&isr_btn, 0x1 /* | other btn bits if used */);

	/* General CPU exception hookup */
	Xil_ExceptionInit();
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
								 (Xil_ExceptionHandler)XIntc_InterruptHandler,
								 &sys_intc);
	Xil_ExceptionEnable();

	// Enable interrupt controller
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR);
	XIntc_Enable(&sys_intc, XPAR_MICROBLAZE_0_AXI_INTC_ENCODER_IP2INTC_IRPT_INTR);

	// Start interrupt controller
	XIntc_Start(&sys_intc, XIN_REAL_MODE);
	// register handler with Microblaze
	microblaze_register_handler(
		(XInterruptHandler) XIntc_DeviceInterruptHandler,
		(void*) XPAR_MICROBLAZE_0_AXI_INTC_DEVICE_ID);
	microblaze_enable_interrupts();

	xil_printf("\n Interrupts Enabled for Encoder and Button\r\n");

	// Twist Knob

	// General
	// Initialize Exceptions
	// Press Knob
	// Register Exception
	// Twist Knob
	// Register Exception
	// General
	// Enable Exception

	// Variables for reading Microblaze registers to debug your interrupts.
//	{
//		u32 axi_ISR =  Xil_In32(intcPress.BaseAddress + XIN_ISR_OFFSET);
//		u32 axi_IPR =  Xil_In32(intcPress.BaseAddress + XIN_IPR_OFFSET);
//		u32 axi_IER =  Xil_In32(intcPress.BaseAddress + XIN_IER_OFFSET);
//		u32 axi_IAR =  Xil_In32(intcPress.BaseAddress + XIN_IAR_OFFSET);
//		u32 axi_SIE =  Xil_In32(intcPress.BaseAddress + XIN_SIE_OFFSET);
//		u32 axi_CIE =  Xil_In32(intcPress.BaseAddress + XIN_CIE_OFFSET);
//		u32 axi_IVR =  Xil_In32(intcPress.BaseAddress + XIN_IVR_OFFSET);
//		u32 axi_MER =  Xil_In32(intcPress.BaseAddress + XIN_MER_OFFSET);
//		u32 axi_IMR =  Xil_In32(intcPress.BaseAddress + XIN_IMR_OFFSET);
//		u32 axi_ILR =  Xil_In32(intcPress.BaseAddress + XIN_ILR_OFFSET) ;
//		u32 axi_IVAR = Xil_In32(intcPress.BaseAddress + XIN_IVAR_OFFSET);
//		u32 gpioTestIER  = Xil_In32(sw_Gpio.BaseAddress + XGPIO_IER_OFFSET);
//		u32 gpioTestISR  = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_ISR_OFFSET ) & 0x00000003; // & 0xMASK
//		u32 gpioTestGIER = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_GIE_OFFSET ) & 0x80000000; // & 0xMASK
//	}
}


void QF_onIdle(void) {        /* entered with interrupts locked */

    QF_INT_UNLOCK();                       /* unlock interrupts */

    {
    	// Write code to increment your interrupt counter here.
    	i_cnt++;
    	//QActive_postISR((QActive *)&AO_Lab2A, ENCODER_DOWN); is used to post an event to your FSM



// 			Useful for Debugging, and understanding your Microblaze registers.
//    		u32 axi_ISR =  Xil_In32(intcPress.BaseAddress + XIN_ISR_OFFSET);
//    	    u32 axi_IPR =  Xil_In32(intcPress.BaseAddress + XIN_IPR_OFFSET);
//    	    u32 axi_IER =  Xil_In32(intcPress.BaseAddress + XIN_IER_OFFSET);
//
//    	    u32 axi_IAR =  Xil_In32(intcPress.BaseAddress + XIN_IAR_OFFSET);
//    	    u32 axi_SIE =  Xil_In32(intcPress.BaseAddress + XIN_SIE_OFFSET);
//    	    u32 axi_CIE =  Xil_In32(intcPress.BaseAddress + XIN_CIE_OFFSET);
//    	    u32 axi_IVR =  Xil_In32(intcPress.BaseAddress + XIN_IVR_OFFSET);
//    	    u32 axi_MER =  Xil_In32(intcPress.BaseAddress + XIN_MER_OFFSET);
//    	    u32 axi_IMR =  Xil_In32(intcPress.BaseAddress + XIN_IMR_OFFSET);
//    	    u32 axi_ILR =  Xil_In32(intcPress.BaseAddress + XIN_ILR_OFFSET) ;
//    	    u32 axi_IVAR = Xil_In32(intcPress.BaseAddress + XIN_IVAR_OFFSET);
//
//    	    // Expect to see 0x00000001
//    	    u32 gpioTestIER  = Xil_In32(sw_Gpio.BaseAddress + XGPIO_IER_OFFSET);
//    	    // Expect to see 0x00000001
//    	    u32 gpioTestISR  = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_ISR_OFFSET ) & 0x00000003;
//
//    	    // Expect to see 0x80000000 in GIER
//    		u32 gpioTestGIER = Xil_In32(sw_Gpio.BaseAddress  + XGPIO_GIE_OFFSET ) & 0x80000000;


    }
}

/* Q_onAssert is called only when the program encounters an error*/
/*..........................................................................*/
void Q_onAssert(char const * const file, int line) {
    (void)file;                                   /* name of the file that throws the error */
    (void)line;                                   /* line of the code that throws the error */
    QF_INT_LOCK();
    printDebugLog();
    for (;;) {
    }
}

/* Interrupt handler functions here.  Do not forget to include them in lab2a.h!
To post an event from an ISR, use this template:
QActive_postISR((QActive *)&AO_Lab2A, SIGNALHERE);
Where the Signals are defined in lab2a.h  */

/******************************************************************************
*
* This is the interrupt handler routine for the GPIO for this example.
*
******************************************************************************/
void GpioHandler(void *CallbackRef) {
	xil_printf("Button set up\r\n");

	XGpio *g = CallbackRef;
	uint32_t ButtonStatusReg;
	ButtonStatusReg = XGpio_DiscreteRead(g, 1);

	//BTNR
	if (ButtonStatusReg & 0x04){
		QActive_postISR((QActive *)&AO_Lab2A,MODE_RIGHT);
	}

	//BTND
	if (ButtonStatusReg & 0x08){
		QActive_postISR((QActive *)&AO_Lab2A,MODE_DOWN);
	}
	//BTNL
	if (ButtonStatusReg & 0x02){
		QActive_postISR((QActive *)&AO_Lab2A,MODE_LEFT);
	}

	//BTNU
	if (ButtonStatusReg & 0x01){
		QActive_postISR((QActive *)&AO_Lab2A,MODE_UP);
	}
	XGpio_InterruptClear(g, 1);

}

void TwistHandler(void *CallbackRef){
	XGpio *twist_Gpio = CallbackRef;

	uint32_t status = XGpio_DiscreteRead(twist_Gpio, 1);
	XGpio_InterruptClear(twist_Gpio,1);
	debounceTwistInterrupt();

}

void debounceTwistInterrupt(){
	// Read both lines here? What is twist[0] and twist[1]?
	uint32_t value = XGpio_DiscreteRead(&isr_rot, 1);
	if(value & 0b100){
		QActive_postISR((QActive *)&AO_Lab2A,ENCODER_CLICK);
	}
	uint32_t EncoderStatusReg;

	EncoderStatusReg = XGpio_DiscreteRead(&isr_rot, 1);
	//xil_printf("The ENCODER is: %d\n", EncoderStatusReg);
	// Decode valid CW/CCW transitions




	switch(fsm_state){
		case 0: //11
			if (EncoderStatusReg == 1) {
				fsm_state = 1;
				dire = 1;
			}
			else if (EncoderStatusReg == 2) {
				fsm_state = 4;
				dire = -1;
			}
			break;
		case 1: //01 and CW start
			if (dire == 1 && EncoderStatusReg == 0){
				fsm_state = 2;
			}
			else if (EncoderStatusReg == 3){
				fsm_state = 0;
				dire = 0;
			}
			break;
		case 2: //00 and CW step 1
			if (EncoderStatusReg == 2 && dire == 1){
				fsm_state = 3;
			}
			else if (EncoderStatusReg == 1){
				fsm_state = 1;
			}
			break;
		case 3: //10 CW step 2
			if (EncoderStatusReg == 3 && dire == 1){
				fsm_state = 0;
				QActive_postISR((QActive *)&AO_Lab2A, ENCODER_UP);
				break;
			}
			else if (EncoderStatusReg == 0){
				fsm_state = 2;
			}
			break;
		case 4: //CCW start 10
			if (EncoderStatusReg == 0 && dire == -1){
				fsm_state = 5;
			}
			else if(EncoderStatusReg == 3){
				fsm_state = 0;
				dire = 0;
			}
			break;
		case 5: //CCW step 1 00
			if (EncoderStatusReg == 1 && dire == -1){
				fsm_state = 6;
			}
			else if (EncoderStatusReg == 2){
				fsm_state = 4;
			}
			else if(EncoderStatusReg == 0){
				fsm_state = 5;
			}
			break;
		case 6: //CCW step 2 01
			if (EncoderStatusReg == 3 && dire == -1){
				fsm_state = 0;
				QActive_postISR((QActive *)&AO_Lab2A, ENCODER_DOWN);
				break;
			}
			else if (EncoderStatusReg == 0){
				fsm_state = 5;
			}
			break;
	}




	// Clear interrupt

	XGpio_InterruptClear(&isr_rot, 1);
// How can you use reading from the two GPIO twist input pins to figure out which way the twist is going?




}

void debounceInterrupt() {
	QActive_postISR((QActive *)&AO_Lab2A, ENCODER_CLICK);
	// XGpio_InterruptClear(&sw_Gpio, GPIO_CHANNEL1); // (Example, need to fill in your own parameters
}
