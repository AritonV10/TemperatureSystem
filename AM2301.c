/****************************************************************************************
 * 
 *   Author      : Ariton Viorel
 *   Created     : 3/2/2021
 *   Description : Implementation file
 *   
 */

/*****************************************************************************************/
/********************************** Includes *********************************************/
#include "AM2301.h"
#include "InputCapture.h"


/*****************************************************************************************/
/*********************************** Config **********************************************/

/*
   Temperature - 2 byte
   Humidity    - 2 byte
   Checksum    - 1 byte
*/
#define AM2301__nNFrameBits (40)

/*
   Unit: ms [miliseconds]
*/
#define AM2301__nRequestDataTime (0x1F3F) /* 2 ms */

#define AM2301__nRequestDataPin  PORTB, 0x09


/*****************************************************************************************/
/******************************* Macros & Defines ****************************************/


/*
 * ( (4 * 0xFFFE) + 5) * 1/16e = ~16.38ms
 */
#define AM2301__ASM(L) #L
#define AM2301__BlockingDelay16msRel(L) {     \
    __asm__ __volatile__(                     \ 
      "ldi r25, hi8("AM2301__ASM(L)")" "\n\t" \
      "ldi r24, lo8("AM2301__ASM(L)")" "\n\t" \
      "1:" "\n\t"                             \
      "sbiw r24, 1" "\n\t"                    \
      "brne 1b"                               \
      ::                                      \                      
    );                                        \ 
}


#define AM2301__vRequestData(){                             \
    AM2301__DDRB |= 0x01;                                   \
    AM2301__vSetPinLow(AM2301__PORTB, 0x09);                \ 
    AM2301__BlockingDelay16msRel(AM2301__nRequestDataTime); \
    AM2301__vSetPinHigh(AM2301__PORTB, 0x09);               \
    AM2301__DDRB &= 0xFE;                                   \
  }



#define AM2301__n16BitsMask       (0x000000000000FFFF)
#define AM2301__n8BitsMask        (0x00000000000000FF)

/*
 * Pin 8 (ICP)
 */
#define AM2301__DDRB  *(volatile uint8_t *)0x0024
#define AM2301__PORTB *(volatile uint8_t *)0x0025 


#define True  (1u)
#define False !True

#define AM2301__vSetBitValue(Value, Position, Variable) (Variable |= (Value << (Position - 1)))
#define AM2301__vSetBit(Position, Variable)             (Variable |= (1u << (Position - 1)))
#define AM2301__vToggleBit(Position, Variable)          (Variable ^= (1u << (Position - 1)))
#define AM2301__vUnsetBit(Position, Variable)           (Variable &= ~(1u << (Position - 1)))

#define AM2301__vSetPinHigh(Port, PIN) (AM2301__vSetBit(PIN, Port))
#define AM2301__vSetPinLow(Port, PIN)  (AM2301__vUnsetBit(PIN, Port))


#define AM2301__u8Conversion(L)   ( ((2 << 7) * (L >> 8)) + ((2 << 3) * ((L & 0x00F0u) >> 4)) + (L & 0x000F) )
#define AM2301__u16Humidity(L)    (uint16_t)((L >> 24) & AM2301__n16BitsMask)
#define AM2301__u16Temperature(L) (uint16_t)((L >> 8) & AM2301__n16BitsMask)
#define AM2301__u8Parity(L)       (uint8_t)(L & AM2301__n8BitsMask)
#define AM2301__vData(L, C, R)    (L = L | ((uint64_t)C << ((AM2301__nNFrameBits - 1) - R)))
#define AM2301__u8ParitySum(L, C) (uint8_t)(((L) & AM2301__n8BitsMask) + (((L) >> 8) & AM2301__n8BitsMask) + ((C) & AM2301__n8BitsMask) + (((C) >> 8) & AM2301__n8BitsMask))


/*****************************************************************************************/
/******************************* Global Variables ****************************************/

/*
 * Nr. of bits received
 */
static volatile int8_t   AM2301__i8NBits;
/*
 * Signals if all the bits were received
 */
static volatile uint8_t  AM2301__u8BitsReceivedFlag;
/*
 * Acknowledge bits
 */
static volatile uint8_t  AM2301__u8ResponseSignal;
static volatile uint64_t AM2301__u64DataFrame;


/*****************************************************************************************/
/**************************** Private Function Declarations *******************************/


static int8_t
AM2301__i8Read(void);


/*****************************************************************************************/
/**************************** Public Function Definitions ********************************/

int8_t
AM2301_i8Read(float *pfTemperature, float *pfHumidity) {

  uint16_t u16Temp;
  int8_t   i8State;

  /* Get the frame */
  i8State = AM2301__i8Read();

  if (i8State == AMC2301_nTimeoutError)
    return (AMC2301_nTimeoutError);


  /* Check the parity bits */
  if (AM2301__u8ParitySum(AM2301__u16Temperature(AM2301__u64DataFrame), AM2301__u16Humidity(AM2301__u64DataFrame))  == AM2301__u8Parity(AM2301__u64DataFrame)) {
    ;
  } else {
    AM2301__u64DataFrame >>= 1;
    if (AM2301__u8ParitySum(AM2301__u16Temperature(AM2301__u64DataFrame), AM2301__u16Humidity(AM2301__u64DataFrame)) != AM2301__u8Parity(AM2301__u64DataFrame))
      return (AMC2301_nParityError);
  }

  /* Debug: data gets shifted left by 1 bit */
  u16Temp = AM2301__u8Conversion((AM2301__u16Temperature(AM2301__u64DataFrame) & 0x7FFFu));

  if(u16Temp >= 500)
    u16Temp >>= 1;
    
  *pfTemperature = u16Temp / 10.0;
  
  
  *pfHumidity    = AM2301__u8Conversion(AM2301__u16Humidity(AM2301__u64DataFrame)) / 10.0;

  return (AMC2301_nOk);

}


/*****************************************************************************************/
/**************************** Private Function Definitions *******************************/

void
AM2301__vLowTimeCallback(uint32_t u32Ticks) {
  
  if(AM2301__u8ResponseSignal <= 2u)
    ++AM2301__u8ResponseSignal;
}

void
AM2301__vHighTimeCallback(uint32_t u32Ticks) {

  /* Response signal */

  if (AM2301__u8BitsReceivedFlag != True )
    if (AM2301__u8ResponseSignal >= 3) {

      /* To avoid noise */
      if (AM2301__u8BitsReceivedFlag != True ) {

        if (AM2301__i8NBits >= 0) {



          /* 820 ticks = ~53us = .05 * 1/6e3 */
          AM2301__vData(AM2301__u64DataFrame, (u32Ticks >= 1000u), AM2301__i8NBits);

          if (AM2301__i8NBits >= (AM2301__nNFrameBits - 1)) {

            AM2301__u8BitsReceivedFlag = True;

            return;
          }

        }

        ++AM2301__i8NBits;
      }
    }

}


static int8_t
AM2301__i8Read(void) {


  /* Setup the ICP for a single shot read */
  ICP_vRegisterCallback((void *)&AM2301__vLowTimeCallback, ICP_nenLowTimeCallback);
  ICP_vRegisterCallback((void *)&AM2301__vHighTimeCallback, ICP_nenHighTimeCallback);
  ICP_vSetup(ICP_nenLow);

  /* Reset data */
  AM2301__u64DataFrame       = 0;
  AM2301__i8NBits            = 0;
  AM2301__u8ResponseSignal   = 0;
  AM2301__u8BitsReceivedFlag = False;

  /* Keep the line low for 2ms */
  AM2301__vRequestData();

  /* Wait ~2.4ms since it uses our 2ms request time for the response signals */
  AM2301__BlockingDelay16msRel(0x257F);

  /* Check to see if we received the signals */
  if(AM2301__u8ResponseSignal != 3u)
    return(AMC2301_nTimeoutError);
   
  /* Wait until the data has been received */
  for (; AM2301__u8BitsReceivedFlag != 1u ;)
    ;

  ICP_vStop();

  return(AMC2301_nOk);

} /* AM2301.c End */
