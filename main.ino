#include <SD.h>
#include <SPI.h>

// Valores para serem alterados de acordo com a execução desejada
char nomeArquivo[] = "SAUDE9.TXT";
const int valoresParaLer = 10;  // Quantos pares de (f,T) serão lidos

// Valores para funcionamento do programa:
File arquivo;  // Variável com o arquivo

byte chipSelectPin = 4;             // Pino do cartão SD
float valorLido = 0;                // Variável que armazena último valor lido
float frequencias[valoresParaLer];  // Lista das frequencias lidas
float duracoes[valoresParaLer];     // Lista das duracoes lidas
int valoresLidos = 0;               // Contador de valores lidos

void setup() {
  Serial.begin(115200);
  inicializaSD();
  listaArquivosSD();
  lerArquivoCartaoSD(nomeArquivo);
}

void loop() {}

void inicializaSD() {
  if (!SD.begin(chipSelectPin)) {
    Serial.println("Falha na inicializacao");
    return;
  }
  Serial.println("Cartao inicializado com sucesso.");
}

void listaArquivosSD() {
  File root;
  root = SD.open("/");
  Serial.println("Arquivos no cartão SD:");
  printArquivos(root, 0);
  root.close();
}

// funcao que le ate certo caracter
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

// Funcao que pula linhas que comecam com #
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
    for (int i = 0; i < valoresLidos; i++) {
      Serial.print(frequencias[i]);
      Serial.print(",");
      Serial.println(duracoes[i]);
    }
  } else {
    Serial.println("Problema no arquivo!");
    Serial.print("Nome do arquivo informado: ");
    Serial.println(nomeDoArquivo);
    arq.close();
    return;
  }
<<<<<<< HEAD
  delay(5);
  Serial.println(count);
=======
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
>>>>>>> 4f444814ce3688f74f442e24ee4ce7f140d72a51
}