/*
 * File:   main.c
 * Author: Vili Ståhlberg
 *
 * Created on December 5, 2021, 6:34 PM
 */


#include <avr/io.h>
#include "FreeRTOS.h"
#include "clock_config.h"
#include "task.h"
#include "queue.h"
#include <string.h>

#define F_CPU 3333333
#define BLINK_DELAY 250
#define USART0_BAUD_RATE(BAUD_RATE) ((float)(F_CPU * 64 / (16 * (float)BAUD_RATE)) + 0.5)

QueueHandle_t g_msg_queue; // Global variable: message queue
QueueHandle_t g_digit_queue; // Global variable: message queue

// Function to send a character to the user via USART.
void USART0_sendChar(char c)
{
    while (!(USART0.STATUS & USART_DREIF_bm))
    {
        ;
    }
    USART0.TXDATAL = c;
}

// Function to read values from the user via USART
char USART0_read()
{
    while (!(USART0.STATUS & USART_RXCIF_bm))
    {
        ;
    }
    return USART0.RXDATAL;
}

// Function to initialize USART
void USART0_init(void)
{
    PORTA.DIR &= ~PIN1_bm;
    PORTA.DIR = PIN0_bm;
    USART0.BAUD = (uint16_t)USART0_BAUD_RATE(9600);
    USART0.CTRLB |= USART_TXEN_bm; 
    USART0.CTRLB |= USART_RXEN_bm;
}

// This task takes care of displaying the correct digit.
void display(void* parameter)
{
    uint8_t digit;
    uint8_t digits[] =
    {
        0b00111111, 0b00000110,
        0b01011011, 0b01001111,
        0b01100110, 0b01101101,
        0b01111101, 0b00000111,
        0b01111111, 0b01100111,
        0b01111001, 0b00000000
    };
    PORTC.DIRSET = 0xFF;
    while (1)
    {
        if (xQueueReceive(g_digit_queue, (void*)&digit, 0) == pdTRUE)
        {
            PORTC.OUT = digits[digit];
        }
        vTaskDelay(pdMS_TO_TICKS(BLINK_DELAY));
    }
    vTaskDelete(NULL);
}

void send(void* parameter)
{
    char c;
    for (;;)
    {
        if (xQueueReceive(g_msg_queue, (void*)&c, 0) == pdTRUE)
        {
            USART0_sendChar(c);
        }
    }
    vTaskDelete(NULL);
}

/*
 * This task will:
 * Read the value from USART
 * Convert the char to a uint8_t
 * Add the digit to the digit queue
 * Add the message to the message queue (char by char)
 */
void receive(void* parameter){
    uint8_t digit; // A variable for storing the actual digit value in type uint8_t
    char c; // A variable for storing the value read with USART.
    char queueChar; // The char stored in this variable will be sent to the send-task.
    char msg1[] = "It's a valid digit.\r\n";
    char msg2[] = "Error! Not a valid digit.\r\n";
    for (;;)
    {
        c = USART0_read();
        switch(c)
        {
            case '0':
                digit = 0;
            break;
            case '1':
                digit = 1;
            break;
            case '2':
                digit = 2;
            break;
            case '3':
                digit = 3;
            break;
            case '4':
                digit = 4;
            break;
            case '5':
                digit = 6;
            break;
            case '6':
                digit = 6;
            break;
            case '7':
                digit = 7;
            break;
            case '8':
                digit = 8;
            break;
            case '9':
                digit = 9;
            break;
            default:
                digit = 10;
        }
        if (xQueueSend(g_digit_queue, (void*)&digit, 10) == pdTRUE)
        {
            if (digit == 10)
            {
                for (uint8_t i = 0; i < strlen(msg2); i++)
                {
                    queueChar = msg2[i];
                    if(xQueueSend(g_msg_queue, (void*)&queueChar, 10))
                    {
                        ;
                    }
                }
            }
            else
            {
                for (uint8_t i = 0; i < strlen(msg1); i++)
                {
                    queueChar = msg1[i];
                    if(xQueueSend(g_msg_queue, (void*)&queueChar, 10))
                    {
                        ;
                    }
                }
            }
        }
    }
    vTaskDelete(NULL);
}

int main(void)
{
    USART0_init();
    g_digit_queue = xQueueCreate(10, sizeof(uint8_t));
    g_msg_queue = xQueueCreate(150, sizeof(char));
    
    // Create a task that handles displaying digits.
    xTaskCreate(
    display,
    "display",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY,
    NULL
    );
    
    // Create a task that handles sending messages via USART.
    xTaskCreate(
    send,
    "send",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY,
    NULL
    );
    
    // Create a task that handles reading messages.
    xTaskCreate(
    receive,
    "receive",
    configMINIMAL_STACK_SIZE,
    NULL,
    tskIDLE_PRIORITY,
    NULL
    );

    // Start the scheduler
    vTaskStartScheduler();

    // Scheduler will not return
    return 0;
}