/****************************************************************************************
 * 
 *   Author      : Ariton Viorel
 *   Created     : 3/2/2021
 *   Description : Header file
 *   
 */

#ifndef AM2301__H__GUARD
#define AM2301__H__GUARD

  /*****************************************************************************************/
  /**********************************  Includes ********************************************/
  #include "stdint.h"
  
  /*****************************************************************************************/
  /******************************* Macros & Defines ****************************************/
  
  #define AMC2301_nOk             1
  #define AMC2301_nTimeoutError  -1
  #define AMC2301_nParityError   -2
  

  /*****************************************************************************************/
  /**************************** Public Function Declarations *******************************/
  
  /*
   * Function:  AM2301_i8Read 
   * --------------------
   * Returns the data read by the AM2301 sensor
   *    
   *
   *  In: the addresses of the variables the data to be placed in
   *
   *  returns: state of the read (See AM2301.h)
   *           
   */
  int8_t
  AM2301_i8Read(float *, float *);

#endif
/* AM2301.h End */
