#include <EtherCard.h>
#include <EEPROM.h>

#define LED1     4
#define LED2     5
#define RELE     6
#define ALARMA   7

const char website[] PROGMEM = "gamenrick.github.io"; // Cambia esto por tu usuario real
byte Ethernet::buffer[700];
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

unsigned long tiempoAnterior = 0;
const unsigned long intervalo = 15000; // 15 segundos

void setup() {
  Serial.begin(9600);
  Serial.println(F("Iniciando Ethernet..."));

  if (ether.begin(sizeof Ethernet::buffer, mymac, 53) == 0) {
    Serial.println(F("Fallo inicialización Ethernet"));
    while (1);
  }

  if (!ether.dhcpSetup()) {
    Serial.println(F("Fallo DHCP"));
    while (1);
  }

  Serial.println(F("Ethernet listo"));
  Serial.println(ether.myip[0]); Serial.print(".");
  Serial.print(ether.myip[1]); Serial.print(".");
  Serial.print(ether.myip[2]); Serial.print(".");
  Serial.println(ether.myip[3]);

  ether.printIp("Servidor DNS:", ether.dnsip);
  ether.printIp("IP local:", ether.myip);
  ether.printIp("Gateway:", ether.gwip);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(RELE, OUTPUT);
  pinMode(ALARMA, OUTPUT);
}

void loop() {
  ether.packetLoop(ether.packetReceive());

  unsigned long ahora = millis();
  if (ahora - tiempoAnterior >= intervalo) {
    tiempoAnterior = ahora;

    Serial.println(F("Consultando estado desde GitHub..."));

    if (ether.dnsLookup(website)) {
      ether.printIp("IP de GitHub:", ether.hisip);

      ether.browseUrl(PSTR("/Arduino_10_SE/"), "estado.txt", website, callback);
    } else {
      Serial.println(F("Error al resolver DNS"));
    }
  }
}

void callback(byte status, word off, word len) {
  if (status == 0) {
    Serial.println(F("Fallo conexión HTTP"));
    return;
  }

  char* body = (char*) Ethernet::buffer + off;
  Serial.println(F("Respuesta recibida:"));
  Serial.println(body);

  // Buscar y aplicar los estados
  aplicarEstado("LED1", LED1, body);
  aplicarEstado("LED2", LED2, body);
  aplicarEstado("RELE", RELE, body);
  aplicarEstado("ALARMA", ALARMA, body);
}

void aplicarEstado(const char* nombre, uint8_t pin, const char* cuerpo) {
  char linea[30];
  snprintf(linea, sizeof(linea), "%s=on", nombre);
  if (strstr(cuerpo, linea)) {
    digitalWrite(pin, HIGH);
    Serial.print(nombre); Serial.println(" ON");
    return;
  }

  snprintf(linea, sizeof(linea), "%s=off", nombre);
  if (strstr(cuerpo, linea)) {
    digitalWrite(pin, LOW);
    Serial.print(nombre); Serial.println(" OFF");
    return;
  }
}
