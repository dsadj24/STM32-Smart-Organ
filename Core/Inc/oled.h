#ifndef __OLED_H
#define __OLED_H

#include "stm32f1xx_hal.h"

/* OLED 硬件配置 */
#define OLED_SCL_PIN    GPIO_PIN_6
#define OLED_SDA_PIN    GPIO_PIN_7
#define OLED_SCL_PORT   GPIOB
#define OLED_SDA_PORT   GPIOB

/* OLED 命令定义 */
#define OLED_CMD        0x00
#define OLED_DATA       0x40

/* OLED 屏幕参数 */
#define OLED_WIDTH      128
#define OLED_HEIGHT     64

/* 函数声明 */
void OLED_I2C_Init(void);
void OLED_I2C_Start(void);
void OLED_I2C_Stop(void);
void OLED_I2C_WriteByte(uint8_t byte);
void OLED_WriteCmd(uint8_t cmd);
void OLED_WriteData(uint8_t data);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetPos(uint8_t x, uint8_t y);
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr);
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *str);
void OLED_Refresh(void);

#endif