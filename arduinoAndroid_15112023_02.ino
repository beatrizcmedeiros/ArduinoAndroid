#include <SPI.h>
#include <Ethernet.h>
#include <TimeLib.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 177);

EthernetServer server(80);

const int pinoSom = A0;
const int pinoBuzzer = 8;

boolean alarme = false;
time_t tempoAlarme = 0;

void setup() {
  Serial.begin(9600);
  pinMode(pinoSom, INPUT);
  pinMode(pinoBuzzer, OUTPUT);

  setTime(10, 4, 0, 15, 11, 2023);

  Ethernet.begin(mac, ip);
  server.begin();

  Serial.print("Server is at ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();

  if (client) {
    Serial.println("New client");

    boolean currentLineIsBlank = true;

    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);

        if (c == '\n' && currentLineIsBlank) {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");
          client.println("Refresh: 5"); // Atualização automática a cada 5 segundos
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");

          if (client.find("GET /?alarme=ativo") != -1) {
            alarme = true;
            tempoAlarme = now();
            emitirSomAlarme();
            client.println("<h1>Alarme Ativo</h1>");
            client.print("<p>Ativado em: ");
            client.print(hour(tempoAlarme));
            client.print(":");
            client.print(minute(tempoAlarme));
            client.print(":");
            client.print(second(tempoAlarme));
            client.print(" - ");
            client.print(day(tempoAlarme));
            client.print("/");
            client.print(month(tempoAlarme));
            client.print("/");
            client.print(year(tempoAlarme));
            client.println("</p>");
          } else if (client.find("GET /?alarme=inativo") != -1) {
            alarme = false;
            client.println("<h1>Alarme Inativo</h1>");
          }

          client.println("</html>");
          break;
        }

        if (c == '\n') {
          currentLineIsBlank = true;
        } else if (c != '\r') {
          currentLineIsBlank = false;
        }
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected");
  }

  // Detecção de som
  int valorPinoSom = analogRead(pinoSom);
  Serial.print("Valor lido no pinoSom (analógico): ");
  Serial.println(valorPinoSom);

  if (alarme && valorPinoSom > 45) {
    emitirSomAlarme();
  }

  delay(10000);
}

void emitirSomAlarme() {
  for (int i = 0; i < 2000; i++) {
    digitalWrite(pinoBuzzer, HIGH);
    delayMicroseconds(500);
    digitalWrite(pinoBuzzer, LOW);
    delayMicroseconds(500);
  }
}
