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

#define	SONAR_PORT	GPIO_PORTD_BASE
#define TRIG_PIN	GPIO_PIN_1
#define ECHO_PIN	GPIO_PIN_0
#define CLK			80000000
unsigned long xyu = 0;

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
		cnt -= SysTickValueGet();
		cnt = (cnt/80000);
		GPIOIntTypeSet(SONAR_PORT, ECHO_PIN, GPIO_RISING_EDGE);
		state = 0;

	}else{
		cnt = SysTickValueGet();
		GPIOIntTypeSet(SONAR_PORT, ECHO_PIN, GPIO_FALLING_EDGE);
		state = 1;
	}



}

void hw_init(void){
	SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

	SysTickPeriodSet(0xffffffff);
	SysTickEnable();

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	GPIODirModeSet(SONAR_PORT, TRIG_PIN, GPIO_DIR_MODE_OUT);

	GPIOPinTypeGPIOInput(SONAR_PORT, ECHO_PIN);
	GPIOIntTypeSet(SONAR_PORT, ECHO_PIN, GPIO_RISING_EDGE);
	GPIOPinIntEnable(SONAR_PORT,ECHO_PIN);
	IntEnable(INT_GPIOD);

	//Timer configuration
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	TimerConfigure(TIMER0_BASE, TIMER_CFG_A_PERIODIC);
	const long timer_match = (CLK/1000000)*10;
	const long timer_out = (CLK/1000)*100;
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

