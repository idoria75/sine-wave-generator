#include <AD9833.h>
#include <DS3231.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <digitalWriteFast.h>

DS3231 clock;
RTCDateTime dt;
File arquivo;
char nomeArquivo[] = "LOG.TXT";

const int valoresParaLer = 20;
byte chipSelectPin = 4;             // Pino do cartão SD
float valorLido = 0;                // Variável que armazena último valor lido
float frequencias[valoresParaLer];  // Lista das frequencias lidas
float duracoes[valoresParaLer];     // Lista das duracoes lidas
int valoresLidos = 0;

void setup() {
  Serial.begin(115200);
  inicializaSD();
  listaArquivosSD();
  lerArquivoCartaoSD(nomeArquivo);
  printValoresLidos();
  // Initialize DS3231
  Serial.println("Initialize DS3231");
  clock.begin();
  delay(100);
  // Set sketch compiling time
  // clock.setDateTime(__DATE__, __TIME__);
}

void loop() {
  printClockData();
  delay(1000);
}

void printClockData() {
  dt = clock.getDateTime();

  Serial.print("Raw data: ");
  Serial.print(dt.year);
  Serial.print("-");
  Serial.print(dt.month);
  Serial.print("-");
  Serial.print(dt.day);
  Serial.print(" ");
  Serial.print(dt.hour);
  Serial.print(":");
  Serial.print(dt.minute);
  Serial.print(":");
  Serial.print(dt.second);
  Serial.println("");
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

// Rotina para ler ate certo caracter
float readUntilChar(File f, char delimitador) {
  char charRead;
  String valorEmString = "";

  // Verifica final do arquivo
  if (!(f.available())) {
    return -1;
  }

  while (f.available()) {
    char caractAtual = f.read();
    if (caractAtual == delimitador) {
      // Serial.println("-");
      // Serial.println(valorEmString.toFloat());
      return valorEmString.toFloat();
    } else {
      valorEmString += caractAtual;
    }
  }
}

// Rotina que pula linhas que comecam com #
void pulaLinhaComentario(File f) {
  char a;
  while (a != '\n') {
    a = f.read();
  }
}

void lerArquivoCompleto(String nomeDoArquivo) {
  File arq = SD.open(nomeDoArquivo);
  Serial.print("Arquivo: ");
  Serial.println(nomeDoArquivo);
  Serial.println(arq.available());
  while (arq.available() > 0) {
    char c = arq.read();
    Serial.print(c);
    delay(20);
  }
  arq.close();
}

// Funcao que le dados de frequencia e periodo
// e salva nas respectivas listas
void lerArquivoCartaoSD(String nomeDoArquivo) {
  Serial.print("Lendo arquivo: ");
  Serial.println(nomeDoArquivo);
  File arq = SD.open(nomeDoArquivo);
  // Serial.println(arq.available());
  if (arq) {
    while (arq.peek() == '#') {
      pulaLinhaComentario(arq);
    }
    while ((valorLido != -1) and (valoresLidos < valoresParaLer)) {
      valorLido = readUntilChar(arq, ',');
      // Serial.print("Valor lido freq: ");
      // Serial.println(valorLido);
      if (valorLido == -1) {
        break;
      } else {
        frequencias[valoresLidos] = valorLido;
        valorLido = readUntilChar(arq, '\n');
        // Serial.print("Valor lido periodo: ");
        // Serial.println(valorLido);
        duracoes[valoresLidos] = valorLido;
        valoresLidos++;
      }
    }
    arq.close();
  } else {
    Serial.println("Problema no arquivo!");
    Serial.print("Nome do arquivo informado: ");
    Serial.println(nomeDoArquivo);
    arq.close();
    return;
  }
}

// Imprime na tela os valores armazenados nas listas
void printValoresLidos() {
  for (int i = 0; i < valoresLidos; i++) {
    Serial.print(frequencias[i]);
    Serial.print(",");
    Serial.println(duracoes[i]);
  }
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