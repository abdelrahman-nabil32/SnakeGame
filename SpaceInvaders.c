
#include "./tm4c123gh6pm.h"
#include "Nokia5110.h"
#include "Random.h"
#include "TExaS.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#define WIDTH 84
#define HEIGHT 48
#define MAX_LENGTH 100
//here we may write the registers directories 

//here is the registers 

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

// Snake struct
typedef struct {
    int x[MAX_LENGTH];
    int y[MAX_LENGTH];
    int length;
    Direction direction;
} Snake;

// Food struct
typedef struct {
    int x;
    int y;
} Food;

bool outroWaitingFlag=0;
short int selectionWaiting=0;
int score=0;
int i;
int xPositionsArray[84/6];
int yPositionsArray[48/6];
Snake snake;
Food food;
volatile  long FallingEdges = 0;


void DisableInterrupts(void); // Disable interrupts
void EnableInterrupts(void);  // Enable interrupts
long StartCritical (void);    // previous I bit, disable interrupts
void EndCritical(long sr);    // restore I bit to previous value
void WaitForInterrupt(void);  // low power mode

void EdgeCounter_Init(void);






#define myBlankW     ((unsigned char)myBlank[18])
#define myBlankH     ((unsigned char)myBlank[22])
#define xPositionSize (WIDTH/myBlankW)
#define yPositionSize (HEIGHT/myBlankH)




// *************************** Capture image dimensions out of BMP**********



int main(void){
  //TExaS_Init(SSI0_Real_Nokia5110_Scope);  // set system clock to 80 MHz
  Random_Init(1);
  Nokia5110_Init();
	EdgeCounter_Init();           // initialize GPIO Port F interrupt


}


//function implementations 



// You can use this timer only if you learn how it works



void EdgeCounter_Init(void){  
  
  //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>	
  GPIO_PORTF_IS_R |= 0x00000000;     // (d) PF4,PF0 is edge-sensitive
  GPIO_PORTF_IBE_R |= 0x00;           //     PF4,PF0 is not both edges
  GPIO_PORTF_IEV_R |= 0x00000000;    //     PF4,PF0 falling edge event
  GPIO_PORTF_ICR_R |= ((1<<0) | (1<<1) | (1<<2) | (1<<3));      // (e) clear flag4,flag0
  GPIO_PORTF_IM_R |= ((1<<0) | (1<<1) | (1<<2) | (1<<3));      // (f) arm interrupt on PF4,PF0
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000; // (g) priority 5
  NVIC_EN0_R = 0x40000000;      // (h) enable interrupt 30 in NVIC
  EnableInterrupts();           // (i) Clears the I bit
}
void GPIOPortF_Handler(void){
	 if(GPIO_PORTF_RIS_R&(1<<0)) //SW2
	  {
		GPIO_PORTF_ICR_R = 1<<0; //Acknowledge PF0
		if(outroWaitingFlag){
			selectionWaiting=2;
			}else{
			if(snake.direction != LEFT)
					snake.direction = RIGHT;
			}
				//write the code of SW2
	  }
		else if(GPIO_PORTF_RIS_R&(1<<1)) //SW1
		{
		GPIO_PORTF_ICR_R = 1<<1; //Acknowledge PF4
			if(outroWaitingFlag){
			selectionWaiting=1;
			}else{
				if(snake.direction != UP)
			snake.direction = DOWN;
			}//write the code of SW1
	  }
		else if(GPIO_PORTF_RIS_R&(1<<2)) //SW1
		{
		GPIO_PORTF_ICR_R = 1<<2; //Acknowledge PF4
			if(snake.direction != RIGHT)
			snake.direction = LEFT;  //write the code of SW1
	  }
		else if(GPIO_PORTF_RIS_R&(1<<3)) //SW1
		{
		GPIO_PORTF_ICR_R = 1<<3;  //Acknowledge PF4
			if(snake.direction != DOWN)
			snake.direction = UP;   //write the code of SW1
	  }
}



