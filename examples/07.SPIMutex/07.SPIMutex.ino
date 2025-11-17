/*!
 *@file SPIMutex.ino
 *@brief SPI Resource Lock
 *@details Since the DFR1195 screen occupies the SPI pins, operating SPI peripherals in multithreaded 
           programming may cause errors. Therefore, we have built-in an SPI resource lock "SPI_MUTEX" 
           in this project to ensure thread safety. This example demonstrates how to use SPI when 
           implementing multithreaded programming in the project.
 *@copyright Copyright (c) 2010 DFRobot Co.Ltd (http://www.dfrobot.com)
 *@licence The MIT License (MIT)
 *@author [Martin](Martin@dfrobot.com)
 *@version V0.0.1
 *@date 2025-3-28
 *@url https://github.com/DFRobot/DFRobot_LoRaWAN_ESP32S3
 */

 #include "DFRobot_LoRaWAN.h"

 LCD_OnBoard screen;
 
 #define BG_COLOR        COLOR_RGB565_BLACK      // Screen background color
 #define TEXT_COLOR      COLOR_RGB565_GREEN      // Screen font color
 /*
 * Recommended font
 * -------------------------------------------------------------------------------
 * FreeMono9pt7b, FreeMono12pt7b, FreeMonoBold9pt7b, FreeSans9pt7b, FreeSerif9pt7b
 * For more fonts, see the directory: src\external\DFRobot_GDL_LW\src\Frame\Fonts\
 */ 
 #define TEXT_FONT       &FreeMono9pt7b          // font
 #define TEXT_SIZE       1                       // Screen font size
 #define LINE_HEIGHT     18                      // Line height
 #define POX_X           0                       // Screen print position X coordinate
 #define POX_Y           15                      // Screen print position Y coordinate
 #define LINE_1          0                       // Line number
 #define LINE_2          1
 #define LINE_3          2
 #define LINE_4          3
 
 uint32_t count = 0;
 
 void spi_user_Task(void *pvParameters)
 {
     while (1)
     {
         if (xSemaphoreTake(SPI_MUTEX, portMAX_DELAY) == pdTRUE)
         {   
             // Operate your SPI peripherals here
 
             printf("User Get SPI_MUTEX\n");
             xSemaphoreGive(SPI_MUTEX);
         }
         delay(300);     // Add some delay to allow the display to acquire SPI_MUTEX
     }
 }
 
 void setup()
 {
     Serial.begin(115200);
     screen.begin();
     /*
     * The screen backlight is turned on by default. 
     * You can use this method to control the backlight switch. 
     * When the backlight is turned off, the screen will not display any content.
     */
     // screen.Backlight(ON);
 
     xTaskCreate(spi_user_Task, "SPI_USER", 8192, NULL, 2, NULL);    // Create your thread
     
     screen.fillScreen(BG_COLOR);    
     screen.setTextColor(TEXT_COLOR);
     screen.setFont(TEXT_FONT);
     screen.setTextSize(TEXT_SIZE);
     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
     screen.printf("SPIMutex");
     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
     screen.printf("Test");
     delay(2000);
 
 }
 
 void loop()
 {
     screen.fillScreen(BG_COLOR);    
     screen.setTextColor(TEXT_COLOR);
     screen.setFont(TEXT_FONT);
     screen.setTextSize(TEXT_SIZE);
     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_1);
     screen.printf("Sending...");
     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_2);
     screen.printf("Confirmed");
     screen.setCursor(POX_X, POX_Y + LINE_HEIGHT * LINE_3);
     screen.printf("Packet %dst", count++);
 
     delay(3000);
 }