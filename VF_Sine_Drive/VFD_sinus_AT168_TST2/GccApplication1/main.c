// Funktionierende Version
// - Sinusform definiert (Achtung: an zwei Stellen)
// - _delay_ms überall mit Faktor 20
// - entscheidend war der Block-C am Display
// - 10k Pullup an den Datenleidungen wären nicht schlecht
// - 4-Lagen-Design wichtig


/* VFD 3phase Sine Control with single ATMega 88/168/328                                   */
/*! \file *********************************************************************
 *
 * \brief
 *	VFD = Variable Frequency Drive
 *      This file contains the full implementation of the VFD control,
 *      except the PID-controller.
 *  	To successfully compile it and run on the AVR, 
 *	select -Os optimization in your compiler options.
 * 	Fuse your controller to no CLKDIV8, external fullswing high frequency crystal 
 *	and if you wish enable EESAVE to keep your EEPROM settings. 
 *	Alternatively, you could use the internal RC Oscillator with its 8 Mhz
 *	but the PWM Frequency is well in hearing range then (15.6kHz).
 *
 * \par Roughly based on Application note:
 *      AVR447: Sinusoidal driving of three-phase motor using
 *      ATmega48/88/168/328
 *
 * \author
 * 	Matthias Schoeldgen - DC7UR 
 *
 *	Hardware and LCD
 *  	------------------
 *      A HD44780 Display is connected to
 * 	LCD Data DB4-7 on PC0-PC3
 *  	LCD E to PB0
 * 	LCD RS to PB4
 *	LCD RW to PB5
 *  Connections have been chosen to not interfere with the six PWM outputs.
 *	Peter Fleury's library is used to drive the LCD and it has been modified
 *	See lcd.h for the settings. Make sure to apply pin and XTAL settings
 *      to LCD.h and set DIVISIONEER in vfd.h according to the formula there
 * 	The default settings are for a 16MHz Crystal.
 *      
 *	Three buttons are connected to PD0 to PD2 (grounding the resp. pin)
 *	Button 0 black cycles through parameters
 *	Button 1 red   decreases selected parameter
 *	Button 2 green increases selected parameter
 *	Holding Button 0 and pressing 1 select the extended menu
 *  where the PID parameters and the Deadtime are set
 *  In this menu holding button 0 and pressing button 1 will store current values to the EEPROM
 *  Button 0 and 2 pressed simultaneously in this menu exits and returns to main menu
 *  The EEPROM set is run on power up
 *      
 *  Frequency and Amplitude control:
 *  when jumper on PD4 is set (grounded) control of Frequency and V/Hz is done
 * 	by analog values from ADC inputs 4 and 5 resp.
 *
 *  Direction control is provided through PD7: 
 *	Attention! 
 *	This drive doesn't measure motor rotation and can't determine wether 
 *  the motor is stopped or not. To change direction you should stop the motor first.
 *
 *  The variable Inco is the width between table values
 *  resulting frequency is:
 *
 * (F_CPU/512/256)/(SIN_TABLE_LENGTH/Inco) = Fout
 *
 *  The 'V/Hz' parameter controls the PWM Power according to the frequency
 *  Adjust it for the power of the motor at its working frequency
 *  Those values and the Deadtime can be stored in EEPROM
 * 	The PID parameters are stored there,too
 *
 * 	Contact me: mikrocontroller@schoeldgen.de
 * 	this project compiles with AVR studio 4 and WinAVR
 * 
 * Diagnostic outputs on startup. If everything goes well, you'll probably never see those
 * as they are thrown on the LCD very quickly on start. 
 *  0 - Port & LCD init done
 *  1 - Timer init done
 *  2 - Pinchange Interrupt init done
 *  3 - ADC init done
 *  4 - PID Controller init done
 *  5 - init fastFlags done
 *  6 - init Timer IRQs done
 *  7 - EEPROM init done and keys not stuck
 * 
 ******************************************************************************/
// I really don't know if its necessary to include all this here, but if it is not
// needed it won't eat up flash ROM anyway
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>  
#include <stdint.h>
#include <stdlib.h> 
#include <string.h>
#include <ctype.h>
#include <util/delay.h>
// project headers 
#include "vfd.h"
#include "vfdtables.h"
// PID controller headers
#include "pid.h"
// Peter Fleury's LCD Lib
#include "lcd.h"
/* ------------- Global variables ----*/
/*! \brief Flags for Waveform and Direction 
 *
 * This variable contains all the flags used for VF control.
 * Note that we use direct registers to hold some of the more time-critical
 * variables, although it probably isn't necessary. It is a nice example of
 * using this compiler feature, though.
 
 * The fastFlags register holds a bit set of some control parameters. Its declaration is in vfd.h
 */
register volatile VFDflags_t fastFlags asm ("r9"); 

/*! \brief Increment used for sine table iteration.
 *
 *  This variable holds the increment used for sine table iteration.
 *  It is stored in an 8.8 fixed point format.
 */
register volatile uint16_t sineTableIncrement asm ("r2");

/*! \brief Index into sine table
 *
 *  This variable is used as an index into the sine table. It is
 *  stored in a 8.8 fixed point format, so it can be directly incremented by
 *  sineTableIncrement. Only the high byte is used as table index.
 */
volatile uint16_t sineTableIndex ;        

/*! \brief The amplitude of the generated sine waves.
 *
 *  This variable controls the amplitude of the generated sine waves.
 *  The range is 0-255. Calculated in SpeedController from frequency and V/Hz
 */
register volatile uint8_t amplitude asm("r6");
// This is the frequency to voltage ratio
register volatile uint8_t VperHz asm("r7");
// This is the main frequency control 
register volatile uint8_t Inco asm("r8") ;

/*! \brief current selected parameter
 *
 */
volatile uint8_t parameter = 0;
// calculated frequency * 10
uint16_t freq;
//! The most recent speed input from ADC
volatile uint8_t speedInput;
// desired dead time from eeprom
volatile uint8_t DEAD_TIME_HALF = 20;

/*! \brief Speed controller run flag.
 *
 *  This variable is set to TRUE every time the speed controller should be run.
 */
volatile uint8_t SpeedControllerRun = FALSE;

//! Struct used to hold PID controller parameters and variables.
volatile pidData_t pidParameters;

// EEPROM values preset for generating an .eep file
// adjusting only necessary if the very initial values are not what you want.
// normally you set EEPROM values from the running system
// 
uint8_t ee_DEAD_TIME_HALF __attribute__((section(".eeprom"))) = 20;
// sinTableIncrement where 79 is about 50 Hz
uint8_t ee_Inco __attribute__((section(".eeprom"))) = 79;
// desired V/f ratio
uint8_t ee_VperHz __attribute__((section(".eeprom"))) = 45;
// PID params
uint16_t ee_pid_P __attribute__((section(".eeprom"))) = PID_K_P;
uint16_t ee_pid_I __attribute__((section(".eeprom"))) = PID_K_I;
uint16_t ee_pid_D __attribute__((section(".eeprom"))) = PID_K_D;

/* ----- SUB Programs ---*/
/*! \brief Initializes I/O port directions and pull-up resistors
 *
 *  This function initializes all I/O ports with correct
 *  direction and pull-up resistors, if needed.
 */
static void PortsInit(void)
{
// Port C is LCD Data Output and ADC Input
// the LCD lib does this on its own, but here for completeness
  DDRC = 0b00001111;
// PORTB, PORTD outputs
// clr pullups but do nothing else here. The sine driver will take care of the output states
  PORTB &= ~(PWM_PATTERN_PORTB);
  PORTD &= ~(PWM_PATTERN_PORTD);
// Set the inputs on Port D for the three buttons
  DDRD &= ~BUTTON_MASK;
// Enable pull-up on input signals.
  PORTD |= _BV(DIRECTION_COMMAND_PIN) | _BV(EXTERNAL_CONTROL_PIN) | BUTTON_MASK ;
}
/*! \brief Initializes and synchronizes Timers
 *
 *  This function sets the correct prescaler and starts all
 *  three timers. The timers are synchronized to ensure that
 *  all PWM signals are aligned.
 */
static void TimersInit(void)
{
  //Set all timers in "Phase correct mode". Do not enable outputs yet.
  TCCR0A = (1 << WGM00);
  TCCR1A = (1 << WGM11);	
  TCCR2A = (1 << WGM20);
//Set top value of Timer/counter1.
  ICR1 = 0xff;
//Synchronize timers. These values were determined by running the simulator
  TCNT0 = 0;
  TCNT1 = 3;
  TCNT2 = 5;
  // Start all 3 timers.
  TCCR0B = (0 << CS01) | (1 << CS00);
  TCCR1B = (1 << WGM13) | (0 << CS11) | (1 << CS10);
  TCCR2B = (0 << CS21) | (1 << CS20);
}

/*! \brief Initialize pin change interrupts.
 *
 *  This function initializes pin change interrupt on 
 *  motor direction control input.
 */
static void PinChangeIntInit(void)
{
  // Initialize pin change interrupt on direction command pin
  // PD7 is PCINT23
  PCMSK2 = _BV(PCINT23);
  // Enable pin change interrupt on ports with pin change signals
  PCICR = (1 << PCIE2);
}
/* This really does nothing except restarting the ADC with IRQ enabled
 * If you want to calibrate, do it here.
 */
static void CalibADC(void)
{
 DisablePWMOutputs();
 _delay_ms(20*200);
//Initialize ADC and restart, now with IRQ 
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (0 << ADATE) | (1 << ADIF) | (1 << ADIE) | (ADC_PRESCALER); 
}
/*! 
 *  This function initializes the ADC for frequency and amplitude control
 *  The ADC is triggered once and then relies on 
 *  the ADC complete interrupt service routine to get restarts
 *  Declaration of ADMUX and the other constants can be found in vfd.h
   Purists probably ask why i shift zeros into the byte
 * but its making it all clearer and more easy to change
 * remember this is init only and will only be run ONCE... 
*/

static void ADCInit(void)
{
  ADCSRA = (1 << ADEN) | (0 << ADSC) | (0 << ADATE) | (1 << ADIF) | (0 << ADIE) | (ADC_PRESCALER);
  // disable digital inputs on analog channels
  DIDR0 = ( 1 << ADC_CHANNEL_FREQUENCY) | (1 << ADC_CHANNEL_AMPLITUDE ); 
 //Select initial AD conversion channel.
  ADMUX = ADMUX_FREQUENCY;
  //Set trigger source to free run
  ADCSRB = (0 << ADTS2) | (0 << ADTS1) | (0 << ADTS0);
  //Initialize ADC and start once, no IRQ 
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (0 << ADATE) | (0 << ADIF) | (0 << ADIE) | (ADC_PRESCALER);
  CalibADC();
}

/* now lets initialize the EEPROM and read in our variables
 * as the LCD is now up and running, we can let the user know of things.
 * A fail here would be fatal to the PA. So if somehow a wrong value is read
 * we set to MIN_DEAD_TIME to save the life of the PA.
 */
static void EEPROMInit(void)
{
 	lcd_gotoxy(0,1);
	lcd_puts_P( "Reading EEPROM  ");
	DEAD_TIME_HALF = eeprom_read_byte(&ee_DEAD_TIME_HALF);

	if (DEAD_TIME_HALF < MIN_DEAD_TIME) DEAD_TIME_HALF = MIN_DEAD_TIME;
// Working parameters
	Inco = eeprom_read_byte(&ee_Inco);
	VperHz = eeprom_read_byte(&ee_VperHz);
// PID values
	pidParameters.P_Factor = eeprom_read_word(&ee_pid_P);
	pidParameters.I_Factor = eeprom_read_word(&ee_pid_I);
	pidParameters.D_Factor = eeprom_read_word(&ee_pid_D);
	_delay_ms(20*500);
	lcd_gotoxy(0,1);
	lcd_puts_P( "Read Done       ");
	_delay_ms(20*500);
/* wait for button release
 * Don't ask why this is here --- ok, you may ask: Its here if some fool powers
 * up the VFD and keeps the buttons pressed or the buttons somehow are stuck.
 * That would probly lead to undesired operation and so - - - lets wait
 */ 
	if (keyin() > 0){};   
}
/* brief: button routines
 * three buttons return hex 1,2,4 resp.
 * note that debouncing is not really necessary here as the polling routine 
 * just happens to pass by.
 */
char keyin(void){
uint8_t i = 0;
i = ~BUTTON_PORT;
i &= BUTTON_MASK;
 return i; 
}
// Console//LCD utilities
static void printspc(void)
{
 lcd_putc(' ');
}
/* Some basic routines to print hecadecimal and 
 * decimal numbers plus some conversion stuff.
 * table unfortunately needs to reside in RAM for now
 * but hey, no money back for saved memory
*/
char hextable[18] = "0123456789ABCDEF";
/* very simple output to hex */
static void printbyte(const unsigned char data)
{
	lcd_putc(hextable[data >> 4]);
	lcd_putc(hextable[data & 0x0f]);
}
static void printword(const unsigned int data)
{
	printbyte(data >> 8);
	printbyte(data);
}
/* output a number in decimal style */
char dectable[12] = "000000000000";
char a;
static void printdec(int16_t number) {
uint8_t n,i;
/* yeah, yeah warning "pointer to integer without a cast" 
 * Am i tired of this or what 
 */
	a = itoa(number,dectable,10);
	i = strlen(dectable);
	for (n=0;n < i;n++) {
		if (n == i-1) lcd_putc('.');		
		lcd_putc(dectable[n]);
	}
}
static void printnum(int16_t number) {
uint8_t n,i;
	a = itoa(number,dectable,10);
	i = strlen(dectable);
	for (n=0;n < i;n++) {
		lcd_putc(dectable[n]);
	}
}
/* write values to EEPROM 
 * IRQs must be disabled before writing and are re-enabled afterwards
 * therefore disabling the PWM is safest. Reestablishing sine driving is done
 * by resetting the waveform to undefined. The timer IRQ will handle the remains. 
 */
void wrEEPROM(void) {
 	lcd_gotoxy(0,1);
	lcd_puts_P( "Storing EEPROM  " );
	cli();
	DisablePWMOutputs();
// real writing is here
	eeprom_busy_wait();
	eeprom_write_byte(&ee_DEAD_TIME_HALF,DEAD_TIME_HALF);
	eeprom_busy_wait();
	eeprom_write_byte(&ee_Inco,Inco);
	eeprom_busy_wait();
	eeprom_write_byte(&ee_VperHz,VperHz);
	eeprom_busy_wait();
// write PID values
	eeprom_write_word(&ee_pid_P,pidParameters.P_Factor);
	eeprom_busy_wait();
	eeprom_write_word(&ee_pid_I,pidParameters.I_Factor);
	eeprom_busy_wait();
	eeprom_write_word(&ee_pid_D,pidParameters.D_Factor);
	eeprom_busy_wait();
// tell the interrupt to reestablish Sine Driving
	fastFlags.driveWaveform = WAVEFORM_UNDEFINED;
	sei();
// tell the user
	_delay_ms(20*500);
	lcd_gotoxy(0,1);
	lcd_puts_P( "Stored          " );
	_delay_ms(20*500);
	// wait for button release
	if (keyin() > 0){};
}
/* Cursorpositions for selectable parameters */
const PROGMEM uint8_t cursorextpos[EXTNUMPARAMS+1] = { 0,4,8,13 };

static void showExtendedPars(void)
{
const uint8_t *p = cursorextpos;

	lcd_gotoxy(0,1);  	
	printword(pidParameters.P_Factor);
	printword(pidParameters.I_Factor);
	printword(pidParameters.D_Factor);printspc();
	printbyte(DEAD_TIME_HALF);printspc();
	lcd_gotoxy(pgm_read_byte(p+parameter),1); // set cursor below active parameter
	_delay_ms(20*5); 	
}

/* The extended menu offers access to the PID parameters and the Deadtime setting 
 * The deadtime is in cpu cycles. For a PA with IR2112/2110 and the IRFGP40BC 
 * a setting of 20 seems to be good.
 * Note that a high dead time setting costs resolution and a setting too low will burn the Fuse 
 * or otherwise cause overcurrent. 
 * Adjust in a safe environment with current monitoring. 
 * Place on the LCD is limited, thus PID and DT is displayed as Hex.
 * Wrap around on these values should be avoided, thus the range check
*/
static void execExtendedCommand(void)
{
char n=0;
uint8_t j=0;

 lcd_home();  
 // this line is  only written once, actual values are always displayed in line 2
 lcd_puts_P( "P   I   D    DT " );
 parameter = 0;
 // wait for button release
 while (keyin()!=0){};
// pressing black and green button simultaneously exits this menu
while (n != 5) {
	n = keyin();
	switch (n) 
		{
// cycle through parameters
		case 1 : parameter++; if (parameter > EXTNUMPARAMS) parameter = 0;
			break;
// red button decrements parameter
		case 2 : 
			switch (parameter) {
			 	case 0 : if (pidParameters.P_Factor < 1) pidParameters.P_Factor = 1; 
				 	 pidParameters.P_Factor--; 
					 break;
				case 1 : if (pidParameters.I_Factor < 1) pidParameters.I_Factor = 1; 
					 pidParameters.I_Factor--;
					 break;
				case 2 : if (pidParameters.D_Factor < 1) pidParameters.D_Factor = 1;
					 pidParameters.D_Factor--;
					 break;
				case 3 : DEAD_TIME_HALF--; 
					 if (DEAD_TIME_HALF < MIN_DEAD_TIME) DEAD_TIME_HALF = MIN_DEAD_TIME;
					 break;
				default : break;
			}
			break;
// green button increments parameter
		case 4 : 
			switch (parameter) {
			 	case 0 : if (pidParameters.P_Factor > 65534) pidParameters.P_Factor = 65534; 
 					 pidParameters.P_Factor++;
					 break;
				case 1 : if (pidParameters.I_Factor > 65534) pidParameters.I_Factor = 65534; 
 					 pidParameters.I_Factor++;
					 break;
				case 2 : if (pidParameters.D_Factor > 65534) pidParameters.D_Factor = 65534; 
 					  pidParameters.D_Factor++;
					  break;
				case 3 : DEAD_TIME_HALF++; 
					if (DEAD_TIME_HALF > 200) DEAD_TIME_HALF = 200;
					 break;
				default : break;
				}
			break;
// pressing black (1) and red (2) button simultaneously writes EEPROM
		case 3 : wrEEPROM(); break;
// pressing black and green(4) button simultaneously exits this menu
		case 5 : break;

		default: j = 0; break;
		} // switch
	PID_Init(pidParameters.P_Factor, pidParameters.I_Factor, pidParameters.D_Factor,(pidData_t *) &pidParameters);
// includes a little autorepeat accelerator
	if (keyin() > 0) {
		j++;
		if (j<10) _delay_ms(20*60);
		else {
			_delay_ms(20*10);
			j = 11;
			}
	}
	showExtendedPars();
	} // while
  parameter = 0;
  lcd_clrscr();
 // restore original diplay
  lcd_puts_P( "Freq  V/Hz  Amp " );
 // wait for button release
 while (keyin()!=0){};
}
// Cursorpositions for selectable parameters
const PROGMEM uint8_t cursorpos[NUMPARAMS+1] = { 0,6 };

/* print a line with the most important variables
 * resulting frequency is:
 *
 * (F_CPU/512/256*Steps)/(SINE_TABLE_LENGTH/Inco)
 * though for displaying we multiply by 10 
 * to show accurate frequency in integer math
 * amplitude is shown in percent
 */
static void showPars(void)
{
volatile uint32_t cnt;
const uint8_t *p = cursorpos;

	lcd_gotoxy(0,1);  	
	printdec(freq);printspc();lcd_gotoxy(6,1);
	printdec(VperHz);printspc();printspc();lcd_gotoxy(12,1);
	printnum((uint16_t)(amplitude*100)/255);lcd_putc('%');printspc();
	lcd_gotoxy(pgm_read_byte(p+parameter),1); // set cursor below active parameter
	_delay_ms(20*5); 

	//lcd_clrscr();
//
	//lcd_gotoxy(0,1);  	lcd_putc('1');
	//lcd_gotoxy(6,1);  	lcd_putc('2');
	//lcd_gotoxy(12,1);  	lcd_putc('3');

	
	//for (cnt=0; cnt<=500; cnt++) {;}	
}
/* repeatedly execute this code part. Here we scan the buttons, set params and display stuff
 * Although we blast to LCD quite often, it won't interfere with the 
 * sine drive as this is completely interrupt driven
 * if the "external control" jumper is set only the extended menu option will work
 */
static void execCommand(void)
{
char n;
static uint8_t i;
n = keyin();
// first make sure we get inputs from buttons and not from analog inputs
if (!fastFlags.externalControl){
		switch (n) 
		{
// black button - cycle through parameters
		case 1 : parameter++; if (parameter > NUMPARAMS) parameter = 0;
			break;
// red button  - decrement parameter
		case 2 :
			switch (parameter) {
			case 0 : if (Inco < 1) Inco = 1; 
				Inco--; 
				break;
			case 1 : if (VperHz < 1) VperHz = 1; 
 				VperHz--; 
				break;
			default : break;
			}
			break;
// green button - increment parameter
		case 4 : 
			switch (parameter) {
			case 0 :if (Inco > 254) Inco = 254; 
 				Inco++;  
				break;
			case 1 :if (VperHz > 254) VperHz = 254; 
 				VperHz++; 
				break;
			default : break;
			}
			break;
// pressing first black (1) and then red (2) button simultaneously goes to extended menu
		case 3 : execExtendedCommand();
			 break;
		default: i = 0; break;
		} // switch
    } else { // external control
// with external control we only can go to the extended menu
		switch (n) 
		{
// pressing black (1) and red (2) button simultaneously goes to sub menu
		case 3 : execExtendedCommand();
			 break;
		default: i = 0; break;
		} // switch
   }  // external control
// simple autorepeater
	if (keyin() > 0) {
	i++;
	if (i<10) _delay_ms(20*60);
	else {
		_delay_ms(20*10);
		i = 11;
	   }
	}
    showPars();	
}
/*! \brief Updates global desired direction flag.
 *
 *  Running this function triggers a reading of the direction
 *  input pin. The desiredDirection flag is set accordingly.
 */
static void DesiredDirectionUpdate(void)
{
 if ( bit_is_clear(PIND,DIRECTION_COMMAND_PIN) )
  {
    fastFlags.desiredDirection = DIRECTION_REVERSE;
  }
  else
  {
    fastFlags.desiredDirection = DIRECTION_FORWARD;
  }
}
/* select control method by examining the EXTERNAL_CONTROL_PIN
 * if it is jumpered to ground, we derive V/Hz and Inco from analog signals
 */
static void DesiredControlUpdate(void)
{
 if ( bit_is_clear(PIND,EXTERNAL_CONTROL_PIN) )
  {
    fastFlags.externalControl = TRUE;
  }
  else
  {
    fastFlags.externalControl = FALSE;
  }
}

/* ***************************************************************************************
 *! \brief Main function / initialization
 *
 *  The main function initializes all subsystems needed for sine control
 *  and enables interrupts, which kicks off the fully interrupt-driven
 *  PWM generator. The main function goes into an loop where it
 *  does the LCD and button work. 
 *  During startup we throw some numbers on the display to help when an error in 
 *  any initialization occurs
 *  Lets initialize all variables here - even if its only for completeness
 *******************************************************************************************+
 */

#define PURE_SINE_DRIVE 1 

int main(void)
{
	volatile uint32_t cnt;
	
  cli();
//  variables
  amplitude = 0 ;
  speedInput = 0 ;
  Inco = 1;
  parameter = 0;
 //Initialize peripherals.
  PortsInit();
 /* initialize display, cursor off */
  lcd_init(LCD_DISP_ON);
  lcd_home();
#ifdef PURE_SINE_DRIVE
  lcd_puts_P( "VF Sinus Drive\n" );
#else
  lcd_puts_P( "VF Motor Drive\n" );
#endif
 lcd_putc('0');  // diagnostic outputs
// timers
  TimersInit();
  lcd_putc('1');
// jumpers 
  PinChangeIntInit();
  lcd_putc('2');
// analog 
  ADCInit();
  lcd_putc('3');
  PID_Init(PID_K_P, PID_K_I, PID_K_D,(pidData_t *) &pidParameters);
  lcd_putc('4');
  //Initialize fastflags
  {
    fastFlags.externalControl = FALSE;
    fastFlags.desiredDirection = DIRECTION_FORWARD;
    fastFlags.driveWaveform = WAVEFORM_UNDEFINED;
  }
  lcd_putc('5');
// read the port pins and set fastFlags accordingly
  DesiredDirectionUpdate();
  DesiredControlUpdate();
// Enable Timer1 capture event interrupt.
  TIFR1 = _BV(ICF1) | _BV(OCF1B) | _BV(OCF1A) | _BV(TOV1) ;
  TIMSK1 = _BV(ICIE1);
  lcd_putc('6');
  _delay_ms(20*500);
// read in stored values
  EEPROMInit();
  lcd_putc('7');
  lcd_clrscr();
  lcd_command(LCD_DISP_ON_CURSOR);
 // this line is  only written once, actual values are always displayed in line 2
  lcd_puts_P( "Freq  V/Hz  Amp " );
/* activate the engine by  enabling interrupts globally and let sine driver take over. */
  sei();
/*************************** MAIN LOOP   *************************/
 //the main loop handles user interface 
  while(1)//for(;;)
  {
    execCommand();
  }
}
/*************************** END MAIN LOOP   *************************/
/*
 * From here on most routines are only run from the interrupt driven VFD
 *
 */
/*! \brief  Speed regulator loop.
 *
 *  This function is called every SPEED_REGULATOR_TIME_BASE ticks. In this
 *  implementation, a simple PID controller loop is called if we run from external control inputs
 *  note the internal representation is 10 times the real values.
 */
static void SpeedController(void)
{
int32_t outputValue, amplitudeValue;
 // calculate the frequency
if (fastFlags.externalControl) {
//Calculate an increment setpoint from the analog speed input.
	  int16_t incrementSetpoint = ((uint32_t)speedInput * SPEED_CONTROLLER_MAX_INCREMENT) / SPEED_CONTROLLER_MAX_INPUT;
//PID regulator with feed forward from speed input.
	  outputValue = (uint32_t)speedInput;
	  outputValue += PID_Controller(incrementSetpoint, ((uint16_t)Inco),(pidData_t *) &pidParameters);
	  Inco = (uint16_t)outputValue;
 }
// clamp
 freq = DIVISIONEER/(SINE_TABLE_DISP/Inco);
 amplitudeValue = (freq/10)*VperHz;
 if (amplitudeValue < 1)
  	{
    		amplitudeValue = 0;
  	}
 else if (amplitudeValue > 2549)
  	{
    		amplitudeValue = 2550;
  	}
 amplitude = amplitudeValue / 10;
}

/*! \brief Waits for the start of the next PWM cycle.
 *
 *  Can be used to make sure that a shoot-through
 *  does not occur in the transition between two output waveform generation modes.
 */
static void TimersWaitForNextPWMCycle(void)
{
  //Clear Timer1 Capture event flag.
  TIFR1 = (1 << ICF1);

  //Wait for new Timer1 Capture event flag.
  while ( !(TIFR1 & (1 << ICF1)) )
  {
  }
}
/*! \brief Enables PWM output pins
 *
 *  This function enables PWM outputs by setting the port direction for
 *  all PWM pins as output. The PWM configuration itself is not altered
 *  in any way by running this function.
 */
static void EnablePWMOutputs(void)
{
  DDRB |= PWM_PATTERN_PORTB;
  DDRD |= PWM_PATTERN_PORTD;
}
/*! \brief Disables PWM output pins
 *
 *  This function disables PWM outputs by setting the port direction for
 *  all PWM pins as inputs, thus overriding the PWM. The PWM configuration
 *  itself is not altered in any way by running this function.
 */
static void DisablePWMOutputs(void)
{
  DDRB &= ~PWM_PATTERN_PORTB;
  DDRD &= ~PWM_PATTERN_PORTD;
}
/*! \brief Configures timers for sine wave generation
 *
 *  This function is called every time sine wave generation is
 *  needed. PWM outputs are safely disabled while configuration
 *  registers are changed to avoid unintended driving or shoot-
 *  through.
 */
static void TimersSetModeSinusoidal(void)
{
  //Set PWM pins to input (Hi-Z) while changing modes.
  DisablePWMOutputs();

  //Sets all 3 timers in inverted pair mode.
  TCCR0A = (1 << COM0A1) | (0 << COM0A0) | (1 << COM0B1) | (1 << COM0B0) | (1 << WGM00);
  TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (1 << COM1B1) | (1 << COM1B0) | (1 << WGM11);
  TCCR2A = (1 << COM2A1) | (0 << COM2A0) | (1 << COM2B1) | (1 << COM2B0) | (1 << WGM20);

  //Make sure all outputs are turned off before PWM outputs are enabled.
  OCR0A = OCR1AL = OCR2A = 0;
  OCR0B = OCR1BL = OCR2B = 0xff;

  //Wait for next PWM cycle to ensure that all outputs are updated.
  TimersWaitForNextPWMCycle();

  fastFlags.driveWaveform = WAVEFORM_SINUSOIDAL;
 //Change PWM pins to output again to allow PWM control.
  EnablePWMOutputs();
}

/*! \brief Returns the high and low values with deadband for a given compare value.
 *
 * This function takes as argument a desired compare value and inserts a symmetric
 * deadband. The compare values for high and low side with deadband are returned
 * through the two supplied pointers.
 * The variable \ref DEAD_TIME_HALF is used as deadband and the resulting deadtime
 * will be DEAD_TIME_HALF clock cycles times 2.
 *
 * \param compareValue desired compare value
 * \param compareHighPtr Pointer used to return high side compare value with dead band.
 * \param compareLowPtr  Pointer used to return low side compare value with dead band.
 */
static void InsertDeadband(const uint8_t compareValue, uint8_t * compareHighPtr, uint8_t * compareLowPtr)
{
  if (compareValue <= DEAD_TIME_HALF)
  {
    *compareHighPtr = 0x00;
    *compareLowPtr = compareValue;
  }
  else if (compareValue >= (0xff - DEAD_TIME_HALF))
  {
    *compareHighPtr = 0xff - (2 * DEAD_TIME_HALF);
    *compareLowPtr = 0xff;
  }
  else
  {
    *compareHighPtr = compareValue - DEAD_TIME_HALF;
    *compareLowPtr = compareValue + DEAD_TIME_HALF;
  }
}
/*! \brief Adjusts the sine table index according to the current increment.
 *
 *  This function increases the sine table index with the given increment
 *  The index is then adjusted to be within the table length.
 *
 *  \param increment The increment (in 8.8 format) added to the sine table index.
 */
static void AdjustSineTableIndex(const uint16_t increment)
{
  sineTableIndex += increment ;

  // If the table index is out of bounds, wrap the index around the table end
  // to continue from the beginning. Also wrap the next sector start index.
  if ((sineTableIndex >> 8) >= SINE_TABLE_LENGTH)
  {
    sineTableIndex -= (SINE_TABLE_LENGTH << 8);
  }
}
void ChangeDirection(void)
{
  DesiredDirectionUpdate();
  //Disable driver signals
  DisablePWMOutputs();
  fastFlags.driveWaveform = WAVEFORM_UNDEFINED;
}

/*! \brief Direction input change interrupt service routine.
 *
 *  This ISR is called every time the direction input pin changes
 *  state. The desired direction flag is updated accordingly. 
 */
ISR(PCINT2_vect)
{
  //Call the routine to actually stop the motor and reverse
	ChangeDirection();
}
// 
EMPTY_INTERRUPT(TIMER0_OVF_vect);

// ISR stub for  unused irqs. The AVRs sometimes need to fire an interupt and 
// execute it before OC registers are updated
EMPTY_INTERRUPT(TIMER0_COMPB_vect);

ISR(TIMER0_COMPA_vect,ISR_ALIASOF (TIMER0_COMPB_vect));

/*! \brief Timer1 Capture Event interrupt service routine.
 *
 * This interrupt service routine is run everytime the up-down counting timer1
 * reaches TOP (0xff). New sinusoidal output values are calculated and the
 * timers are updated to reflect the new values.
 * The core routine of the VF Drive
 */
ISR(TIMER1_CAPT_vect)
{
static uint8_t speedRegTicks = 0;
uint8_t tempU, tempV, tempW;
    {
  if (fastFlags.driveWaveform != WAVEFORM_SINUSOIDAL) TimersSetModeSinusoidal();
      const uint8_t *sineTablePtr = sineTable ;  // set to start of sine table
      sineTableIncrement = Inco;			// stepwidth = frequency selection
      AdjustSineTableIndex(sineTableIncrement);  // call the routine to update the pointer

      //Add sine table offset to pointer. Must be multiplied by 3, since one
      //value for each phase is stored in the table.
      sineTablePtr += (sineTableIndex >> 8) * 3;

      tempU = pgm_read_byte(sineTablePtr++);
      if (fastFlags.desiredDirection == DIRECTION_FORWARD)
      {
        tempW = pgm_read_byte(sineTablePtr++);
        tempV = pgm_read_byte(sineTablePtr);
      }
      else
      {
        tempV = pgm_read_byte(sineTablePtr++);
        tempW = pgm_read_byte(sineTablePtr);
      }
    }
/* Scale sine modulation values to the current amplitude.
 * Pure sine drive needs to be symmetrical to zero 
 * Motor drive doesn't
 */
#ifdef PURE_SINE_DRIVE
    tempU = 128 - ((uint16_t)(amplitude * tempU) >> 9);
    tempV = 128 - ((uint16_t)(amplitude * tempV) >> 9);
    tempW = 128 - ((uint16_t)(amplitude * tempW) >> 9);
#else
    tempU = ((uint16_t)(amplitude * tempU) >> 8);
    tempV = ((uint16_t)(amplitude * tempV) >> 8);
    tempW = ((uint16_t)(amplitude * tempW) >> 8);
#endif
    {
    	uint8_t compareHigh, compareLow;
    	InsertDeadband(tempU, &compareHigh, &compareLow);
		OCR0A = compareHigh;
		OCR0B = compareLow;

    	InsertDeadband(tempV, &compareHigh, &compareLow);
		OCR1AL = compareHigh;
		OCR1BL = compareLow;

    	InsertDeadband(tempW, &compareHigh, &compareLow);
		OCR2A = compareHigh;
		OCR2B = compareLow;
    }
// regular task calls the SpeedController 
  	speedRegTicks++;
    if (speedRegTicks >= SPEED_CONTROLLER_TIME_BASE)
    {
		SpeedController();
     	speedRegTicks = 0; 
	}
}
/*! \brief AD conversion complete interrupt service routine
 *
 *  This interrupt service routine is automatically executed every time
 *  an AD conversion is finished and the converted result is available
 *  in the ADC data register.
 *
 *  The switch/case construct makes sure the converted value is stored in the
 *  variable corresponding to the selected channel and changes the channel for
 *  the next ADC measurement.
 *
 *  More ADC measurements can be added to the cycle by extending the switch/
 *  case construct.
 *  to avoid interfering with the main generator routine, 
 *  this is declared as non-blocking
 *
 *  Only the 8 most significant bits of the ADC result are used.
 */
ISR(ADC_vect, ISR_NOBLOCK)
{
  if (fastFlags.externalControl) {
  switch (ADMUX)
  {
  case ADMUX_FREQUENCY:
    	speedInput = (speedInput + ADCH) >> 1; // do some averaging
    	ADMUX = ADMUX_AMPLITUDE;
    	break;
  case ADMUX_AMPLITUDE:
    	VperHz = (VperHz + ADCH) >> 1;
    	ADMUX = ADMUX_FREQUENCY; 
    	break;
  default:
    //This is probably an error and should be handled. Instead we set the multiplexer and just run away
    	ADMUX = ADMUX_FREQUENCY; 
    break;
  } 
// restart the ADC
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (0 << ADATE) | (1 << ADIF) | (1 << ADIE) | ADC_PRESCALER;
 }
}
const PROGMEM char VersionandCopyright[36] = "1.1 (c)2009-2017 matzetronics";
const PROGMEM char EndofFile[22] = "Ende der Fahnenstange";
