/****************************************************************************************
 * 
 *   Author      : Ariton Viorel
 *   Created     : 2/2/2021
 *   Description : Header file
 *   
 */

/*****************************************************************************************/
/**********************************  Includes ********************************************/
#include "InputCapture.h"
#include "Arduino.h"

/*****************************************************************************************/
/******************************* Macros & Defines ****************************************/

#define ICP__ChangeEdgeDetection() { \
                                      \
  TCCR1B ^= (1u << ICES1);           \
                                     \
  TIFR1 |= (1u << ICF1);             \
                                     \
}        
/*****************************************************************************************/
/********************************* Type Definitions **************************************/

typedef union {
  
 uint16_t u16Ticks;
 
 struct {
   
    uint8_t u8Low;
   
    uint8_t u8High;
    
 }__byte;
  
}ICP__tstTimeStamp;

typedef void(*fpCallback)(uint32_t);

/*****************************************************************************************/
/******************************* Global Variables ****************************************/

static void                       *ICP__vpHighTimeCallback = NULL;
static void                       *ICP__vpLowTimeCallback  = NULL;
static void                       *ICP__vpPeriodCallback   = NULL;


static volatile ICP_tenPulseLevel  ICP__enCurrentPulse;
static volatile ICP_tenPulseLevel  ICP__enPreviousPulse = ICP_nenInvalid;
static volatile ICP_tenPulseLevel  ICP__enPeriodPulse;

static volatile uint16_t           ICP__u16OverflowCounter;

static volatile ICP__tstTimeStamp  ICP__stCapture;

static volatile uint32_t           ICP__u32CurrentCapture;
static volatile uint32_t           ICP__u32PreviousCapture;

static volatile ICP__tstTimeStamp  ICP__stPeriodCapture;


/*****************************************************************************************/
/**************************** Public Function Definitions ********************************/

void 
ICP_vSetup(ICP_tenPulseLevel enCurrentPulse) {

  ICP__enCurrentPulse = enCurrentPulse;

  /* Disable interrupts */
  noInterrupts();
  
  TCCR1A = 0;
  
  /* No prescaler, falling edge capture */
  TCCR1B = (1u << CS10);
  
  TCCR1C = 0;
  
  /* Enable overflow and ICP interrupt */
  TIMSK1 = ((1u << ICIE1) | (1u << TOIE1));
  TIFR1  = ((1u << ICF1) | (1u << TOV1)); 
  
  ICP__u16OverflowCounter  = 0;
  
  ICP__u32PreviousCapture  = 0;
  ICP__u32CurrentCapture   = 0;

  TCNT1                    = 0;
  /* Enable interrupts */
  interrupts();
  
}

void
ICP_vRestart(void) {
  
    
}

void
ICP_vStop(void) {
  TIMSK1 = 0;
}

void
ICP_vRegisterCallback(void *vpCallback, ICP_tenCallbackType enCallbackType) {
  
  if(vpCallback == NULL)
    return;
  
  switch(enCallbackType) {
    
    case ICP_nenPeriodCallback:
      ICP__vpPeriodCallback = vpCallback;
    break;
    
    case ICP_nenLowTimeCallback:
      ICP__vpLowTimeCallback = vpCallback;
    break;
    
    case ICP_nenHighTimeCallback:
      ICP__vpHighTimeCallback = vpCallback;
    break;
  }
}

ISR(TIMER1_OVF_vect) {

  ++ICP__u16OverflowCounter;
  TCNT1  = 0;
}

ISR(TIMER1_CAPT_vect) {


  /*
   * C1 - Current
   * C2 - Previous
   *   
   * |---|_____|----|___
   *     FE   RE    FE
   * 
   * F1(1) = Capture time stamp and store it C2 = C1
   * RE    = Capture timp stamp and call [Low Time] callback with (C1 - C2) and signal for a reset
   * FE(2) = Capture timp stamp--the ICP register was reset at RE so It starts counting from 0-- and call [High Time] callback, store C2 = C1
   */
  uint32_t u32Overflow;
  
  u32Overflow                  = ICP__u16OverflowCounter;
  /* Capture the ticks */
  ICP__stCapture.__byte.u8Low  = ICR1L;
  ICP__stCapture.__byte.u8High = ICR1H;
  
  /* Check for missed overflow */
  if((TIFR1 & bit(TOV1)) && (ICP__stCapture.u16Ticks < 0x7FFFu))
      ++u32Overflow;
      
  ICP__u32CurrentCapture = (uint32_t)((uint32_t)(u32Overflow << 16u) + ICP__stCapture.u16Ticks); 
  
  /* Check if It's the first capture */
  if(ICP__enPreviousPulse != ICP_nenInvalid) {
  
    switch(ICP__enCurrentPulse) {
      
      case ICP_nenLow:

        if(ICP__u32CurrentCapture >= ICP__u32PreviousCapture) {
          ((fpCallback)ICP__vpHighTimeCallback)(ICP__u32CurrentCapture - ICP__u32PreviousCapture);
        } else {
          ((fpCallback)ICP__vpHighTimeCallback)(((uint32_t)(-1) - ICP__u32PreviousCapture) + ICP__u32CurrentCapture);
        }
       
      
        break;

      case ICP_nenHigh:

        /* Call the low time callback */
        if(ICP__u32CurrentCapture >= ICP__u32PreviousCapture) {
          ((fpCallback)ICP__vpLowTimeCallback)(ICP__u32CurrentCapture - ICP__u32PreviousCapture);
        } else {
          ((fpCallback)ICP__vpLowTimeCallback)(((uint32_t)(-1) - ICP__u32PreviousCapture) + ICP__u32CurrentCapture);
        }

        /* Successful read - reset the counters */
        break;
    }
  
  }

  /* Save the current time stamp */
  ICP__u32PreviousCapture = ICP__u32CurrentCapture;
  
  ICP__enPreviousPulse    = ICP__enCurrentPulse;

  ICP__enCurrentPulse     = (ICP_tenPulseLevel)(ICP__enCurrentPulse ^ 0x00000003);
  
  /* Change the edge */
  ICP__ChangeEdgeDetection();  
  
} /* InputCapture.cpp End */
