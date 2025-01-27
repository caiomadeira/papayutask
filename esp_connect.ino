#include <ESP8266WiFi.h>
#include<time.h>
#include<SoftwareSerial.h>
#include<FS.h>

// Constants
// config do wifi
const char* SSID = "pachamama2G";
const char* PASSWORD = "962Bruto@";

// servidor ntp publico
const char* nptServer = "pool.ntp.org";
const long gmtOffset_sec = -10800; // Horario de brasilia: -3h = -10800 segunos

// Comunicacao serial c o arduino
SoftwareSerial espCommArd(4, 0);
// String receivedMessage; // pra guardar as respostas do arduino

WiFiServer server(80);

String firstFeed = "06:30";
String secondFeed = "17:30";
String extraFeed = "17:30";
//unsigned long lastFeedTime = 0;

String header;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void saveFeedTimes() {
  File file = SPIFFS.open("/firstFeed.txt", "w");
  if (file) {
    file.println(firstFeed);
    file.close();
    Serial.println("firstFeed salvo: " + firstFeed);
  }

  file = SPIFFS.open("/secondFeed.txt", "w");
  if (file) {
    file.println(secondFeed);
    file.close();
    Serial.println("secondFeed salvo: " + secondFeed);
  }

  file = SPIFFS.open("/extraFeed.txt", "w");
  if (file) {
    file.println(extraFeed);
    file.close();
    Serial.println("extraFeed salvo: " + extraFeed);
  }

  Serial.println("Horários atualizados no SPIFFS.");
  SPIFFS.end();  // Força a gravação dos arquivos antes de desligar
  SPIFFS.begin(); // Reinicia para evitar falha de acesso futuro
  Serial.println("Reiniciada.");
}

void loadFeedTimes() {
  if (SPIFFS.exists("/firstFeed.txt")) {
    File file = SPIFFS.open("/firstFeed.txt", "r");
    firstFeed = file.readString();
    firstFeed.trim();
    if (firstFeed.length() != 5) firstFeed = "06:30";
    file.close();
    Serial.println("firstFeed carregado: " + firstFeed);
  } else {
    Serial.println("Arquivo firstFeed não encontrado.");
  }

  if (SPIFFS.exists("/secondFeed.txt")) {
    File file = SPIFFS.open("/secondFeed.txt", "r");
    secondFeed = file.readString();
    secondFeed.trim();
    if (secondFeed.length() != 5) secondFeed = "17:30";
    file.close();
    Serial.println("secondFeed carregado: " + secondFeed);
  } else {
    Serial.println("Arquivo secondFeed não encontrado.");
  }

  if (SPIFFS.exists("/extraFeed.txt")) {
    File file = SPIFFS.open("/extraFeed.txt", "r");
    extraFeed = file.readString();
    extraFeed.trim();
    if (extraFeed.length() != 5) extraFeed = "17:30";
    file.close();
    Serial.println("extraFeed carregado: " + extraFeed);
  } else {
    Serial.println("Arquivo extraFeed não encontrado.");
  }
}

void setup() 
{
  Serial.begin(9600);
  espCommArd.begin(9600); // Comunicação serial com o Arduino
  Serial.println("ESP8266 - Serial pronto.");
  delay(100);

  Serial.printf("conectando ao wi-fi: %s\n", SSID);
  WiFi.begin(SSID, PASSWORD);

  while(WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".\n");
  }
  Serial.println("\nWifi conectado!");
  Serial.printf("IP do ESP8266: %s\n", WiFi.localIP().toString().c_str());

  // Inicia o SPIFFS
  if (!SPIFFS.begin()) {
    Serial.println("Erro ao montar o SPIFFS");
    ESP.restart();
    return;
  }
  Serial.println("SPIFFS montado com sucesso.");

  // Carregar horários salvos
  loadFeedTimes();

  // config ntp
  configTime(gmtOffset_sec, 0, nptServer);
  Serial.println("NTP configurado.");
  // Setup web
  server.begin();

  Serial.println("Listando arquivos no SPIFFS:");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) 
  {
    Serial.print("Arquivo encontrado: ");
    Serial.println(dir.fileName());
  }
}

// Código para obter o valor de um parâmetro na URL
String getValue(String parameter, String data) {
  int startIndex = data.indexOf(parameter + "=");
  if (startIndex == -1) {
    return "";
  }
  startIndex += parameter.length() + 1;
  int endIndex = data.indexOf("&", startIndex);
  if (endIndex == -1) {
    endIndex = data.indexOf(" ", startIndex);
  }
  return data.substring(startIndex, endIndex);
}


void loop() 
{
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if ((header.indexOf("GET /update?firstFeed") >= 0) || 
              header.indexOf("GET /update?secondFeed") >= 0 ||
              header.indexOf("GET /update?extraFeed") >= 0) {
            Serial.println("Caiu aqui passou dos ifs");
            firstFeed = getValue("firstFeed", header);
            secondFeed = getValue("secondFeed", header);
            extraFeed = getValue("extraFeed", header);
            // check if url params is empty
            String tempFirstFeed = getValue("firstFeed", header);
            if (tempFirstFeed != "") firstFeed = tempFirstFeed;

            String tempSecondFeed = getValue("secondFeed", header);
            if (tempSecondFeed != "") secondFeed = tempSecondFeed;

            String tempExtraFeed = getValue("extraFeed", header);
            if (tempExtraFeed != "") extraFeed = tempExtraFeed;

              // Salva os novos horários no SPIFFS imediatamente
            saveFeedTimes();
            
            defaultHttpHeaderContent(client);
            client.println("<h1>Horarios Atualizados!</h1>");
            client.println("<a href='/'>Voltar</a>");
            break;
          } else if (currentLine.length() == 0) {
            defaultHttpHeaderContent(client);
            client.println(GetHtmlPage());
            break;
          } else { currentLine = ""; }
        } else if (c != '\r') { currentLine += c; }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }

  struct tm timeInfo;
  if (!getLocalTime(&timeInfo))
  {
    Serial.println("Falha ao obter horário.");
    delay(1000);
    return;
  }
  
  char currentTime[9];
  snprintf(currentTime, sizeof(currentTime), "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);

  Serial.printf("Hora atual: %02d:%02d:%02d\n", timeInfo.tm_hour, timeInfo.tm_min, timeInfo.tm_sec);
  Serial.println("First feed: " + firstFeed + "; SecondFeed: " + secondFeed + "; Extra feed: " + extraFeed);
  firstFeed = formatTimeString(firstFeed);
  secondFeed = formatTimeString(secondFeed);
  extraFeed = formatTimeString(extraFeed);
  int counter = 0;
  String currentDay = String(timeInfo.tm_mday);
  Serial.println("Current Day: " + currentDay);
  Serial.println("First feed: " + firstFeed + "; SecondFeed: " + secondFeed + "; Extra feed: " + extraFeed);
  if ((firstFeed == String(currentTime)) || 
      (secondFeed == String(currentTime)) || 
      (extraFeed == String(currentTime))) 
  {
    Serial.println("Hora de alimentar!");
    espCommArd.println("1");
    delay(60000);
  } 
  delay(10000); // Atualiza a cada 10 segundos
}

// Função para substituir '%3A' por ':'
String formatTimeString(String s1) {
  String s2 = "";
  for (int i = 0; i < s1.length(); i++) {
    // Substitui o "%3A" por ":"
    if (i < s1.length() - 2 && s1[i] == '%' && s1[i+1] == '3' && s1[i+2] == 'A') {
      s2 += ":";  // Adiciona o ':' na nova string
      i += 2;  // Avança 2 posições para pular o "%3A"
    } else {
      s2 += s1[i];  // Adiciona o restante dos caracteres à nova string
    }
  }
  return s2;
}

void defaultHttpHeaderContent(WiFiClient client)
{
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
}

String GetHtmlPage()
{
  String htmlPage = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <link rel="icon" href="data:,">
    <style>
      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}
      .button { background-color: #195B6A; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}
      .button2 {background-color: #77878A;}
    </style>
  </head>
  <body>
    <h1>Papayuta5000</h1>
    <h4>v0.0.1</h4>
    <h2>Horarios:</h2>
    <form action="/update" method="get">
      <label for="firstFeed">Primeiro horario:</label>
      <input type="time" id="firstFeed" name="firstFeed" value="%FIRST_FEED%"><br><br>

      <label for="secondFeed">Segundo horario:</label>
      <input type="time" id="secondFeed" name="secondFeed" value="%SECOND_FEED%"><br><br>

      <label for="extraFeed">Terceiro horario (EXTRA):</label>
      <input type="time" id="extraFeed" name="extraFeed" value="%EXTRA_FEED%"><br><br>

      <input type="submit" value="Atualizar Horario" class="button">
    </form>
    
    <p> A Rori foi alimentada </p>
  </body>
  </html>
  )rawliteral";

  htmlPage.replace("%FIRST_FEED%", firstFeed);
  htmlPage.replace("%SECOND_FEED%", secondFeed);
  htmlPage.replace("%EXTRA_FEED%", extraFeed);

  return htmlPage;
}