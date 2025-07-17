Класс для работы с SIM868E

Пример использования:

Назанчит пины:
#define GPIO_PORT  GPIOD;          // pwrkey
#define PWRKEY_PIN GPIO_PIN_10;   // pwrkey
    
#define POWER_GPIO_PORT GPIOD        // VBAT
#define POWER_PIN       GPIO_PIN_11  // VBAT


SIM868E sim(&huart3, POWER_GPIO_PORT, POWER_PIN);
sim.powerOn();
if (sim.init()) {
        sim.sendSMS("+79170216159", "Hello from STM32!");
        sim.httpGet("http://example.ru/api/data");
}
