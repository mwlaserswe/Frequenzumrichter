/*! \file *********************************************************************
 *
 * \brief
 * 	
 *      VFD control header file.
 *
 *      This file contains all #defines, typedefs and prototypes related to
 *      the VFD control project.
 *
 * \par Based on Application note:
 *      AVR447: Sinusoidal driving of three-phase permanent motor using
 *      ATmega48/88/168
 *      stripped from anything Sensor related 
 *
 *
 * \author
 * 	 Matthias Schoeldgen - DC7UR 
 * $Revision: 0
 * $RCSfile: vfd.h,v $
 * $Date: 2012/12/22 07:20:51 $  \n
 ******************************************************************************/

#ifndef _VFD_H_
#define _VFD_H_
// ADAPT THIS TO YOUR CPU Frequency !!!
// (F_CPU*1000)/512/256
#define DIVISIONEER 122070U

/* Normally you won't need to alter anything after this line */
//! FALSE constant value.

#define FALSE   0

//! TRUE constant value, defined to be compatible with comparisons.
#define TRUE    (!FALSE)

// fallback for EEPROM errors
#define MIN_DEAD_TIME 8
// parameters in the Standard menu - 1
#define NUMPARAMS 1
// parameters in the extended menu - 1
#define EXTNUMPARAMS 3

#define BUFFLENGTH 32

//! The number of elements in the sine modulation table per phase.
#define SINE_TABLE_LENGTH 192U

#if SINE_TABLE_LENGTH != 192U
#warning "Changing sine modulation table length can have unwanted side effects. Consult the documentation for more information."
#endif

//! The number of elements in the sine modulation table for calculating fine display values.
#define SINE_TABLE_DISP  SINE_TABLE_LENGTH*100

//! Bit pattern of PWM pins placed on PORTB.
#define PWM_PATTERN_PORTB   ((1 << PB1) | (1 << PB2) | (1 << PB3))

//! Bit pattern of PWM pins placed on PORTD.
#define PWM_PATTERN_PORTD   ((1 << PD3) | (1 << PD5) | (1 << PD6))

//! Forward direction flag value.
#define DIRECTION_FORWARD       0

//! Reverse direction flag value.
#define DIRECTION_REVERSE       1

//! Unknown direction flag value.
#define DIRECTION_UNKNOWN       3


//! The ADC channel where the analog frequency control is connected.
#define ADC_CHANNEL_FREQUENCY   4

//! The ADC channel where the analog V/Hz input is connected.
#define ADC_CHANNEL_AMPLITUDE     5
//! ADC clock prescaler 2 setting.
#define ADC_PRESCALER_2        ((0 << ADPS2) | (0 << ADPS1) | (1 << ADPS0))

//! ADC clock prescaler 4 setting.
#define ADC_PRESCALER_4        ((0 << ADPS2) | (1 << ADPS1) | (0 << ADPS0))

//! ADC clock prescaler 8 setting.
#define ADC_PRESCALER_8        ((0 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))

//! ADC clock prescaler 16 setting.
#define ADC_PRESCALER_16       ((1 << ADPS2) | (0 << ADPS1) | (0 << ADPS0))

//! ADC clock prescaler 64 setting.
#define ADC_PRESCALER_64       ((1 << ADPS2) | (1 << ADPS1) | (0 << ADPS0))

//! ADC clock prescaler used in this application.
#define ADC_PRESCALER  ADC_PRESCALER_64

//! ADC internal voltage reference channel value.
#define ADC_REFERENCE_VOLTAGE_INTERNAL      ((1 << REFS1) | (1 << REFS0))

//! ADC VCC voltage reference channel value.
#define ADC_REFERENCE_VOLTAGE_VCC           ((0 << REFS1) | (1 << REFS0))

//! ADC AREF voltage reference channel value.
#define ADC_REFERENCE_VOLTAGE_AREF          ((0 << REFS1) | (0 << REFS0))

/*! ADC voltage reference used in this application.
 *
 *  \todo Select ADC voltage reference channel.
 */
#define ADC_REFERENCE_VOLTAGE   ADC_REFERENCE_VOLTAGE_VCC

//! ADMUX settings for Frequency input
#define ADMUX_FREQUENCY   (ADC_REFERENCE_VOLTAGE | (1 << ADLAR) | (ADC_CHANNEL_FREQUENCY << MUX0))

//! ADMUX settings for V/f input
#define ADMUX_AMPLITUDE   (ADC_REFERENCE_VOLTAGE | (1 << ADLAR) | (ADC_CHANNEL_AMPLITUDE << MUX0))


/*! Pin where direction command input is located.
 *  and the three input buttons
 *  Do not change to pin on a different PORT!
 */
#define DIRECTION_COMMAND_PIN   PD7
// if this pin is grounded on power up we switch to the 
//ADC inputs 4 and 5 to control frequency and amplitude resp.
#define EXTERNAL_CONTROL_PIN    PD4
// button inputs
#define BUTTON_PORT PIND
#define BUTTON_MASK ((1<<PD0) | (1<<PD1) | (1<<PD2))

//! Waveform status flag for sinusoidal driving.
#define WAVEFORM_SINUSOIDAL           1

//! Waveform status flag used in transitions between different types of driving.
#define WAVEFORM_UNDEFINED            3

/*! The number of ticks between each iteration of the speed loop.
 *  \todo Adjust speed control loop time base.
 */
#define SPEED_CONTROLLER_TIME_BASE   200

/*! PID controller proportional gain constant.
 *  \todo Adjust PID controller proportional gain. (Only for closed loop)
 * These here are only initial values and can be changed in the controller software
 */
#define PID_K_P    96

/*! PID controller integral gain constant.
 *  \todo Adjust PID controller integral gain. (Only for closed loop)
 */
#define PID_K_I    8

/*! PID controller derivative gain constant.
 *  \todo Adjust PID controller derivative gain. (Only for closed loop)
 */
#define PID_K_D    16

/*! The maximum increment (maximum frequency) to use as setpoint when the maximum
 *  speed reference value is input.
 *  \todo Adjust maximum increment. (Maximum frequency, used by speed controller)
 */
#define SPEED_CONTROLLER_MAX_INCREMENT      255

/*! Max speed reference input. (Rounded up to closest power of 2 in this case,
 *  which is recommended to speed up division.
 *  When using the current implementation the only useful value is 256
 *  \todo Adjust Maximum speed reference input value.
 */
#define SPEED_CONTROLLER_MAX_INPUT          256

//Typedefs
/*! \brief Collection of all motor control flags.
 *
 *  This struct contains all motor control flags used in this implementation.
 */
typedef struct VFDflags
{
  uint8_t externalControl: 1; 	//! if pot control is selected, this is true
  uint8_t desiredDirection : 1; //! The desired direction of rotation.
  uint8_t driveWaveform : 2;    //! The current waveform that should be produced.
} VFDflags_t;


//Function prototypes
// static void ConfigClockSystem( void );
static void TimersInit(void);
static void TimersSetModeSinusoidal(void);
static void TimersWaitForNextPWMCycle(void);
static void PortsInit(void);
static void PinChangeIntInit(void);
static void ADCInit(void);
static void EEPROMInit(void);
// terminal expansion
char keyin(void);
int pow16(unsigned char exponent);
int hex2dec(char * hexcode);
static void showPars(void);
static void execCommand(void);
//
static void SpeedController(void);
static void DesiredDirectionUpdate(void);
static void DesiredControlUpdate(void);
static void InsertDeadband(const uint8_t compareValue, uint8_t * compareHighPtr, uint8_t * compareLowPtr);
static void AdjustSineTableIndex(const uint16_t increment);
static void EnablePWMOutputs(void);
static void DisablePWMOutputs(void);
static void ChangeDirection(void);

/*! \mainpage
* \section intro Introduction
* This documents data structures, functions, variables, defines, enums, and
* typedefs in the software for the VFD Sine Drive.
*
* \section compinfo Compilation Info
* \section deviceinfo Device Info
* ATmega328/ATmega88/ATmega168 can be used. The example is
* written for ATmega88.
*
* \section note Note
*
* Most of the source code in this application note is placed in one file (main.c)
* to be able to take full advantage of the compiler's optimization.
*
*/

#endif
