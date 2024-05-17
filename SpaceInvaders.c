
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
void initializeSnake(Snake *snake, int x, int y);
void moveSnake(Snake *snake);
bool checkFood(Snake *snake, Food *food);
void generateNewFood(Food *food);
void initializeFood(Food *food);
void PortF_Init(void);
void Timer2_Init(unsigned long period);
bool checkCollision(Snake *snake, int newX, int newY);



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

void PortF_Init(void){ volatile unsigned long delay;
  SYSCTL_RCGC2_R |= 0x00000020;     // 1) F clock
  delay = SYSCTL_RCGC2_R;           // delay
  GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 2) unlock PortF PF0
  GPIO_PORTF_CR_R = 0x1F;           // allow changes to PF4-0
  GPIO_PORTF_AMSEL_R = 0x00;        // 3) disable analog function
  GPIO_PORTF_PCTL_R = 0x00000000;   // 4) GPIO clear bit PCTL
  GPIO_PORTF_DIR_R = 0x10;          // 5) PF0-3 input, PF4 output
  GPIO_PORTF_AFSEL_R = 0x00;        // 6) no alternate function
  GPIO_PORTF_PUR_R = 0x1F;          // enable pullup resistors on PF4,PF0
  GPIO_PORTF_DEN_R = 0x1F;          // 7) enable digital pins PF4-PF0
}


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

//////////////**************
// Initialize snake at given position
void initializeSnake(Snake *snake, int x, int y) {
    snake->x[0] = x;
    snake->y[0] = y;
    snake->length = 1;
    snake->direction = RIGHT;
}
// Move snake
void moveSnake(Snake *snake) {
    // Move body segments
    for ( i = snake->length - 1; i > 0; --i) {
        snake->x[i] = snake->x[i-1];
        snake->y[i] = snake->y[i-1];
    }
    
    // Move head based on direction
    switch (snake->direction) {
        case UP:
            snake->y[0]-=myBlankH;
            break;
        case DOWN:
            snake->y[0]+=myBlankH;
            break;
        case LEFT:
            snake->x[0]-=myBlankW;
            break;
        case RIGHT:
            snake->x[0]+=myBlankW;
            break;
    }
}
void initializeFood(Food *food) {
  int Xtemp = xPositionsArray[rand() % xPositionSize];
	int Ytemp = yPositionsArray[rand() % yPositionSize];
			
	while(checkCollision(&snake,Xtemp,Ytemp)||(snake.x[0] == Xtemp) || (snake.y[0] == Ytemp))
	{
		 Xtemp = xPositionsArray[rand() % xPositionSize];
		 Ytemp = yPositionsArray[rand() % yPositionSize];
	}
    food->x = Xtemp;
    food->y = Ytemp;
}
// Check if snake has eaten food
bool checkFood(Snake *snake, Food *food) {
    return (snake->x[0] == food->x && snake->y[0] == food->y);
}
// Generate new food at random position
void generateNewFood(Food *food) {
    int Xtemp = xPositionsArray[rand() % xPositionSize];
		int Ytemp = yPositionsArray[rand() % yPositionSize];
			
		while(checkCollision(&snake,Xtemp,Ytemp)||(snake.x[0] == Xtemp) || (snake.y[0] == Ytemp))
		{
			Xtemp = xPositionsArray[rand() % xPositionSize];
			Ytemp = yPositionsArray[rand() % yPositionSize];
		}
		food->x = Xtemp;
    food->y = Ytemp;
}

// You can use this timer only if you learn how it works
void Timer2_Init(unsigned long period){
  unsigned long volatile delay;
  SYSCTL_RCGCTIMER_R |= 0x04;   // 0) activate timer2
  delay = SYSCTL_RCGCTIMER_R;	
  TIMER2_CTL_R = 0x00000000;    // 1) disable timer2A during setup
  TIMER2_CFG_R = 0x00000000;    // 2) configure for 32-bit mode
  TIMER2_TAMR_R = 0x00000002;   // 3) configure for periodic mode, default down-count settings
  TIMER2_TAILR_R = period-1;    // 4) reload value
  TIMER2_TAPR_R = 0;            // 5) bus clock resolution
  TIMER2_ICR_R = 0x00000001;    // 6) clear timer2A timeout flag
  TIMER2_IMR_R = 0x00000001;    // 7) arm timeout interrupt
  NVIC_PRI5_R = (NVIC_PRI5_R&0x00FFFFFF)|0x80000000; // 8) priority 4
// interrupts enabled in the main program after all devices initialized
// vector number 39, interrupt number 23
  NVIC_EN0_R = 1<<23;           // 9) enable IRQ 23 in NVIC
  TIMER2_CTL_R = 0x00000001;    // 10) enable timer2A
}
void Timer2A_Handler(void){ 
  TIMER2_ICR_R = 0x00000001;   // acknowledge timer2A timeout
	 moveSnake(&snake);
	if (checkFood(&snake, &food)) 
		{
			if(outroWaitingFlag==0){
			++score;
			}
     generateNewFood(&food);
		 snake.length++; //increase the snake length
    }
}
bool checkCollision(Snake *snake, int newX, int newY) {
    // Check wall collision
    if (newX < 0 || newX >= WIDTH || newY < 0 || newY >= HEIGHT) {
        return true;
    }
    
    // Check self collision
    for ( i = 1; i < snake->length; ++i) {
        if (snake->x[i] == newX && snake->y[i] == newY) {
            return true;
        }
    }
    
    return false;
}
