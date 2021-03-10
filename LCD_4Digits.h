/****************************************************************************************
 * 
 *   Author      : Ariton Viorel
 *   Created     : 3/2/2021
 *   Description : Implementation file
 *   
 */


#ifndef LCD__H__GUARD
#define LCD__H__GUARD

  
  /*****************************************************************************************/
  /**********************************  Includes ********************************************/
  #include "stdint.h"
  
  
  /*****************************************************************************************/
  /*********************************** Config **********************************************/
  
  #define LCD_SEG__vPin(C) ((uint16_t)1u << (C))
  #define LCD_SEG__DigitsOrder {LCD_SEG__vPin(0x000Cu), LCD_SEG__vPin(0x000Bu)}
  
  
  enum {
  
    LCD_SEG__nDot         = LCD_SEG__vPin(0x02u),
    LCD_SEG__nMiddle      = LCD_SEG__vPin(0x03u),
    LCD_SEG__nBottomRight = LCD_SEG__vPin(0x04u),
    LCD_SEG__nTopRight    = LCD_SEG__vPin(0x05u),
    LCD_SEG__nTop         = LCD_SEG__vPin(0x06u),
    LCD_SEG__nTopLeft     = LCD_SEG__vPin(0x07u),
    LCD_SEG__nBottomLeft  = LCD_SEG__vPin(0x09u),
    LCD_SEG__nBottom      = LCD_SEG__vPin(0x0Au)
  };
  
  /*****************************************************************************************/
  /**************************** Public Function Declarations *******************************/

  /**
   * Function:  LCD_SEG__vDisplayNumber 
   * --------------------
   * Displays a number on the LCD 4-digit segment
   *    
   *
   *  In: the number to be displayed
   *      
   *
   *  returns: void
   *           
   */
  void
  LCD_SEG__vDisplayNumber(uint8_t);


#endif
