#include <AD9833.h>
#include <DS3231.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <digitalWriteFast.h>

#define LED 7

DS3231 clock;
RTCDateTime dt;
File arquivo;
char nomeArquivo[] = "LOG.TXT";

byte chipSelectPin = 4;  // Pino do cartão SD
String aux = "";

void setup() {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(115200);
  inicializaSD();
  listaArquivosSD();
  clock.begin();
  delay(100);
  // Configura relogio (rodar somente uma vez)
  clock.setDateTime(__DATE__, __TIME__);
}

// Escreve no arquivo a cada 5 segundos
void loop() {
  aux = getDateTime();
  writeToFile(nomeArquivo, aux);
  Serial.println(aux);
  delay(5000);
}

// Escreve no arquivo e mantém LED aceso p/ evitar
// corremper o cartao SD
void writeToFile(String nomeArq, String dataEHora) {
  digitalWrite(LED, HIGH);
  File arq = SD.open(nomeArq, FILE_WRITE);
  arq.println(dataEHora);
  delay(100);
  arq.close();
  digitalWrite(LED, LOW);
}

// Retorna string com data e hora
String getDateTime() {
  dt = clock.getDateTime();
  String resposta = "Utilizado em: ";
  resposta += dt.hour;
  resposta += ":";
  resposta += dt.minute;
  resposta += ":";
  resposta += dt.second;
  resposta += " ";
  resposta += dt.day;
  resposta += "-";
  resposta += dt.month;
  resposta += "-";
  resposta += dt.year;
  return resposta;
}

void inicializaSD() {
  if (!SD.begin(chipSelectPin)) {
    Serial.println("Falha na inicializacao");
    return;
  }
  Serial.println("Cartao inicializado com sucesso.");
}

// Rotina para listar na tela o conteudo do SD
void listaArquivosSD() {
  File root;
  root = SD.open("/");
  Serial.println("Arquivos no cartão SD:");
  printArquivos(root, 0);
  root.close();
}

// Escreve na serial nome dos arquivos que estao
// no cartao SD
void printArquivos(File dir, int numTabs) {
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial.println("/");
      printArquivos(entry, numTabs + 1);
    } else {
      Serial.print("\t\t");
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}