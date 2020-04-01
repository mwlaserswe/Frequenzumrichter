/*****************************************************************
**
** Description         : Servo Test Programm
**
** Compiler            : AVR-GCC 3.4.6 (WinAVR 20060421)
** Hardware            : AVR-Ctrl
** Chip type           : AT90S8535
** Clock frequency     : 8,000000 MHz
** Internal SRAM size  : 512
** External SRAM size  : 0
** Data Stack size     : 128
** Autor               : Michael Odenwald
** Date                : 04.06.2006
** Status              : Freeware
**
******************************************************************/

#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include <stdarg.h>
#include "lcd.h"

u08 servo_busy;

#define SCRATCH 16

/*
** sprintf
**
** Sehr einfachen sprintf.
**
*/
int sprintf(u08 *buf, const u08 *format, ...)
{
  u08 scratch[SCRATCH];
  u08 format_flag;
  u16 u_val=0, base;
  u08 *ptr;
  va_list ap;

  va_start (ap, format);
  for (;;){
    while ((format_flag = *format++) != '%'){      /* Until '%' or '\0' */
      if (!format_flag){va_end (ap); return (0);}
      *buf = format_flag; buf++; *buf=0;
    }

    switch (format_flag = *format++){

    case 'c':
      format_flag = va_arg(ap,int);
    default:
      *buf = format_flag; buf++; *buf=0;
      continue;
    case 'S':
    case 's':
      ptr = va_arg(ap,char *);
      strcat(buf, ptr);
      continue;
    case 'o':
      base = 8;
      *buf = '0'; buf++; *buf=0;
      goto CONVERSION_LOOP;
    case 'i':
      if (((int)u_val) < 0){
        u_val = - u_val;
        *buf = '-'; buf++; *buf=0;
      }
      /* no break -> run into next case */
    case 'u':
      base = 10;
      goto CONVERSION_LOOP;
    case 'x':
      base = 16;

    CONVERSION_LOOP:
      u_val = va_arg(ap,int);
      ptr = scratch + SCRATCH;
      *--ptr = 0;
      do {
        char ch = u_val % base + '0';
        if (ch > '9')
          ch += 'a' - '9' - 1;
        *--ptr = ch;
        u_val /= base;
      } while (u_val);
      strcat(buf, ptr);
      buf += strlen(ptr);
    }
  }
}

/*
** startservopuls
**
** Vorbereiten des Timer Compare Laufs und setzen des Ausgangs Pin
**
*/
void startservopuls(int time_ms)
{
  if(servo_busy)
     return;

  TCCR1B = 1;							// Timer einschalten
  OCR1AH = (time_ms>>8)&0x00FF;			// High Byte des Timer/Counter Output Compare Register laden
  OCR1AL = time_ms&0x00FF;				// Low Byte des Timer/Counter Output Compare Register laden
  TCNT1H = 0;							// High Byte des Timer/Counter Daten Register zu 0
  TCNT1L = 0;							// Low Byte des Timer/Counter Daten Register zu 0
  TIMSK |= (1<<OCIE1A);					// Timer/Counter Interrupt Mask -> Output Compare Match Interrupt Enable Timer/Counter einschalten
  PORTD |= (1<<5);						// Port D Bit 5 einschalten
  servo_busy=1;
}

/*
** ISR TIMER1_COMPA_vect
**
** Dies ist die Interrupt Service Routine wenn der Zähler den internen Zählwert für die Servo Puls Länge erreicht ist
**
*/
ISR(TIMER1_COMPA_vect)
{
	PORTD &= ~(1<<5);					// PortD Bit 5 ausschalten
	servo_busy = 0;						// Flag zurücksetzen
}

/*
** main
**
** Vorbereiten der LCD Ausgabe, Abfrage der Tasten, Vorbereiten der Servo Puls Erzeugung
**
*/
int main(void)
{
  u08 tmp;
  unsigned int time_ms = 12000;			// 1,5 ms neutral
  char text[16];						// Buffer

  lcd_init(LCD_DISP_ON);				// Initialisiere Display, Cursor off
  lcd_puts("+++ AVR Ctrl +++");			// Hello Text ausgeben

  DDRA = 0x00;							// alle Anschlüsse Port A auf input
  DDRD = 0xff;							// alle Anschlüsse Port D auf output
  PORTD = 0x00;							// alle Anschlüsse Port D auf null

  startservopuls(time_ms);				// Plus einschalten, Timer starten
  sei();								// Interrupt einschalten

  while(1)								// Start der main Schleife
  {
	tmp = PINA;
    switch(tmp)
    {
       case 0x80 : time_ms += 100;		// Taste für langsam nach Links
                   if (time_ms > 16000)
                     time_ms = 16000;
                   break;
       case 0x40 : time_ms = 16000;		// Taste Linker Anschlag
                   break;
       case 0x20 : time_ms = 12000;		// Taste Neutral
                   break;
       case 0x10 : time_ms = 8000;		// Taste Rechter Anschlag
                   break;
       case 0x08 : time_ms -= 100;
                   if (time_ms < 8000)	// Taste für langsam nach Rechts
                     time_ms = 8000;
                   break;
     }
     sprintf(text, "Tp us: %i", (time_ms - 8000));    // Ausgabe Buffer mit Variable füllen
     lcd_gotoxy(6,1);                                 // Cursor auf 0,1
     lcd_puts("     ");                               // Löschen des alten Inhalts
     lcd_gotoxy(0,1);                                 // Cursor auf 0,1
     lcd_puts(text);                                  // Text ausgeben

     startservopuls(time_ms);                         // Servopuls erzeugen
     _delay_ms(20);                                   // 20 Milisekunden Warten bis zum nächsten Durchlauf
  }
}

