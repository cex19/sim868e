#include "sim868e.hpp"
#include <cstring>

// Конструктор
SIM868E::SIM868E(UART_HandleTypeDef* huart, GPIO_TypeDef* powerPort, uint16_t powerPin)//, uint16_t pwrkeyPin)//, GPIO_TypeDef* pwrkeyPort) 
    : huart(huart), powerPort(powerPort), powerPin(powerPin){}//, pwrkeyPin(pwrkeyPin) //,  pwrkeyPort(pwrkeyPort) {}
 
// Включение модуля (удержание POWER на 1.5 сек)
void SIM868E::powerOn() {
    HAL_GPIO_WritePin(powerPort, powerPin, GPIO_PIN_RESET);
    HAL_Delay(1500);
    HAL_GPIO_WritePin(powerPort, powerPin, GPIO_PIN_SET);
    HAL_Delay(3000);  // Ждём инициализации
}

// Выключение модуля
void SIM868E::powerOff() {
    std::string response;
    sendATCommand("AT+CPOWD=1", response, 2000);  // Корректное отключение
}
// Отправка AT-команды и получение ответа
bool SIM868E::sendATCommand(const std::string& command, std::string& response, uint32_t timeout) {
    response.clear();
    std::string cmd = command + "\r\n";
    HAL_UART_Transmit(huart, (uint8_t*)cmd.c_str(), cmd.length(), timeout);

    uint8_t buffer[128];
    uint32_t startTime = HAL_GetTick();
    while (HAL_GetTick() - startTime < timeout) {
        if (HAL_UART_Receive(huart, buffer, sizeof(buffer) - 1, 100) == HAL_OK) {
            buffer[sizeof(buffer) - 1] = '\0';
            response += (char*)buffer;
            if (response.find("READY") != std::string::npos || response.find("OK") != std::string::npos || 
                response.find("ERROR") != std::string::npos) {
                break;
            }
        }
    }
    return response.find("OK");
}

// Инициализация модуля
bool SIM868E::init() {
    std::string response;
    if (!sendATCommand("AT", response)) return false;  // Проверка связи
    if (!sendATCommand("ATE0", response)) return false; // Отключение эха
    //if (!sendATCommand("AT+CPIN?", response)) return false; // Проверка SIM-карты на PIN 
    return true;
}


bool SIM868E::waitForResponse(const std::string& expected, uint32_t timeout) {
    uint32_t startTime = HAL_GetTick();
    std::string response;
    uint8_t buffer[128] = {0};
    
    while (HAL_GetTick() - startTime < timeout) {
        if (HAL_UART_Receive(huart, buffer, sizeof(buffer) - 1, 100) == HAL_OK) {
            buffer[sizeof(buffer) - 1] = '\0';  // Гарантируем null-terminated строку
            response += reinterpret_cast<char*>(buffer);
            
            // Проверяем наличие ожидаемого ответа
            if (response.find(expected) != std::string::npos) {
                return true;
            }
            
            // Проверяем ошибку (можно добавить другие коды ошибок)
            if (response.find("ERROR") != std::string::npos) {
                return false;
            }
        }
        HAL_Delay(10);  // Небольшая задержка для уменьшения нагрузки на CPU
    }
    return false;  // Таймаут
}

// Отправка SMS
bool SIM868E::sendSMS(const std::string& phone, const std::string& text) {
    std::string response;
    if (!sendATCommand("AT+CMGF=1", response)) return false; // Режим текстовой SMS
    if (!sendATCommand("AT+CMGS=\"" + phone + "\"", response)) return false;
    HAL_UART_Transmit(huart, (uint8_t*)text.c_str(), text.length(), 1000);
    HAL_UART_Transmit(huart, (uint8_t*)"\x1A", 1, 1000); // Ctrl+Z для отправки
    return waitForResponse("+CMGS:", 5000);
}

// HTTP GET-запрос
bool SIM868E::httpGet(const std::string& url) {
    std::string response;
    if (!sendATCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"", response)) return false;
    if (!sendATCommand("AT+SAPBR=3,1,\"APN\",\"internet\"", response)) return false;
    if (!sendATCommand("AT+SAPBR=1,1", response, 3000)) return false; // Активация GPRS
    if (!sendATCommand("AT+HTTPINIT", response)) return false;
    if (!sendATCommand("AT+HTTPPARA=\"URL\",\"" + url + "\"", response)) return false;
    if (!sendATCommand("AT+HTTPACTION=0", response, 10000)) return false; // GET-запрос
    return true;
}