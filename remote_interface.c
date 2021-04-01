/**
 * main.c
 */
#include "msp.h"
#include "uart.h"
#include "adc.h"

#define LOL 0x75

#define RED BIT6
#define GREEN BIT4
#define BLUE BIT6
#define PB2 BIT5
#define PB1 BIT1

#define S1_PIN P5
#define S2_PIN P3
#define RED_PIN P2
#define BLUE_PIN P5
#define GREEN_PIN P2
#define ONE_CYCLE 3006 //1ms delay

#define X_INPUT_CHAN 15 //use A15 connected to p6.0
#define Y_INPUT_CHAN 9 //use A9 connected to p4.4

int S1_PRESSED[] = { 'S', '1', ' ', 'p', 'r', 'e', 's', 's', 'e', 'd' };
int S2_PRESSED[] = { 'S', '2', ' ', 'p', 'r', 'e', 's', 's', 'e', 'd' };

void msDelay(unsigned int ms)
{
    while (ms > 0)
    {
        _delay_cycles(ONE_CYCLE);
        ms--;
    }
}

/*int transfer(int value)
 {
 int value_5;
 int value_4;
 int value_3;
 int value_2;
 int value_1;

 int division=[]={10000, 1000, 100, 10,1};

 int index=0;
 int value_size;
 value_size=sizeof(division) / sizeof(division[0]);
 while(index<=value_size)
 {
 what[index]=value/division[index]%10;
 }
 what[index]=value
 value_5 = value / 10000;
 value_4 = (value - value_5 * 10000) / 1000;
 value_3 = (value - value_5 * 10000 - value_4 * 1000) / 100;
 value_2 = (value - value_5 * 10000 - value_4 * 1000 - value_3 * 100) / 10;
 value_1 = (value - value_5 * 10000 - value_4 * 1000 - value_3 * 100
 - value_2 * 10);

 return value_5
 }*/

void main(void)
{
    int value_5;
    int value_4;
    int value_3;
    int value_2;
    int value_1;
    int value;

    int data;
    volatile int result_x;
    volatile int result_y;

    int division[] = { 100000, 10000, 1000, 100, 10 };
    //int division=[]=
    //{   100000, 10000, 1000, 100,10};

    int x_broken[5];
    int y_broken[5];

    int value_size;
    value_size = sizeof(division) / sizeof(division[0]);

    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    RED_PIN->DIR |= RED;
    GREEN_PIN->DIR |= GREEN;
    BLUE_PIN->DIR |= BLUE;

//enable port3 and port 5 for interrupt
    S2_PIN->IE |= PB2;
    S1_PIN->IE |= PB1;

//edge trigger on falling for port3.5 and port5.1
    S2_PIN->IES |= PB2;
    S1_PIN->IES |= PB1;

//clear flag for port3.5 and 5.1
    S2_PIN->IFG &= ~PB2;
    S1_PIN->IFG &= ~PB1;

//enable NVIC for port 3 and port5
    NVIC_EnableIRQ(PORT3_IRQn);
    NVIC_EnableIRQ(PORT5_IRQn);

//global interrupt enable
    __enable_interrupts();

//configure Pin 4.4 as an analog input
    P4->SEL0 |= BIT4;
    P4->SEL1 |= BIT4;

//configure Pin 6.0 as an analog input
    P6->SEL0 |= BIT0;
    P6->SEL1 |= BIT0;

    uartInit();

    while (1)
    {
        int x_index = 0;
        int y_index = 0;

        data = uartGetC();
        uartPutC(data);

        if (data == 'R')
        {
            RED_PIN->OUT ^= RED;
        }
        else if (data == 'G')
        {
            //turn on green LED and turn off the rest
            GREEN_PIN->OUT ^= GREEN;
        }
        else if (data == 'B')
        {
            //turn on blue LED and turn off the rest
            BLUE_PIN->OUT ^= BLUE;
        }
        else if (data == 'J')
        {
            data = uartGetC();
            uartPutC(data);
            if (data == 'V')
            {
                adcInit(Y_INPUT_CHAN);
                result_y = adcSample();
                uartPutC(5);
                while (y_index < value_size)
                {
                    value=result_y;
                    value_5 = value / 10000;
                    value_4 = (value - value_5 * 10000) / 1000;
                    value_3 = (value - value_5 * 10000 - value_4 * 1000) / 100;
                    value_2 = (value - value_5 * 10000 - value_4 * 1000 - value_3 * 100) / 10;
                    value_1 = (value - value_5 * 10000 - value_4 * 1000 - value_3 * 100
                    - value_2 * 10);//y_broken[y_index] = result_y % division[y_index];
                    //uartPutC(y_broken[y_index]);
                    uartPutC(value_5);
                    uartPutC(value_4);
                    uartPutC(value_3);
                    uartPutC(value_2);
                    uartPutC(value_1);
                    y_index++;
                }

            }
            else if (data == 'H')
            {
                adcInit(X_INPUT_CHAN);
                result_x = adcSample();
                while (x_index < value_size)
                {
                    x_broken[x_index] = result_x % division[x_index];
                    uartPutC(x_broken[x_index]);
                    x_index++;
                }
            }
            else
            {
            }
        }
    }

}

void PORT3_IRQHandler(void)
{

    int index = 0;
    int arraySize;
    arraySize = sizeof(S1_PRESSED) / sizeof(S1_PRESSED[0]);

    msDelay(10);
//S2 toggle the red LED and clear the flag after 10ms of delay
    S2_PIN->IFG &= ~PB2;

//debounce the push button by checking if the button is still pushed after 10ms of delay
    if (S2_PIN->IN & PB2)
    {
    }
    else
    {
        while (index < arraySize)
        {
            uartPutC(S1_PRESSED[index]);
            index++;
        }
    }

}

void PORT5_IRQHandler(void)
{
    int index = 0;
    int arraySize;
    arraySize = sizeof(S2_PRESSED) / sizeof(S2_PRESSED[0]);

    msDelay(10);
//S1 toggle the blue LED and clear the flag after 10ms of delay
    S1_PIN->IFG &= ~PB1;

//debounce the push button by checking if the button is still pushed after 10ms of delay
    if (S1_PIN->IN & PB1)
    {
    }
    else
    {
        while (index < arraySize)
        {
            uartPutC(S2_PRESSED[index]);
            index++;
        }
    }
}
