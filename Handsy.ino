#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

// Constantes servidor
const char* SV_DOMINIO = "echo.jsontest.com";
const int   SV_PORTA = 80;
const char* SV_CAMINHO  = "/0/1/1/1";
const char* SV_HTTP_RESPOSTA = "HTTP/1.0";

// Constantes estado da saida
const int ON = LOW;
const int OFF = HIGH;

// Constantes saida
const int QTD_SAIDAS = 2;
const int SAIDAS[QTD_SAIDAS] = {D1, D2};
const int JSON_TAMANHO = JSON_OBJECT_SIZE(QTD_SAIDAS) + (QTD_SAIDAS * 2) + (QTD_SAIDAS * 1);

// Struct configurações de rede
struct configWifi {
    char nome[32];
    char senha[64];
};

// Declaração de variáveis
boolean estadoAtual[QTD_SAIDAS];

// Setup Inicial
void setup() {
    // Inicialização as saidas
    for (int i = 0; i < QTD_SAIDAS; i++) {
        pinMode(SAIDAS[i], OUTPUT);
        digitalWrite(SAIDAS[i], OFF);
    }

    // Inicialização da porta Serial
    Serial.begin(115200);
    while (!Serial) {
        continue;
    }

    // Configuração rede WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoConnect(true);
    configWifi config {
        "WiFi",
        "01234567"
    };

    // Conectar a rede WiFi
    if (!wiFiConectar(config)) {
        Serial.println("Erro ao conectar a rede WiFi");
        return;
    }
    wiFiPrint();
}

// Loop de execução principal
void loop() {
    delay(1000);

    if (!WiFi.isConnected()) {
        Serial.println("Erro na conexão com a rede WiFi");
        return;
    }

    // Buscando status no servidor
    boolean estadoServer[QTD_SAIDAS];
    if (!getEstadoServer(estadoServer)){
        return;
    }

    // Atulizando o estado nas saidas
    setEstadoSaidas(estadoServer);
}

/**
 * Inicia conexão com a rede Wifi.
 * 
 * @param config struct com configurações da rede WiFi.
 * @return true caso consiga conectar, false caso não consiga.
 */
boolean wiFiConectar(configWifi config) {
    WiFi.begin(config.nome, config.senha);
    while (true) {
        delay(500); // Aguarda 500 milésimos de segundos
        int status = WiFi.status();
        if (status == WL_CONNECTED) {
            return true;
        } else if ((status == WL_NO_SSID_AVAIL) || (status == WL_CONNECT_FAILED)) {
            return false;
        }
    }
}

/**
 * Imprime os dados da rede WiFi conectada.
 */
void wiFiPrint() {
    Serial.print("Conectado a rede WiFi\n\tSSID: ");
    Serial.println(WiFi.SSID());
    Serial.print("\t  IP: ");
    Serial.println(WiFi.localIP());
}

/**
 * Pega o estado atual das saidas no servidor.
 * 
 * @param estado vetor de boolean que sera salvo os valores pego do servidor.
 * @return true caso consiga pegar os valores do servidor, false caso não consiga.
 */
boolean getEstadoServer(boolean* estado) {
    // Conexão com servidor HTTP
    WiFiClient client;

    client.setTimeout(1000);
    if (client.connect(SV_DOMINIO, SV_PORTA)) {
        Serial.printf("Conectado ao servidor HTTP: %s:%d\n", SV_DOMINIO, SV_PORTA);
    } else {
        Serial.printf("Falha ao conectar ao servidor HTTP: %s:%d\n", SV_DOMINIO, SV_PORTA);
        return false;
    }

    // Envio da requisição HTTP
    client.printf("GET %s %s\n", SV_CAMINHO, SV_HTTP_RESPOSTA);
    client.printf("Host: %s\n", SV_DOMINIO);
    client.println("Connection: close");
    if (client.println() == 0) {
        Serial.println("Falha ao enviar requisicao HTTP");
        return false;
    }

    // Checagem do status HTTP
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
        if (strcmp(status, "HTTP/1.0 200 OK") != 0) {
        Serial.printf("Resposta inesperada: %s\n", status);
        return false;
    }

    // Pular cabeçalho HTTP
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
        Serial.println("Resposta invalida");
        return false;
    }

    // Desserialização do JSON
    DynamicJsonDocument json(JSON_TAMANHO);
    DeserializationError error = deserializeJson(json, client);
    if (error) {
        Serial.print("Falha durante desserializacao do JSON: ");
        Serial.println(error.c_str());
        return false;
    }

    // Extração de valores
    for (int i = 0; i < QTD_SAIDAS; i++) {
        char chave[2] = "0";
        chave[0] += i;
        estado[i] = json[chave].as<int>();
    }
    
    // Disconectar
    client.stop();

    return true;
}

/**
 * Altera o estado das saidas.
 * 
 * @param novoEstado vetor com os estados que vai ser alterado.
 */
void setEstadoSaidas(boolean* novoEstado) {
    for (int i = 0; i < QTD_SAIDAS; i++) {
        if (estadoAtual[i] != novoEstado[i]) {
            estadoAtual[i] = novoEstado[i];
            digitalWrite(SAIDAS[i], novoEstado[i] ? ON:OFF);
        }
    }
}
