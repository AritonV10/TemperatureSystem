/****************************************************************************************

     Author      : Ariton Viorel
     Created     : 3/2/2021
     Description : Implementation file

*/

/*****************************************************************************************/
/**********************************  Includes ********************************************/

#include "LCD_4Digits.h"


/*****************************************************************************************/
/******************************* Macros & Defines ****************************************/

#define LCD_SEG__PORTD  *(volatile uint8_t *)0x002B
#define LCD_SEG__DDRD   *(volatile uint8_t *)0x002A

#define LCD_SEG__DDRB   *(volatile uint8_t *)0x0024
#define LCD_SEG__PORTB  *(volatile uint8_t *)0x0025


#define LCD_SEG__SegmentsOFF {          \
    LCD_SEG__PORTD = LCD_SEG__DDRD;     \
    LCD_SEG__PORTB = LCD_SEG__DDRB;     \
}


#define LCD_SEG__vDisplayDigit(N) {                                      \
    LCD_SEG__PORTD ^= (LCD_SEG__au16Digit[N] & 0x00FF);                  \
    LCD_SEG__PORTB ^= (LCD_SEG__au16Digit[N] >> 8);                      \
}

#define LCD_SEG__vActivateDigit(L) {                                   \
    uint16_t u16Dummy;                                                 \
    u16Dummy = ( ( (LCD_SEG__PORTB << 8) | LCD_SEG__PORTD) ^ (L) );    \
    LCD_SEG__PORTD = u16Dummy;                                         \
    LCD_SEG__PORTB = (u16Dummy >> 8);                                  \
}



/*****************************************************************************************/
/******************************* Global Variables ****************************************/

static const uint16_t LCD_SEG__au16Digit[] = {
  (LCD_SEG__nTop      | LCD_SEG__nTopLeft  | LCD_SEG__nTopRight | LCD_SEG__nBottom      | LCD_SEG__nBottomLeft | LCD_SEG__nBottomRight),                    /* 0 */
  (LCD_SEG__nTopRight | LCD_SEG__nBottomRight),                                                                                                             /* 1 */
  (LCD_SEG__nTop      | LCD_SEG__nTopRight | LCD_SEG__nMiddle   | LCD_SEG__nBottomLeft  | LCD_SEG__nBottom),                                                /* 2 */
  (LCD_SEG__nTop      | LCD_SEG__nTopRight | LCD_SEG__nMiddle   | LCD_SEG__nBottomRight | LCD_SEG__nBottom),                                                /* 3 */
  (LCD_SEG__nTopLeft  | LCD_SEG__nMiddle   | LCD_SEG__nTopRight | LCD_SEG__nBottomRight),                                                                   /* 4 */
  (LCD_SEG__nTop      | LCD_SEG__nTopLeft  |  LCD_SEG__nMiddle  | LCD_SEG__nBottom      | LCD_SEG__nBottomRight),                                           /* 5 */
  (LCD_SEG__nTop      | LCD_SEG__nTopLeft  |  LCD_SEG__nMiddle  | LCD_SEG__nBottom      | LCD_SEG__nBottomLeft | LCD_SEG__nBottomRight),                    /* 6 */
  (LCD_SEG__nTop      | LCD_SEG__nTopRight |  LCD_SEG__nBottomRight),                                                                                       /* 7 */
  (LCD_SEG__nTop      | LCD_SEG__nTopLeft  | LCD_SEG__nTopRight | LCD_SEG__nBottom      | LCD_SEG__nBottomLeft | LCD_SEG__nBottomRight | LCD_SEG__nMiddle), /* 8 */
  (LCD_SEG__nTop      | LCD_SEG__nTopLeft  | LCD_SEG__nTopRight | LCD_SEG__nBottom      | LCD_SEG__nBottomRight | LCD_SEG__nMiddle),                        /* 9 */
};

static const uint16_t LCD_SEG__au8DigitsOrder[] = LCD_SEG__DigitsOrder;
static const uint16_t LCD_SEG__au16PowerTen[]   = {1, 10, 100, 1000, 1000};
static int8_t         LCD_SEG__i8State          = (int8_t)(sizeof(LCD_SEG__au8DigitsOrder) / 2 - 1);

/*****************************************************************************************/
/**************************** Public Function Definitions ********************************/

void
LCD_SEG__vDisplayNumber(uint8_t u8Number) {

  /* Clear the display */
  LCD_SEG__SegmentsOFF;

  LCD_SEG__vActivateDigit(LCD_SEG__au8DigitsOrder[LCD_SEG__i8State]);

  LCD_SEG__vDisplayDigit((u8Number / LCD_SEG__au16PowerTen[LCD_SEG__i8State]) % 10);

  --LCD_SEG__i8State;

  if (LCD_SEG__i8State == -1)
    LCD_SEG__i8State = (sizeof(LCD_SEG__au8DigitsOrder) / 2 - 1);
}
