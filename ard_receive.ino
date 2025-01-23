#include <Servo.h>

String receivedMessage;
#define SERVO_PIN 9

Servo mainServo;
bool feed = false;

void setup() {
  Serial.begin(9600);  // Inicializa a comunicação serial com o computador/ESP8266
  Serial.println("Arduino pronto para receber!");
  mainServo.attach(SERVO_PIN);
  delay(4000);
}

void loop() {
  // Verifica se há dados chegando da ESP8266
  if (Serial.available()) {
    receivedMessage = Serial.readString();  // Lê a mensagem
    receivedMessage.trim();
    if (strcmp(receivedMessage.c_str(), "1") == 0)
    {
        Serial.print("Recebido da ESP8266 e checado: ");
        Serial.println(receivedMessage);
        // Envia confirmação para a ESP8266
        //Serial.println("Mensagem recebida com sucesso!");
        feed = true;
        Serial.println("feed = true");
    } 
    else { 
      Serial.println("Mensagens não são iguais");
      Serial.print("received message: ");
      Serial.println(receivedMessage); // Correção aqui
    }

    if (feed == true)
    {
      MoveServo();
    }
  }
}

void MoveServo()
{
  Serial.println("[+] Abrindo a caixa de ração...");
  for(int i = 0; i <= 180; i++)
  {
    mainServo.write(i);
    delay(15);          // Aguarda 15ms para o servo se mover
  }
  Serial.println("[+] Caixa aberta!");
  delay(3000);
  Serial.println("[!] Fechando a caixa de ração...");
  for(int i = 180; i >= 0; i--)
  {
    mainServo.write(i);
    delay(15);
  }
  Serial.println("[!] Caixa fechada.");
  feed = false;
}
