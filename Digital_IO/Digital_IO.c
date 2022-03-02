// Digital_IO.c

// February 24, 2022

//****************************************************************************

/*
      									CPE 355

										SysTick Delay
		
		This program counts the number of button presses (on SW1)
		And displays the count in decimal on two 7-segment displays
		Multiplexing the two hex displays wired parallel to eachother.
		
		PA7-6: Multiplexer for display control
		PF4: Input Switch
		PF2: Internal LED
		PB7-0: Output Port for 7-segment display

******************************************************************************
                                                                             *
             To display the contents of some SysTick registers:              *
                                                                             *
	 In Debug mode: Select Peripherals -> Core Peripherals -> SysTick Timer.   *
	                                                                           *
  																																					 *
******************************************************************************
*/


/*
	In this program, it is assumed that core clock is 16 MHz.
*/


#include "tm4c123gh6pm.h"
#include "SysTick.h"

#define HeartBeat() {GPIO_PORTF_DATA_R ^= 0x04;}

void PortA_Init(void);	// PA7 : Hex 0		PA6 : Hex 1
void PortB_Init(void);	// PB6-0 : 7-Segment Display Control Bits
void PortF_Init(void);	// PF4 : Digital Input Increments Counter		PF2 : LED output

unsigned char SW1;		// Input switch SW1 (PF4)
unsigned int counter;	// Counts number of button presses
unsigned char digit[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};	// Hex values for 7-segment display 

unsigned char button = 0;	//State machine: 0 = Released State,		1 = Pressed State
unsigned char hex = 0;		//State machine: 0 = PA7 Hex0,					1 = PA6 Hex1

int main(void){

	// Activate clock for part A, port B, and port F.
  SYSCTL_RCGCGPIO_R |= 0x23;  
										
	
	// Before we can set the other port registers, we need some delay.
	// We could do the usual while loop and check for PRGPIO bit ready.
	//while((SYSCTL_PRGPIO_R&0x00000022) == 0){}; // allow time for clock to start. Wait for status bit to be true.

	// But we could also insert a function that will take a few cycles instead.
	// Checking PRGPIO port status bit is better here, but also want to show that any function that takes 
	// a few cycles is also fine. The function that is inserted here is SysTick_Init().
	
	
  SysTick_Init();             // initialize SysTick timer
	
	PortA_Init();				// initialize PortA pins 7-6 Digital Output
	PortB_Init();				// initialize PortB pins 7-0 Digital Output
	PortF_Init();				// initialize PortF pin PF4 (SW1) Digital Input
  
  while(1){		
		
		HeartBeat();
		SW1 = GPIO_PORTF_DATA_R;	// Read input SW1
		
		// Switch, Counter, and Debouncing
		//--------------------------------------------------------------------
		if(SW1 == (0 << 4) && (button == 0)){		// If pressed event
			SysTick_Wait(16000*10);									// Wait for 1ms for debouncing
			SW1 = GPIO_PORTF_DATA_R;							// Read input SW1
			
			if(SW1 == (0 << 4))										// Correct read from SW1?
			{
				button = 1;													// State machine pressed
				counter++;													// Counter incremented
			}
		}
		
		else{																		// If released
			SysTick_Wait(16000*10);									// Wait for 1ms for debouncing
			SW1 = GPIO_PORTF_DATA_R;							// Read input SW1
			
			if(SW1 == (1 << 4))										// Correct read from SW1?
			{									
				button = 0;													// State machine released
			}
		}
		
		// Timer, Multiplexer, and 7-segment Displays
		//---------------------------------------------------------------------
		
		if(hex == 0){
			GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~(0x40)) | 0x80;	// Clear PA6, Set PA7
			GPIO_PORTB_DATA_R = digit[counter%10];										// Output Lower digit to Hex0
			hex = 1;
		}
		
		else{
			GPIO_PORTA_DATA_R = (GPIO_PORTA_DATA_R & ~(0x80)) | 0x40;	// Clear PA7, Set PA6
			GPIO_PORTB_DATA_R = digit[counter/10];										// Output Higher digit to Hex1
			hex = 0;
		}	
		
		if(counter == 100) counter = 0;	// Reset counter after 100
  }
}



//Subroutine to initialize port A pins for output
// Inputs:
// Outputs: PA7 and PA6
// Digital outputs gate 7-segment display common cathode
void PortA_Init(void){
	
	// Clock for PortA is activated before the function call, and should be given ample time for the status bit to be true
						//SYSCTL_RCGCGPIO_R |= 0x00000001;     
						//while((SYSCTL_PRGPIO_R&0x00000001) == 0){}; 
	
	// PortA pins are not locked
						//GPIO_PORTA_LOCK_R = 0x4C4F434B;   
						//GPIO_PORTA_CR_R |= 0xFF;          


  GPIO_PORTA_AMSEL_R &= ~(0xC0);        // 1) disable analog on PA7-6

  GPIO_PORTA_PCTL_R &= ~(0xFF000000);   // 2) PCTL GPIO on PA7-6. This writes zeros to all PMC bits. Not friendly code!

  GPIO_PORTA_DIR_R |= 0xC0;      				// 3) Pins PA7-6 are outputs. Set the bits

  GPIO_PORTA_AFSEL_R &= ~(0xC0);        // 4) disable alt funct on PA7-6

  GPIO_PORTA_DEN_R |= 0xC0;          		// 5) enable digital I/O on PA7-6
	
}


//Subroutine to initialize port B pins for output
// Inputs:
// Outputs: PB7-0 
// Digital outputs, PORT function
void PortB_Init(void){
	
	// Clock for PortB is activated before the function call, and should be given ample time for the status bit to be true
						//SYSCTL_RCGCGPIO_R |= 0x00000002;     
						//while((SYSCTL_PRGPIO_R&0x00000002) == 0){}; 
	
	// PortB pins are not locked
						//GPIO_PORTB_LOCK_R = 0x4C4F434B;   
						//GPIO_PORTB_CR_R |= 0xFF;          


  GPIO_PORTB_AMSEL_R &= ~(0xFF);        // 1) disable analog on PB7-0

  GPIO_PORTB_PCTL_R &= ~(0xFFFFFFFF);   // 2) PCTL GPIO on PB7-0. This writes zeros to all PMC bits. Not friendly code!

  GPIO_PORTB_DIR_R |= 0xFF;      				// 3) Pins PB7-0 are outputs. Set the bits

  GPIO_PORTB_AFSEL_R &= ~(0xFF);        // 4) disable alt funct on PB7-0

  GPIO_PORTB_DEN_R |= 0xFF;          		// 5) enable digital I/O on PF4 and PF2-0
	
}


// Subroutine to initialize port F pins for input and output
// PF4 is input SW1 
// Inputs: Sw1
// Outputs: LED PF2
// Notes: Digital inputs, PORT function
void PortF_Init(void){ 
	
	// Clock for PortB is activated before the function call, and should be given ample time for the status bit to be true
						//SYSCTL_RCGCGPIO_R |= 0x00000020;     
						//while((SYSCTL_PRGPIO_R&0x00000020) == 0){}; 
	
	// PortF pins PF4 and PF2 are not locked
						//GPIO_PORTF_LOCK_R = 0x4C4F434B;   
						//GPIO_PORTF_CR_R |= 0x14;     

  GPIO_PORTF_AMSEL_R &= ~(0x14);        // 1) disable analog on PF4 and PF2

  GPIO_PORTF_PCTL_R &= ~(0x000F0F00);   // 2) PCTL GPIO on PF4 and PF2

	GPIO_PORTF_DIR_R &= ~(0x10);	 				// 3) Pin PF4 is an inputs. Clear the bits
	GPIO_PORTF_DIR_R |= (0x04);						// 4) Pin PF2 is an output.	Set the bits

  GPIO_PORTF_AFSEL_R &= ~(0x14);        // 5) disable alt funct on PF4 and PF2

  GPIO_PORTF_PUR_R |= 0x10;          		// 6) enable pull-up on PF4

  GPIO_PORTF_DEN_R |= 0x14;          		// 7) enable digital I/O on PF4 and PF2
}


