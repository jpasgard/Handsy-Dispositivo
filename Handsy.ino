#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Constantes servidor
const char* SV_URL = "http://192.168.0.11:3000/node/1";

// Constantes estado da saida
const int ON = LOW;
const int OFF = HIGH;

// Constantes saida
const int QTD_SAIDAS = 2;
const int SAIDAS[QTD_SAIDAS] = {D1, D2};
const int JSON_TAMANHO = JSON_OBJECT_SIZE(QTD_SAIDAS + 1) + (QTD_SAIDAS * 3) + QTD_SAIDAS + 1;

// Struct configurações de rede
struct configWifi {
    char nome[32];
    char senha[64];
};

// Declaração de variáveis
boolean estadoAtual[QTD_SAIDAS] = {OFF};

// Setup Inicial
void setup() {
    // Inicialização as saidas
    for (int i = 0; i < QTD_SAIDAS; i++) {
        pinMode(SAIDAS[i], OUTPUT);
    }
    setEstadoSaidas(estadoAtual);

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
    WiFiClient client;
    HTTPClient http;

    // Conexão com servidor HTTP
    client.setTimeout(1000);
    boolean conexao = http.begin(client, SV_URL);
    if (conexao) {
        Serial.println("Conectado ao servidor");
    } else {
        Serial.println("Falha ao conectar ao servidor");
        return false;
    }

    // Checagem do status HTTP
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        return false;
    }

    // Desserialização do JSON
    String payload = http.getString();
    DynamicJsonDocument json(JSON_TAMANHO);
    DeserializationError error = deserializeJson(json, payload);
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
    http.end();
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
