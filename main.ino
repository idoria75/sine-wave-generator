#include <AD9833.h>
#include <DS3231.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <digitalWriteFast.h>

#define LED 7
#define FNC_PIN 9
#define FNC_PIN2 10

DS3231 clock;
RTCDateTime dt;
File arquivo;
char nomeArquivo[] = "LOG.TXT";

String aux = "";
const int valoresParaLer = 15;      // Quantos pares de (f,T) serão lidos
byte chipSelectPin = 4;             // Pino do cartão SD
float valorLido = 0;                // Variável que armazena último valor lido
float frequencias[valoresParaLer];  // Lista das frequencias lidas
float duracoes[valoresParaLer];     // Lista das duracoes lidas
int valoresLidos = 0;               // Contador de valores lidos

unsigned long tempoInicioOnda = 0;
// Inicializa AD9833
AD9833 gen(FNC_PIN);
AD9833 gen2(FNC_PIN2);

void setup() {
  // gen.Begin() DEVE ser o primeiro comando a ser executado
  gen.Begin();
  gen2.Begin();
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  Serial.begin(115200);
  inicializaSD();
  listaArquivosSD();
  lerArquivoCartaoSD(nomeArquivo);
  printValoresLidos();

  clock.begin();
  delay(100);
  // Configura relogio (rodar somente uma vez)
  // clock.setDateTime(__DATE__, __TIME__);

  for (int i = 0; i < valoresLidos; i++) {
    geraOndaSenoidal(frequencias[i], duracoes[i]);
  }
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
void geraOndaSenoidal(float frequencia, float periodo) {
  Serial.print("Gerando Onda Senoidal -> ");
  Serial.print("Frequencia: ");
  Serial.print(frequencia);
  Serial.print(", ");
  Serial.print("Periodo: ");
  Serial.println(periodo);

  // Atualizar interface aqui

  // Valor de frequencia p/ AD9833
  unsigned long freq_ad = frequencia * 100;
  // Valor de periodo p/ habilitar senoide
  int tempo = periodo * 1000;

  // Envia dados ao AD9833
  gen.ApplySignal(SINE_WAVE, REG0, freq_ad);
  gen2.ApplySignal(SINE_WAVE, REG0, freq_ad - 783);
  gen.EnableOutput(true);
  gen2.EnableOutput(true);
  // Variavel auxiliar para controle do tempo
  unsigned long tempoAtual = millis();  // = 0
  // Armazena tempo atual
  tempoInicioOnda = millis();

  // Enquanto tempo transcorrido for menor do que
  // o desejado para a onda
  while (millis() < tempoInicioOnda + tempo) {
    // Limita dados exibidos na tela
    if (millis() > tempoAtual + 1000) {
      Serial.println(millis() - tempoInicioOnda);
      // Atualiza tempo atual
      tempoAtual = millis();
    }
  }
  // Desliga saida do AD9833
  gen.EnableOutput(false);
  gen2.EnableOutput(false);
  // Atualiza tempo atual
  tempoAtual = millis();

  // Imprime na tela a duracao real da onda
  Serial.print("Tempo decorrido (millis): ");
  Serial.println(tempoAtual - tempoInicioOnda);

  // Atualizar interface aqui
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

// Rotina para iniciar corretamente o cartao SD
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