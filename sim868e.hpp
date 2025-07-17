#pragma once
#include "stm32f4xx_hal.h"  // Подключение HAL для STM32 
#include <string>
#include <vector>

class SIM868E {
private:
    UART_HandleTypeDef* huart;  // Указатель на UART (через него идёт обмен с SIM868E)
    GPIO_TypeDef* powerPort;    // Порт для управления питанием (GPIO)
    uint16_t powerPin;          // Пин для включения/выключения модуля
//    GPIO_TypeDef* pwrkeyPort;    // Порт для управления power key (GPIO)
//    uint16_t pwrkeyPin;          // Пин для управления power key
    
    // Внутренние методы
    bool sendATCommand(const std::string& command, std::string& response, uint32_t timeout = 1000);
    bool waitForResponse(const std::string& expected, uint32_t timeout);

public:
    // Конструктор (передаём UART и пин управления питанием)
    SIM868E(UART_HandleTypeDef* huart, GPIO_TypeDef* powerPort, uint16_t powerPin);//, uint16_t pwrkeyPin);//, GPIO_TypeDef* pwrkeyPort);

    // Основные методы
    bool init();                            // Инициализация модуля
    bool sendSMS(const std::string& phone, const std::string& text);  // Отправка SMS
    bool httpGet(const std::string& url);   // GET-запрос через GPRS
    void powerOn();                         // Включение модуля
    void powerOff();                        // Выключение модуля
};