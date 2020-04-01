/****************************************************************************
 Title	:   HD44780U LCD library
 Author:    Peter Fleury / Modifiziert Michael Odenwald
 Date:	    04.06.2006
 Software:  AVR-GCC 3.4.6 (WinAVR 20060421
 Usage:     See the C include lcd.h file for a description of each function

 			Dies ist eine radikal vereinfachte Version von Peter Fleury
 			HB44780 LCD Library. Sie ist nur für das AVR CTRL Board gedacht.
 			Alle nicht benötigten Statements sind entfernt worden, ebenso
 			der Memory Map Modus, was auf dem AVR CTRL Board nicht
 			vorgesehen ist. Die ganze Library ist nun mit dem neuen Syntax
 			des WinAVR versehen.
 			Die selbst gemachte Delay Routine durch die libavr delay Routine
 			ersetzt worden.

*****************************************************************************/
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "lcd.h"

/*
** constants/macros
*/
#define PIN(x) (*(&x - 2))  /* address of data direction register of port x */
#define DDR(x) (*(&x - 1))  /* address of input register of port x          */


#define SET_BIT(PORT, BITNUM)    ((PORT) |=  (1<<(BITNUM)))
#define CLEAR_BIT(PORT, BITNUM)  ((PORT) &= ~(1<<(BITNUM)))
#define TOOGLE_BIT(PORT, BITNUM) ((PORT) ^=  (1<<(BITNUM)))
#define lcd_e_delay() __asm__ __volatile__( "rjmp 1f\n 1:" );
#define lcd_e_high()    SET_BIT(LCD_E_PORT, LCD_E_PIN)
#define lcd_e_low()     CLEAR_BIT(LCD_E_PORT, LCD_E_PIN)
#define lcd_e_toggle()  toggle_e()

#if LCD_LINES==1
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_1LINE
#else
#define LCD_FUNCTION_DEFAULT    LCD_FUNCTION_4BIT_2LINES
#endif

static void toggle_e(void)
/* toggle Enable Pin */
{
    lcd_e_high();
    lcd_e_delay();
    lcd_e_low();
}


static void inline lcd_write(u08 data,u08 rs)
{
    /* configure data pins as output */
    DDR(LCD_DATA_PORT) = 0xFF;

    if (rs) {   /* write data        (RS=1, RW=0) */

       /* output high nibble first */
       LCD_DATA_PORT = (data&0xF0)|(1<<LCD_RS_PIN);
       lcd_e_toggle();

       /* output low nibble */
       LCD_DATA_PORT = (data<<4)|(1<<LCD_RS_PIN);
       lcd_e_toggle();

    } else {    /* write instruction (RS=0, RW=0) */

       /* output high nibble first */
       LCD_DATA_PORT = (data&0xF0);
       lcd_e_toggle();

       /* output low nibble */
       LCD_DATA_PORT = (data<<4);
       lcd_e_toggle();
    }

    /* all data pins high (inactive) */
    LCD_DATA_PORT = 0xF0;
}


static u08 lcd_read(u08 rs)
{
    register u08 dataH, dataL;

    if (rs) SET_BIT(LCD_RS_PORT, LCD_RS_PIN);    /* RS=1: read data      */
    else    CLEAR_BIT(LCD_RS_PORT, LCD_RS_PIN);    /* RS=0: read busy flag */
    SET_BIT(LCD_RW_PORT, LCD_RW_PIN);            /* RW=1  read mode      */

    /* configure data pins as input */
    DDR(LCD_DATA_PORT) = 0x0F;

    lcd_e_high();
    lcd_e_delay();
    dataH = PIN(LCD_DATA_PORT);         /* read high nibble first */
    lcd_e_low();

    lcd_e_delay();                      /* Enable 500ns low       */

    lcd_e_high();
    lcd_e_delay();
    dataL = PIN(LCD_DATA_PORT);         /* read low nibble        */
    lcd_e_low();

    return ( (dataH) | ((dataL>>4)&0x0F) );
}


static inline unsigned char lcd_waitbusy(void)
/* loops while lcd is busy, reads address counter */
{
    register unsigned char c;

    while ( (c=lcd_read(0)) & (1<<LCD_BUSY)) {}

    return (c);  // return address counter
}


static inline void lcd_newline(u08 pos)
/* goto start of next line */
{
    register u08 addressCounter;

#if LCD_LINES==1
    addressCounter = 0;
#endif
#if LCD_LINES==2
    if ( pos < (LCD_START_LINE2) )
        addressCounter = LCD_START_LINE2;
    else
        addressCounter = LCD_START_LINE1;
#endif
#if LCD_LINES==4
    if ( pos < LCD_START_LINE3 )
        addressCounter = LCD_START_LINE2;
    else if ( (pos >= LCD_START_LINE2) && (pos < LCD_START_LINE4) )
        addressCounter = LCD_START_LINE3;
    else if ( (pos >= LCD_START_LINE3) && (pos < LCD_START_LINE2) )
        addressCounter = LCD_START_LINE4;
    else
        addressCounter = LCD_START_LINE1;
#endif
    lcd_command((1<<LCD_DDRAM)+addressCounter);

}


/*
** PUBLIC FUNCTIONS
*/

/* send commando <cmd> to LCD */
void lcd_command(u08 cmd)
{
    lcd_waitbusy();
    lcd_write(cmd,0);
}

/* goto position (x,y) */
void lcd_gotoxy(u08 x, u08 y)
{
#if LCD_LINES==1
    lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
#endif
#if LCD_LINES==2
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
#endif
#if LCD_LINES==4
    if ( y==0 )
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE1+x);
    else if ( y==1)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE2+x);
    else if ( y==2)
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE3+x);
    else /* y==3 */
        lcd_command((1<<LCD_DDRAM)+LCD_START_LINE4+x);
#endif

}


int lcd_getxy(void)
{
    return lcd_waitbusy();
}

/* clear lcd and set cursor to home position */
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
}

/* set cursor to home position */
void lcd_home(void)
{
    lcd_command(1<<LCD_HOME);
}


/* print character at current cursor position */
void lcd_putc(char c)
{
    register unsigned char pos;

    pos = lcd_waitbusy();   // read busy-flag and address counter
    if (c=='\n')
        lcd_newline(pos);
    else
        lcd_write(c, 1);
}


/* print string on lcd (no auto linefeed) */
void lcd_puts(const char *s)
{
    register char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }

}


/* print string from program memory on lcd (no auto linefeed) */
void lcd_puts_p(const char *progmem_s)
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) ) {
        lcd_putc(c);
    }
}


/* initialize display and select type of cursor */
/* dispAttr: LCD_DISP_OFF, LCD_DISP_ON, LCD_DISP_ON_CURSOR, LCD_DISP_CURSOR_BLINK */
void lcd_init(u08 dispAttr)
{
    /*------ Initialize lcd to 4 bit i/o mode -------*/

    DDR(LCD_DATA_PORT) = 0xFF;         /* all port bits as output */

    _delay_ms(16);        /* wait 16ms or more after power-on       */

    /* initial write to lcd is 8bit */
    LCD_DATA_PORT = LCD_FUNCTION_8BIT_1LINE;
    lcd_e_toggle();
    _delay_ms(5);         /* delay, busy flag can't be checked here */

    LCD_DATA_PORT = LCD_FUNCTION_8BIT_1LINE;
    lcd_e_toggle();
    _delay_us(100);           /* delay, busy flag can't be checked here */

    LCD_DATA_PORT = LCD_FUNCTION_8BIT_1LINE;
    lcd_e_toggle();
    _delay_us(100);           /* delay, busy flag can't be checked here */

    LCD_DATA_PORT = LCD_FUNCTION_4BIT_1LINE; /* set IO mode to 4bit */
    lcd_e_toggle();

    /* from now the lcd only accepts 4 bit I/O, we can use lcd_command() */
    lcd_command(LCD_FUNCTION_DEFAULT);      /* function set: display lines  */
    lcd_command(LCD_DISP_OFF);              /* display off                  */
    lcd_clrscr();                           /* display clear                */
    lcd_command(LCD_MODE_DEFAULT);          /* set entry mode               */
    lcd_command(dispAttr);                  /* display/cursor control       */
}
