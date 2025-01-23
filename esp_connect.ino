#include <ESP8266WiFi.h>
#include<time.h>
#include<SoftwareSerial.h>

SoftwareSerial espCommArd(4, 0);
String receivedMessage;

// config do wifi
const char* ssid = "pachamama2G";
const char* password = "962Bruto@";

// servidor ntp publico
const char* nptServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // Horario de brasilia: -3h = -10800 segunos

// FOOD HOURS
const char* firstFeed = "06:30:00";
const char* secondFeed = "17:30:00";
const char* testFeed = "22:30";

void setup() 
{
  Serial.begin(9600);
  espCommArd.begin(9600); // Comunicação serial com o Arduino
  Serial.println("ESP8266 - Serial pronto.");
  delay(100);

  Serial.printf("conectando ao wi-fi: %s\n", ssid);
  WiFi.begin(ssid, password);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print("Waiting....\n");
  }
  Serial.println("\nWifi conectado!");
  Serial.printf("IP do ESP8266: %s\n", WiFi.localIP().toString().c_str());

  // config ntp
  configTime(gmtOffset_sec, 0, nptServer);
  Serial.println("NTP configurado.");

}

void loop() 
{
  struct tm timeInfo;
  if (!getLocalTime(&timeInfo))
  {
    Serial.println("Falha ao obter horário.");
    delay(1000);
    return;
  }
  
  char currentTime[9];
  snprintf(currentTime, sizeof(currentTime), "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);

  // Exibe o horário no Monitor Serial
  Serial.printf("Data: %02d/%02d/%04d\n", timeInfo.tm_mday, timeInfo.tm_mon + 1, timeInfo.tm_year + 1900);
  Serial.printf("Hora: %02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);

  if (strcmp(currentTime, testFeed) == 0)
  {
    Serial.println("Hora de alimentar!");
    // manda msg pro arduino
    espCommArd.printf("rosita123;%02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  }
  checkFeedbackFromArduino(); // nao sei se vou manter pois n ha resistores no circuito
  delay(10000); // Atualiza a cada 10 segundos
}

void checkFeedbackFromArduino()
{
    if (espCommArd.available())
    {
      receivedMessage = espCommArd.readString();
      Serial.println("Feedback do arduino: ");
      Serial.println(receivedMessage);
      Serial.println("\n");
    }
}