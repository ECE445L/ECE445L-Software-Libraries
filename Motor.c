/* Motor.c
 * Jonathan Valvano
 * RSLK v2.0.2
 * June 28, 2024
// Motor
// TM4C MSPM0
// PF2  PB4  Motor_PWML, ML+, IN3, PWM M1PWM6
// PF3  PB1  Motor_PWMR, MR+, IN1, PWM M1PWM7
// PA3  PB0  Motor_DIR_L,ML-, IN4, GPIO, 0 means forward, 1 means backward
// PA2  PB16 Motor_DIR_R,MR-, IN2, GPIO, 0 means forward, 1 means backward
 *
 */
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/Motor.h"
#include "../inc/Clock.h"
    // write this
    // ***solution***
#define PA2             (*((volatile uint32_t *)0x40004010))
#define PA3             (*((volatile uint32_t *)0x40004020))

#define DIR_L 8
#define DIR_R 4
void Motor_Init(void){
  SYSCTL_RCGCPWM_R |= 0x02;         // 1) activate PWM1
  SYSCTL_RCGCGPIO_R |= 0x00000021;  // activate clock for Port A,F
  while((SYSCTL_PRGPIO_R&0x21) != 0x21){};// allow time for clock to stabilize
  GPIO_PORTA_DIR_R |= 0x0C;        // PA3 PA2 outputs
  GPIO_PORTA_DEN_R |= 0x0C;        // enable digital I/O on PA3 PA2
  PA2 = PA3 = 0;  // forward
  GPIO_PORTF_AFSEL_R |= 0x0C;     // enable alt funct on PF3 PF2
  GPIO_PORTF_PCTL_R = (GPIO_PORTF_PCTL_R&0xFFFFF00FF)|0x00005500;
    // PF2 is M1PWM6
    // PF3 is M1PWM7
  GPIO_PORTF_DIR_R |= 0x0C;             // PF3 PF2 output
  GPIO_PORTF_DEN_R |= 0x0C;             // enable digital I/O on PF3 PF2
  GPIO_PORTF_DR8R_R |= 0x0C;            // high current output on PF3 PF2
    // bit 20=1 for USEPWMDIV
    // bits 19-17 = 0 for divide by 2
    // bits 19-17 = n for divide by 2*(2^n)
    // bits 19-17 = 3 for divide by 16
    // bits 19-17 = 5 for divide by 64
  SYSCTL_RCC_R = 0x00120000 |           // 3) use PWM divider 
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /4 divider
  PWM1_3_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM1_3_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PF2 goes low on LOAD
  // PF2 goes high on CMPA down
  PWM1_3_LOAD_R = MOTOR_PERIOD - 1;    // 5) cycles needed to count down to 0
  PWM1_3_CMPA_R = 10;                  // 6) count value when output rises
    
  PWM1_3_GENB_R = 0xC08;               // low on LOAD, high on CMPB down
  // PF3 goes low on LOAD
  // PF3 goes high on CMPB down  
  PWM1_3_CMPB_R = 10;                   // 6) count value when output rises
  PWM1_3_CTL_R |= 0x00000001;           // 7) start PWM1
  PWM1_ENABLE_R |= 0x000000C0;          // enable PF2 is M1PWM6 and PF3 is M1PWM7
}

void Motor_Forward(uint32_t dutyLeft, uint32_t dutyRight){
  PA2 = PA3 = 0;  // forward
  PWM1_3_CMPA_R = dutyLeft;
  PWM1_3_CMPB_R = dutyRight;
}

void Motor_Backward(uint32_t dutyLeft, uint32_t dutyRight){
  PA2 = DIR_R;
  PA3 = DIR_L;  // backward
  PWM1_3_CMPA_R = MOTOR_PERIOD-dutyLeft;
  PWM1_3_CMPB_R = MOTOR_PERIOD-dutyRight;
}

void Motor_Right(uint32_t dutyLeft, uint32_t dutyRight){
  PA2 = DIR_R; // backward
  PA3 = 0;     // forward
  PWM1_3_CMPA_R = dutyLeft;
  PWM1_3_CMPB_R = MOTOR_PERIOD-dutyRight;
}

void Motor_Left(uint32_t dutyLeft, uint32_t dutyRight){
  PA2 = 0;         // forward
  PA3 = DIR_L;     // backward
  PWM1_3_CMPA_R = MOTOR_PERIOD-dutyLeft;
  PWM1_3_CMPB_R = dutyRight;
}

// **********end of solution

