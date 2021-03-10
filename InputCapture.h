/****************************************************************************************
 * 
 *   Author      : Ariton Viorel
 *   Created     : 2/2/2021
 *   Description : Header file
 *   
 */
 
#ifndef ICP__H__GUARD
#define ICP__H__GUARD

  /*****************************************************************************************/
  /**********************************  Includes ********************************************/
  #include "stdint.h"
  
  /*****************************************************************************************/
  /********************************* Type Definitions **************************************/

  typedef enum {
    
   ICP_nenInvalid = 0,
    
   ICP_nenHigh    = 1,
    
   ICP_nenLow     = 2
    
  }ICP_tenPulseLevel;
  
  typedef enum{
    
    ICP_nenHighTimeCallback,
    
    ICP_nenLowTimeCallback,
    
    ICP_nenPeriodCallback
    
  }ICP_tenCallbackType;
  
  /*****************************************************************************************/
  /**************************** Public Function Declarations *******************************/

   /**
   * Function:  ICP_vRegisterCallback 
   * --------------------
   * Registers a callback for ICP
   *    
   *
   *  In: address of the function
   *      callback type (See InputCapture.h)
   *
   *  returns: void
   *           
   */
  extern void
  ICP_vRegisterCallback(void *, ICP_tenCallbackType);

  /**
   * Function:  ICP_vSetup 
   * --------------------
   * Configures the ICP
   *    
   *
   *  In: the first edge to be detected
   *
   *  returns: void
   *           
   */
  extern void
  ICP_vSetup(ICP_tenPulseLevel);

  /**
   * Function:  ICP_vRestart 
   * --------------------
   * Restarts the reading
   *    
   *
   *  In: void
   *
   *  returns: void
   *           
   */
  extern void
  ICP_vRestart(void);

  /**
   * Function:  ICP_vStop 
   * --------------------
   * Stops the reading
   *    
   *
   *  In: void
   *
   *  returns: void
   *           
   */
  extern void
  ICP_vStop(void);
  
#endif
/* InputCapture.h End */
