// SpaceInvaders.c
// Runs on LM4F120/TM4C123
// Jonathan Valvano and Daniel Valvano
// This is a starter project for the EE319K Lab 10

// Last Modified: 11/21/2017 
// http://www.spaceinvaders.de/
// sounds at http://www.classicgaming.cc/classics/spaceinvaders/sounds.php
// http://www.classicgaming.cc/classics/spaceinvaders/playguide.php
/* This example accompanies the books
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2017

   "Embedded Systems: Introduction to Arm Cortex M Microcontrollers",
   ISBN: 978-1469998749, Jonathan Valvano, copyright (c) 2017

 Copyright 2017 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */
// ******* Possible Hardware I/O connections*******************
// Slide pot pin 1 connected to ground
// Slide pot pin 2 connected to PE2/AIN1
// Slide pot pin 3 connected to +3.3V 
// fire button connected to PE0
// special weapon fire button connected to PE1
// 8*R resistor DAC bit 0 on PB0 (least significant bit)
// 4*R resistor DAC bit 1 on PB1
// 2*R resistor DAC bit 2 on PB2
// 1*R resistor DAC bit 3 on PB3 (most significant bit)
// LED on PB4
// LED on PB5

// Backlight (pin 10) connected to +3.3 V
// MISO (pin 9) unconnected
// SCK (pin 8) connected to PA2 (SSI0Clk)
// MOSI (pin 7) connected to PA5 (SSI0Tx)
// TFT_CS (pin 6) connected to PA3 (SSI0Fss)
// CARD_CS (pin 5) unconnected
// Data/Command (pin 4) connected to PA6 (GPIO), high for data, low for command
// RESET (pin 3) connected to PA7 (GPIO)
// VCC (pin 2) connected to +3.3 V
// Gnd (pin 1) connected to ground

#include <stdint.h>
//#include <stdio.h>
#include "tm4c123gh6pm.h"
#include "ST7735.h"
#include "Random.h"
#include "PLL.h"
#include "TExaS.h"
#include "ADC.h"
#include "print.h"
#include "sound.h"
#include "Timer0.h"
#include "Sprites.c"


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
extern void ADC_Init(void);
extern uint32_t ADC_In(void);
void Delay100ms(uint32_t count); // time delay in 0.1 seconds
void Delay1ms(uint32_t count); // time delay in 0.1 seconds

#define PF1       (*((volatile uint32_t *)0x40025008))
#define PF2       (*((volatile uint32_t *)0x40025010))
#define PF3       (*((volatile uint32_t *)0x40025020))

// *************************** Images ***************************
// enemy ship that starts at the top of the screen (arms/mouth closed)
// width=16 x height=10





//************************************************************************************
enum direction {forward, right, backwards, left};
enum health {alive, dead};
//
struct sprite {
int16_t x; 
int16_t y; 
const uint16_t *image; 
enum direction dir;	
int16_t w; 
int16_t h;
int16_t dx;
int16_t dy;
enum health status;
};

typedef struct sprite sp_t;
sp_t character = {52, 140, sprite_forward, forward, 30, 30, alive};
sp_t bullet = { 0, 0, bullet_sprite, forward, 5, 5, 0, -1,alive};

sp_t enemy = {50, 50, enemy_backward, forward, 30, 30,alive};
sp_t enemy_shot = {71, 50, bullet_sprite, forward, 5, 5, 0, -1,alive};
sp_t erase = {0, 0, erase_sprite, backwards, 30, 30,alive};

sp_t mini_gun = {50, 140, gun_small, backwards, 40, 84,alive};
sp_t mini_solider = {50, 50, solider_small, backwards, 70, 70,alive};


void SysTick_Init(void){
  NVIC_ST_CTRL_R = 0;                   
  NVIC_ST_RELOAD_R = 0x145855;
  NVIC_ST_CURRENT_R = 0;  
	NVIC_SYS_PRI3_R = NVIC_SYS_PRI3_R&0x00FFFFFF; // This is to set the priority to zero	
  NVIC_ST_CTRL_R = 0x00000007;
}


void SysTick_Handler(void){
	PF2 ^= 0x04;
}


void PortE_Init(void){
	volatile int delay = 0;
	SYSCTL_RCGCGPIO_R |= 0x10;
	delay++;delay++;delay++;delay++;delay++;
	delay++;delay++;delay++;delay++;delay++;
	GPIO_PORTE_DEN_R |= 0x01F;
	GPIO_PORTE_DIR_R &= ~0xE0;
	GPIO_PORTE_AMSEL_R &= ~0x1F;
	GPIO_PORTE_AFSEL_R &= ~0x1F;
}


void Button_Check (void){	
	if((GPIO_PORTE_DATA_R & 0x02) == 0x02){
			character.y = (character.y - 1);
			character.image = sprite_forward;
			character.dir = forward;
		}
	if((GPIO_PORTE_DATA_R & 0x04) == 0x04){
			character.x = (character.x + 1);
			character.image = sprite_right;
			character.dir = right;
		}
	if((GPIO_PORTE_DATA_R & 0x08) == 0x08){
			character.y = (character.y + 1);
			character.image = sprite_backward;
			character.dir = backwards;
		}
	if((GPIO_PORTE_DATA_R & 0x10) == 0x10){
			character.x = (character.x - 1);
			character.image = sprite_left;
			character.dir = left;
		}	
	if((GPIO_PORTE_DATA_R & 0X01) == 0X01){
			if(character.dir == forward){
				bullet.x = character.x + 21;
				bullet.y = character.y - 21;
				bullet.dx = 0;
				bullet.dy = -1;
			}
		 if(character.dir == right){
				bullet.x = character.x +21;
				bullet.y = character.y - 5;
				bullet.dx = 1;
				bullet.dy = 0;
			}
		 if(character.dir == backwards){
				bullet.x = character.x + 21;
				bullet.y = character.y - 1;
				bullet.dx = 0;
				bullet.dy = 1;
			}
		 if(character.dir == left){
				bullet.x = character.x -1;
				bullet.y = character.y - 5;
				bullet.dx = -1;
				bullet.dy = 0;			
			}
		}
			 
	ST7735_DrawBitmap(character.x, character.y, character.image, character.w, character.h);
	if(enemy.status != dead){
	ST7735_DrawBitmap(enemy.x, enemy.y, enemy.image, enemy.w, enemy.h);
	}
	bullet.x = bullet.x + bullet.dx;
	bullet.y = bullet.y + bullet.dy;
	ST7735_DrawBitmap(bullet.x, bullet.y, bullet.image, 5, 5);
}

void Death_Check (void){
	if ((bullet.y - enemy.y) == 0 && ((bullet.x - enemy.x) >= 0x03 && (bullet.x - enemy.x) <= 0x18)) {
		erase.x = enemy.x;
		erase.y = enemy.y;
		enemy.status = dead;
		}
		ST7735_DrawBitmap(erase.x, erase.y, erase.image, erase.w, erase.h);
}

void Enemy_Shot(void){
	enemy_shot.x = enemy_shot.x + 0;
	enemy_shot.y = enemy_shot.y + 1;
	ST7735_DrawBitmap(enemy_shot.x, enemy_shot.y, enemy_shot.image, 5, 5);
	if(enemy_shot.y == 0xA4){
		enemy_shot.y = enemy.y;
		enemy_shot.x = enemy.x + 21;
	}
}
	
void Mini_Game (void){
	int minigame = 0;
	uint32_t data = ADC_In();
	if (((character.x - enemy.x) >= -18 && (character.x - enemy.x) <= 0x20) && ((character.y - enemy.y) <= 13 && (character.y - enemy.y) >= -18)) {

  ST7735_FillScreen(0x0000);            // set screen to black
	while(minigame == 0){
		ST7735_DrawBitmap(mini_gun.x, mini_gun.y, mini_gun.image, mini_gun.w, mini_gun.h);	
		ST7735_DrawBitmap(mini_solider.x, mini_solider.y, mini_solider.image, mini_solider.w, mini_solider.h);
			if(data >= 2000){
				minigame = 1;
			}
	}
	}
}


// *************************** Capture image dimensions out of BMP**********

int main(void){
  PLL_Init(Bus80MHz);       // Bus clock is 80 MHz 
  Random_Init(1);
  Output_Init();
	PortE_Init();
	ADC_Init();
  ST7735_FillScreen(0x0000);            // set screen to black
  while(1){
		Button_Check();
		Death_Check();
		Enemy_Shot();
		Mini_Game();
  }
}


// You can use this timer only if you learn how it works

void Delay100ms(uint32_t count){uint32_t volatile time;
  while(count>0){
    time = 727240;  // 0.1sec at 80 MHz
    while(time){
	  	time--;
    }
    count--;
  }
}
