/****************************************************************************************

     Author      : Ariton Viorel
     Created     : 3/2/2021
     Description : Temperature System

*/

/*****************************************************************************************/
/**********************************  Includes ********************************************/

extern "C" {
  #include "AM2301.h"
  #include "LCD_4Digits.h"
}

#include "stdint.h"

/*****************************************************************************************/
/******************************* Macros & Defines ****************************************/

#define MAIN__SREG   *(volatile uint8_t *)0x005F

/* 0 - 7 */
#define MAIN__PORTD  *(volatile uint8_t *)0x002B
#define MAIN__DDRD   *(volatile uint8_t *)0x002A

/* 8 - 13 */
#define MAIN__DDRB   *(volatile uint8_t *)0x0024
#define MAIN__PORTB  *(volatile uint8_t *)0x0025

/* A0 - A5 */
#define MAIN__DDRC   *(volatile uint8_t *)0x0027
#define MAIN__PORTC  *(volatile uint8_t *)0x0028
#define MAIN__PINC   *(volatile uint8_t *)0x0026


/* 32 overflows = ~500ms */
#define MAIN__TemperatureChangeDebounceTime   0x20u
#define MAIN__TemperatureIncreaseDebounceTime 0x10u
#define MAIN__DecreaseTemperatureDebounceTime 0x10u

#define MAIN__IncreaseTemperatureButton (MAIN__PINC & 0x01u)
#define MAIN__DecreaseTemperatureButton (MAIN__PINC & 0x02u)
#define MAIN__ChangeStateButton         (MAIN__PINC & 0x04u)

/* PORTB */
#define MAIN__CelsiusPin (1u << 5)

/*
   ( (4 * 0xFFFE) + 5) * 1/16e = ~16.38ms
*/
#define MAIN__ASM(L) #L
#define MAIN__BlockingDelay16msRel(L) {     \
    __asm__ __volatile__(                   \
      "ldi r25, hi8("MAIN__ASM(L)")" "\n\t" \
      "ldi r24, lo8("MAIN__ASM(L)")" "\n\t" \
      "1:" "\n\t"                           \
      "sbiw r24, 1" "\n\t"                  \
      "brne 1b"                             \
       ::                                   \
     );                                     \
  }

#define MAIN__vDebounce(Button, A, Time, Callback)                                     \
  TIMSK2 = 0x00;                                                                       \
  if (Button == 0) {                                                                   \
    if(MAIN__u8Pressed[(A/2)] != 1u){                                                  \
      if (MAIN__u8Debounce[A + 1] == 0) {                                              \
        MAIN__u8Debounce[A + 1] = MAIN__u8Debounce[A];                                 \
      } else if (MAIN__u8Debounce[A + 1] > MAIN__u8Debounce[A]) {                      \
        if ((((uint8_t)(-1) - MAIN__u8Debounce[A + 1]) + MAIN__u8Debounce[A]) >= Time) \
          MAIN__u8Pressed[(A/2)] = 1u;                                                 \
      } else {                                                                         \
        if ((MAIN__u8Debounce[A] - MAIN__u8Debounce[A + 1]) >= Time)                   \
          MAIN__u8Pressed[(A/2)] = 1u;                                                 \
      }                                                                                \
    }                                                                                  \
  } else {                                                                             \
    if (MAIN__u8Pressed[(A/2)] == 1u) {                                                \
      Callback                                                                         \
    }                                                                                  \                                                                                                                
     MAIN__u8Debounce[A + 1] = 0;                                                      \
     MAIN__u8Pressed[(A/2)] = 0;                                                       \                                     
  }                                                                                    \
  TIMSK2 = (1 << TOIE2);                                                               \



  
/*****************************************************************************************/
/********************************* Type Definitions **************************************/


/**
   The state the LCD is in
*/
typedef enum {

  MAIN__nSetState     = 0x01u,

  MAIN__nDisplay      = 0x02u

} MAIN__tenDisplayState;


/*****************************************************************************************/
/******************************* Global Variables ****************************************/

static float                 MAIN__fTemperature;
static uint8_t               MAIN__u8SetTemperature      = 23;
static uint8_t               MAIN__u8TemperatureState;

static volatile uint8_t      MAIN__u8Debounce[6];
static uint8_t               MAIN__u8Pressed[3];

static MAIN__tenDisplayState MAIN__enDisplayState = MAIN__nDisplay;

static volatile uint8_t      MAIN__u8Delay2s;


/*****************************************************************************************/
/*********************************** Interrupts ******************************************/


/*
    8 bit timer - 1024 prescaler
    (2^8 * (1024/16e6))  = ~17ms overflow
    2000/~17ms           = ~122 overflows for 2s
*/
ISR(TIMER2_OVF_vect) {
  
  ++MAIN__u8Delay2s;
  ++MAIN__u8Debounce[0];
  ++MAIN__u8Debounce[2];
  ++MAIN__u8Debounce[4];
  
}

/*****************************************************************************************/
/**************************** Public Function Declarations *******************************/

void
setup(void) {

  /* Output pins */
  MAIN__DDRB = 0b00111110;
  MAIN__DDRD = 0b11111100;

  /* Use analog A0, A1 and A3 as digital */
  MAIN__PORTC = 0x07;

  /* SREG - Disable interrupts */
  MAIN__SREG &= ~(1u << 7);

  TCCR2A = 0x00;
  TCCR2B = 0x00;

  /* 1024 prescaler */
  TCCR2B = ((1 << CS22) | (1 << CS21) | (1 << CS20));
  /* Normal mode */
  TCCR2A = 0x00;
  /* Overflow interrupt */
  TIMSK2 = (1 << TOIE2);

  TCNT2  = 0;

  /* Enable interrupts */
  MAIN__SREG |= (1u << 7);
  
}

void
loop(void) {

  MAIN__vDebounce(MAIN__ChangeStateButton, 0, MAIN__TemperatureChangeDebounceTime, {  
    MAIN__enDisplayState = (MAIN__tenDisplayState)((uint8_t)MAIN__enDisplayState ^ 3u); 
  });

  switch (MAIN__enDisplayState) {


    /* Temperature set state */
    case MAIN__nSetState:
      
      MAIN__vDebounce(MAIN__IncreaseTemperatureButton, 2, MAIN__TemperatureIncreaseDebounceTime, {  
        ++MAIN__u8SetTemperature; 
      });
      MAIN__vDebounce(MAIN__DecreaseTemperatureButton, 4, MAIN__DecreaseTemperatureDebounceTime, {
        --MAIN__u8SetTemperature;
      });

    LCD_SEG__vDisplayNumber(MAIN__u8SetTemperature);

      break;

    /* Display state */
    case MAIN__nDisplay:
      {
        int8_t i8State;
        float  fHumidity;


        /* Disable overflow interrupt */
        TIMSK2 = 0x00;

        if (MAIN__u8Delay2s >= 122u) {

          (void)AM2301_i8Read(&MAIN__fTemperature, &fHumidity);


          /* Reset the timer */
          MAIN__u8Delay2s = 0;
        }

        /* Enable overflow interrupt */
        TIMSK2 = (1 << TOIE2);


        /* Display the dot */
        switch (MAIN__u8TemperatureState) {
          case 0x03u:
            {
              /* Reset the display */
              MAIN__PORTD = MAIN__DDRD;
              MAIN__PORTB = MAIN__DDRB;

              /* Activate the third digit */
              MAIN__PORTB &= ~MAIN__CelsiusPin;

              /* Display the dot */
              MAIN__PORTD = ~LCD_SEG__nDot;

              ++MAIN__u8TemperatureState;
            }

            break;

          case 0x04u:

            /* Display the C character */
            MAIN__PORTD &= ~LCD_SEG__nTop;
            MAIN__PORTD &= ~LCD_SEG__nTopLeft;

            MAIN__PORTB &= ~(LCD_SEG__nBottomLeft >> 8);
            MAIN__PORTB &= ~(LCD_SEG__nBottom >> 8);

            /* Reset to display the digits */
            MAIN__u8TemperatureState = 0;
            break;

          default:

            /* Display the temperature */
            LCD_SEG__vDisplayNumber((uint8_t)MAIN__fTemperature);

            ++MAIN__u8TemperatureState;
            break;
        }
      }

      break;

  }

  /* 2ms, increase the duty cycle */
  MAIN__BlockingDelay16msRel(0x1F40);

}/* main.c End */
