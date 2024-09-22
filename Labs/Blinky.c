/*----------------------------------------------------------------------------
 * Name:    Blinky.c
 * Purpose: LED Flasher
 * Note(s): __USE_LCD   - enable Output on LCD, uncomment #define in code to use
 *  				for demo (NOT for analysis purposes)
 *----------------------------------------------------------------------------
 * Copyright (c) 2008-2011 Keil - An ARM Company.
 * Name: Anita Tino
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include "Blinky.h"
#include "LPC17xx.h"                       
#include "GLCD.h"
#include "LED.h"
#include "Board_ADC.h" 
#include "KBD.h"

#define __FI        1                      /* Font index 16x24               */
#define __USE_LCD   0										/* Uncomment to use the LCD */

//ITM Stimulus Port definitions for printf //////////////////
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))

#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000



struct __FILE { int handle;  };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
  if (DEMCR & TRCENA) {
    while (ITM_Port32(0) == 0);
    ITM_Port8(0) = ch;
  }
  return(ch);
}
/////////////////////////////////////////////////////////

char text[10];
char text_l[10];

static volatile uint16_t AD_dbg;

uint16_t ADC_last;                      // Last converted value
/* Import external variables from IRQ.c file                                  */
extern uint8_t  clock_ms;

// Variables to store the joystick direction and LED state
char direction_text[20];
uint32_t led_state = 0;

// Function to update the LCD with joystick direction
void display_direction(uint32_t direction) {
    GLCD_SetBackColor(White);
    GLCD_SetTextColor(Blue);
    GLCD_DisplayString(4, 0, __FI, (unsigned char *)"Joystick Direction:");

    switch (direction) {
        case KBD_UP:
            sprintf(direction_text, "UP   ");
            break;
        case KBD_DOWN:
            sprintf(direction_text, "DOWN ");
            break;
        case KBD_LEFT:
            sprintf(direction_text, "LEFT ");
            break;
        case KBD_RIGHT:
            sprintf(direction_text, "RIGHT");
            break;
        case KBD_SELECT:
            sprintf(direction_text, "SELECT");
            break;
        default:
            sprintf(direction_text, "NONE ");
            break;
    }

    // Display direction on the LCD
    GLCD_DisplayString(6, 0, __FI, (unsigned char *)direction_text);
}

// Function to control LEDs based on joystick direction
void control_LEDs_based_on_joystick(uint32_t joystick_value) {
  // Turn off all LEDs first
  LPC_GPIO1->FIOCLR = (1 << 28);    // Clear LED for Up
  LPC_GPIO1->FIOCLR = (1 << 29);    // Clear LED for Down
  LPC_GPIO1->FIOCLR = (1 << 31);    // Clear LED for Left
  LPC_GPIO2->FIOCLR = (1 << 2);     // Clear LED for Right
  LPC_GPIO2->FIOCLR = (1 << 3);     // Clear LED for Select

  // Turn on the corresponding LED based on joystick input
  if (joystick_value & KBD_UP) {
    LPC_GPIO1->FIOSET = (1 << 28);  // Turn on LED for Up
  }
  if (joystick_value & KBD_DOWN) {
    LPC_GPIO1->FIOSET = (1 << 29);  // Turn on LED for Down
  }
  if (joystick_value & KBD_LEFT) {
    LPC_GPIO1->FIOSET = (1 << 31);  // Turn on LED for Left
  }
  if (joystick_value & KBD_RIGHT) {
    LPC_GPIO2->FIOSET = (1 << 2);   // Turn on LED for Right
  }
  if (joystick_value & KBD_SELECT) {
    LPC_GPIO2->FIOSET = (1 << 3);   // Turn on LED for Select
  }
}


/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
  int32_t  res;
  uint32_t AD_sum   = 0U;
  uint32_t AD_cnt   = 0U;
  uint32_t AD_value = 0U;
  uint32_t AD_print = 0U;
	uint32_t Joystick_value; 
	
	//Peripheral Init 

  LED_Init();                                /* LED Initialization            */
  KBD_Init(); 
	ADC_Initialize();                                /* ADC Initialization            */

#ifdef __USE_LCD
  GLCD_Init();                               /* Initialize graphical LCD (if enabled */

  GLCD_Clear(White);                         /* Clear graphical LCD display   */
  GLCD_SetBackColor(Blue);
  GLCD_SetTextColor(Yellow);
  GLCD_DisplayString(0, 0, __FI, "     COE718 Demo    ");
	GLCD_SetTextColor(White);
  GLCD_DisplayString(1, 0, __FI, "   Joystick Control   ");
 
  GLCD_SetBackColor(White);
  GLCD_SetTextColor(Blue);

#endif

  //SystemCoreClockUpdate();
  //SysTick_Config(SystemCoreClock/100);       /* Generate interrupt each 10 ms */

  while (1) {                                /* Loop forever                  */
		uint32_t Joystick_value = get_button(); 
		display_direction(Joystick_value);
		control_LEDs_based_on_joystick(Joystick_value);
    /* AD converter input                                                     */
    // AD converter input
    res = ADC_GetValue();
    if (res != -1) {                     // If conversion has finished
      ADC_last = (uint16_t)res;
      
      AD_sum += ADC_last;                // Add AD value to sum
      if (++AD_cnt == 16U) {             // average over 16 values
        AD_cnt = 0U;
        AD_value = AD_sum >> 4;          // average devided by 16
        AD_sum = 0U;
      }
    }

    /*if (AD_value != AD_print) {
      AD_print = AD_value;               // Get unscaled value for printout
      AD_dbg   = (uint16_t)AD_value;

      sprintf(text, "0x%04X", AD_value); // format text for print out */
    
//LCD Bargraph Functionality			

/*#ifdef __USE_LCD
      GLCD_SetTextColor(Red);
      GLCD_DisplayString(6,  9, __FI,  (unsigned char *)text);
			GLCD_SetTextColor(Green);
      GLCD_Bargraph (144, 7*24, 176, 20, (AD_value >> 2)); max bargraph is 10 bit 
#endif */
    }

    /* Print message with AD value every 10 ms                               */
    if (clock_ms) {
      clock_ms = 0;

      printf("AD value: %s\r\n", text);
    }
	
}
