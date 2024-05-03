#pragma once

#include "miosix.h"

#define ADC_MAX_VALUE 1024.0f
namespace Joystick{

    void initJoystick(uint16_t pinX, uint16_t pinY){
        miosix::FastInterruptDisableLock dLock; //prevent interrupts from interfering with the initialization process
        
        // Enable ADC1 Clock
        RCC->APB2ENR |= (1 << 8); //ADC1 clock enable [DOC 251]

        // Enable GPIOA clock, assumed PINS PA2 and PA5
        RCC->AHB1ENR |= (1 << 8); //IO port A clock enable [DOC 244], can be also used RCC_AHB1ENR_GPIOAEN

        // Prescaler
        ADC->CCR = (2 << 16); //Modify ADCPRE, bit 17:16 to 01 [DOC 430]

        // Resolution [DOC 419]
        ADC1->CR1 |= (1 << 8);  // enable scan Mode (converts all enabled channels sequentially)
        ADC1->CR1 |= (1 << 24); // 10-bit resolution bit 25:24 = 01

        // Data Alignment [DOC 421]
        ADC1->CR2 &= ~(1 << 11); // Left aligned
        ADC1->CR2 |= (1 << 10);  // EOC (End of Conversion) bit after every sequence
        ADC1->CR2 |= (1 << 1);   // Continuous conversion mode (starts a new conversion as soon as the previous conversion has finished)

        // Sampling time for adc channel [DOC 423]
        ADC1->SMPR2 |= (1 << 3 * (uint8_t) pinX);
        ADC1->SMPR2 |= (1 << 3 * (uint8_t) pinY);

        // Sequence for 1 channel conversions [DOC 425]
        ADC1->SQR1 |= (1 << 1);

        // Analog Mode for the Pin [DOC 284]
        GPIO_TypeDef* GPIO = (GPIO_TypeDef*) GPIOA_BASE;
        GPIO->MODER |= (3 << pinX * 2); //Analog Mode 11
        GPIO->MODER |= (3 << pinY * 2); //Analog Mode 11

        //Enable the ADC [DOC 421]
        ADC1->CR2 |= 1 << 0; //ADON bit to 1 to enable

        // Wait to stabilize ADC
        miosix::delayUs(100);

    }

    uint16_t readChannel(uint8_t channel)
    {
        miosix::FastInterruptDisableLock dLock;//prevent interrupts from interfering with the reding process
        
        // Clear sequence register [DOC 426]
        ADC1->SQR3 = 0;

        // Conversion of just one channel in a regular sequence
        ADC1->SQR3 |= (channel << 0);

        // Clear status register [DOC 418]
        ADC1->SR = 0;

        // start the conversion [DOC 421]
        ADC1->CR2 |= (1<<30); //SWSTART: Start conversion of regular channels

        //wait for the conversion to finish (EOC flag set by the HW) [DOC 418]
        while (!(ADC1->SR & (1<<1)));

        //Extract val from DataRegister and clear EOC flag
        return  (ADC1->DR);
    }
}
