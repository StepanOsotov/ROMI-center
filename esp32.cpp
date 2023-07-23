// Подключем библиотеки
#include <WiFi.h>
#include <AsyncUDP.h>
#include <WiFiAP.h>

#define AMOUNT_LED  3

// Определяем вывод светодиода
const uint8_t LED_PIN[AMOUNT_LED] = {21, 22, 23};
uint8_t led_power[AMOUNT_LED] = {0, 0, 0};

bool isConfigIP = false;

// Определяем название и пароль точки доступа
const char* ssid = "udp_server";
const char* password = "1234qwer";

IPAddress ip(172, 16, 8, 23);  //статический IP
IPAddress gateway(172, 16, 8, 1);
IPAddress subnet(255, 255, 255, 0);

// Создаём объект UDP cоединения
AsyncUDP udp;

// Определяем порт
const uint16_t port = 49152;

void toggleLeds(void)
{
  static uint8_t count = 0x00;
  static uint8_t toggle = 0x01;

  if( (count == 0) || (count == 1) )
  {
    digitalWrite(LED_PIN[0], toggle);
    digitalWrite(LED_PIN[1], 1);
    digitalWrite(LED_PIN[2], 1);
  }
  else if( (count == 2) || (count == 3) )
  {
    digitalWrite(LED_PIN[1], toggle);
    digitalWrite(LED_PIN[0], 1);
    digitalWrite(LED_PIN[2], 1);
  }
  else if( (count == 4) || (count == 5) )
  {
    digitalWrite(LED_PIN[2], toggle);
    digitalWrite(LED_PIN[1], 1);
    digitalWrite(LED_PIN[0], 1);
  }

  for(uint8_t i = 0 ; i < AMOUNT_LED; i++)
  {
    led_power[i] = digitalRead(LED_PIN[i]);
  }

  toggle ^= 0x01;
  count++;
  if( (count == 6) )
  {
    count = 0;
  }
}


// Определяем callback функцию обработки пакета
void parsePacket(AsyncUDPPacket packet)
{
  // Записываем адрес начала данных в памяти
  const uint8_t* msg = packet.data();
  // Записываем размер данных
  const size_t len = packet.length();
  // Объект для хранения состояния светодиода в строковом формате
  String led_states;
  String strMessage = "UDP Rx : ";
  strMessage += (char *)msg;

  Serial.println(strMessage);

  if(!memcmp((const char *)msg, "exit", 4))
  {
    digitalWrite(LED_PIN[0], 0x01);
    digitalWrite(LED_PIN[1], 0x01);
    digitalWrite(LED_PIN[2], 0x01);
    return;
  }

  toggleLeds();

  led_states = "{ ";

  for(uint8_t i = 0 ; i < AMOUNT_LED; i++)
  {
    led_states +=  led_power[i] ? "1" : "0";
    led_states += ", ";
  }
  led_states += "}";

  strMessage = "Leds is " + led_states + '\n';

  // Отправляем данные клиенту
  packet.printf(strMessage.c_str());

  // Выводим состояние светодиода в последовательный порт
  Serial.println(strMessage);

}

void setup()
{
  // Устанавливаем режим работы вывода светодиода
  pinMode(LED_PIN[0], OUTPUT);
  pinMode(LED_PIN[1], OUTPUT);
  pinMode(LED_PIN[2], OUTPUT);

  uint8_t toggle = 0x00;
  uint8_t i;
  uint16_t delay_led = 100;
  
  digitalWrite(LED_PIN[0], 1);
  digitalWrite(LED_PIN[1], 1);
  digitalWrite(LED_PIN[2], 1);

  for(i = 0; i < 10; i++)
  {
    digitalWrite(LED_PIN[0], toggle);
    sys_delay_ms(delay_led);
    toggle ^= 0x01;
  }
  digitalWrite(LED_PIN[0], 1);
  toggle = 0x00;
  for(i = 0; i < 10; i++)
  {
    digitalWrite(LED_PIN[1], toggle);
    sys_delay_ms(delay_led);
    toggle ^= 0x01;
  }
  digitalWrite(LED_PIN[1], 1);
  toggle = 0x00;
  for(i = 0; i < 10; i++)
  {
    digitalWrite(LED_PIN[2], toggle);
    sys_delay_ms(delay_led);
    toggle ^= 0x01;
  }
  digitalWrite(LED_PIN[2], 1);

  // Инициируем последовательный порт
  Serial.begin(115200);
  Serial.println();
  Serial.println("Initiating the access point (or hotspot) WiFi");

//---------------------------------------------------------------------------------

  WiFi.mode(WIFI_AP_STA);

  isConfigIP = WiFi.softAPConfig(ip, gateway, subnet);

  if(isConfigIP)
  {
    Serial.println("soft AP config = true");
    WiFi.softAP(ssid, password);
    ip = WiFi.softAPIP();
    WiFi.config(ip, gateway, subnet);
  }
  else
  {
    Serial.println("soft AP config = false");
    return;
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP().toString());   //You can get IP address assigned to ESP

  // Инициируем сервер
  if(udp.listen(port))
  {
    // При получении пакета вызываем callback функцию
    udp.onPacket(parsePacket);
  }

  Serial.println("The server is running.");
}

void loop()
{
  //sys_delay_ms(500);
  sys_delay_ms(10);
  //udp.broadcast("Anyone here?");
}