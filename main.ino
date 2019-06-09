#include <AD9833.h>
#include <DS3231.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <digitalWriteFast.h>
#include <util/atomic.h>

#define LED 7
#define FNC_PIN 9
#define FNC_PIN2 10

DS3231 clock;
RTCDateTime dt;
File arquivo;
char arquivoEntrada[] = "FREQS3.TXT";
char arquivoSaida[] = "LOG3.TXT";
String aux = "";
const byte valoresParaLer = 10;     // Quantos pares de (f,T) serão lidos
const byte chipSelectPin = 4;       // Pino do cartão SD
float valorLido = 0;                // Variável que armazena último valor lido
float frequencias[valoresParaLer];  // Lista das frequencias lidas
int duracoes[valoresParaLer];       // Lista das duracoes lidas
byte valoresLidos = 0;              // Contador de valores lidos

unsigned long tempoInicioOnda = 0;

boolean nivelLogico = true;
const byte isochonicPin = 9;

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
  lerArquivoCartaoSD(arquivoEntrada);
  printValoresLidos();

  clock.begin();
  delay(100);
  // Configura relogio (rodar somente uma vez)
  // clock.setDateTime(__DATE__, __TIME__);
  pinMode(isochonicPin, OUTPUT);
  configuraTimer1();

  for (int i = 0; i < valoresLidos; i++) {
    aux = getDateTime();
    geraOndaSenoidal(frequencias[i], duracoes[i]);
    Serial.println("SD->Hora");
    writeToFile(arquivoSaida, aux);
    Serial.println("SD->Data");
    writeToFile(arquivoSaida, frequencias[i], duracoes[i]);
  }
}

void loop() {}

// Escreve no arquivo e mantém LED aceso durante escrita
// p/ evitar corremper o cartao SD
void writeToFile(String nomeArq, String data) {
  digitalWrite(LED, HIGH);
  File arq = SD.open(nomeArq, FILE_WRITE);
  arq.println(data);
  delay(100);
  arq.close();
  digitalWrite(LED, LOW);
}

// Escreve no arquivo e mantém LED aceso durante escrita
// p/ evitar corremper o cartao SD
void writeToFile(String nomeArq, float f1, int f2) {
  digitalWrite(LED, HIGH);
  File arq = SD.open(nomeArq, FILE_WRITE);
  arq.print(f1);
  arq.print(", ");
  arq.println(f2);
  delay(100);
  arq.close();
  digitalWrite(LED, LOW);
}

// Gera nos ADs as senoides desejadas
void geraOndaSenoidal(float frequencia, float periodo) {
  Serial.print("Freq ");
  Serial.print(frequencia);
  Serial.print(", ");
  Serial.print("Per: ");
  Serial.println(periodo);

  // Atualizar interface aqui

  // Valor de frequencia p/ AD9833
  unsigned long freq_ad = frequencia * 100;
  // Valor de periodo p/ habilitar senoide
  unsigned long tempo = periodo * 1000;

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
    delay(10);
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

ISR(TIMER1_COMPA_vect) {
  digitalWrite(isochonicPin, nivelLogico);
  nivelLogico = !nivelLogico;
}

void configuraTimer1() {
  // Desabilita interrupcoes
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  // 7.83 Hz (3.915Hz p/ cada nível lógico):
  OCR1A = 63856;
  // Modo CTC:
  TCCR1B |= (1 << WGM12);
  // Prescaler 64:
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS11);
  TCCR1B |= (0 << CS12);
  // Habilita CTC:
  TIMSK1 |= (1 << OCIE1A);
  // Habilita interrupcoes
  sei();
}