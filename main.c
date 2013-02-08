/*
 * main.c
 */

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/systick.h"
#include "driverlib/uart.h"

#define	SONAR_PORT	GPIO_PORTD_BASE
#define TRIG_PIN	GPIO_PIN_1
#define ECHO_PIN	GPIO_PIN_0
#define	SERVO_PIN	GPIO_PIN_2

unsigned long f_cpu = 0;
unsigned long dist = 0;


void hw_init(void){
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	f_cpu = SysCtlClockGet();
	SysTickPeriodSet(0xffffffff);
	SysTickEnable();
//	UARTStdioInit();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIODirModeSet(SONAR_PORT, TRIG_PIN, GPIO_DIR_MODE_OUT);
	GPIODirModeSet(SONAR_PORT, SERVO_PIN, GPIO_DIR_MODE_OUT);

	GPIOPinTypeGPIOInput(SONAR_PORT, ECHO_PIN);
	GPIOIntTypeSet(SONAR_PORT, ECHO_PIN, GPIO_RISING_EDGE);
	GPIOPinIntEnable(SONAR_PORT,ECHO_PIN);
	IntEnable(INT_GPIOD);

	//Timer configuration
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
	const long timer_match = (f_cpu/1000000)*10;
	const long timer_out = (f_cpu/1000)*80;
	TimerLoadSet(TIMER0_BASE, TIMER_A, timer_out);
	TimerMatchSet(TIMER0_BASE, TIMER_A, timer_match);
	TimerEnable(TIMER0_BASE, TIMER_A);
	TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
	TimerIntClear(TIMER0_BASE,TIMER_A);
	IntEnable(INT_TIMER0A);

	IntMasterEnable();

}



void main(void) {
	hw_init();
	while(1){

	}

}

void Timer0AIntHandler(void){
	static char flag = 0;

	if(flag){
		GPIOPinWrite(SONAR_PORT,TRIG_PIN,0);
		TimerIntDisable(TIMER0_BASE, TIMER_TIMA_MATCH);
		TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
		flag = 0;
	}else{
		GPIOPinWrite(SONAR_PORT,TRIG_PIN,1);
		TimerIntDisable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
		TimerIntEnable(TIMER0_BASE, TIMER_TIMA_MATCH);
		flag = 1;
	}
}




void GPIODIntHandler(void){

	static unsigned long cnt = 0;
	static int state = 0;

	if (state){

		//dist = (f_cpu/(SysTickValueGet()-cnt))*58;
		dist = SysTickValueGet();
		dist = cnt - dist;
		GPIOIntTypeSet(SONAR_PORT, ECHO_PIN, GPIO_RISING_EDGE);
		state = 0;

	}else{
		cnt = SysTickValueGet();
		GPIOIntTypeSet(SONAR_PORT, ECHO_PIN, GPIO_FALLING_EDGE);
		state = 1;
	}
}


