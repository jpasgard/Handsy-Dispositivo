#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

// Constantes servidor
const char* SV_URL = "http://192.168.0.11:3000/node/1";
const int   SV_JSON_TAM = JSON_OBJECT_SIZE(2) + 10;

// Constantes estado da saida
const int ON = LOW;
const int OFF = HIGH;

// Constante saida
const int SAIDA = D1;

// Struct configurações de rede
struct configWifi {
    char nome[32];
    char senha[64];
};

// Declaração de variáveis
boolean estadoAtual = false;
boolean estadoServidor = false;

// Setup Inicial
void setup() {
    // Inicialização da saida
    pinMode(SAIDA, OUTPUT);
    digitalWrite(SAIDA, OFF);

    // Inicialização da porta Serial
    Serial.begin(115200);
    while (!Serial) {
        continue;
    }

    // Configuração rede WiFi
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
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
    if (!getEstadoServer(&estadoServidor)){
        return;
    }

    // Atualizando o estado da saida
    setEstadoSaidas();
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
    Serial.println(WiFi.SSID().c_str());
    Serial.print("\t  IP: ");
    Serial.println(WiFi.localIP());
}

/**
 * Pega o estado atual das saidas no servidor.
 * 
 * @param  estado ponteiro boolean que sera salvo o valor pego do servidor.
 * @return true caso consiga pegar os valores do servidor, false caso não consiga.
 */
boolean getEstadoServer(boolean *estado) {
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
    DynamicJsonDocument json(SV_JSON_TAM);
    DeserializationError error = deserializeJson(json, payload);
    if (error) {
        Serial.println("Falha durante desserializacao do JSON: ");
        return false;
    }

    // Extração de valores
    *estado = json["status"].as<bool>();

    // Disconectar
    http.end();
    client.stop();

    return true;
}

/**
 * Altera o estado da saida.
 */
void setEstadoSaidas() {
    if (estadoAtual != estadoServidor) {
        estadoAtual = estadoServidor;
        digitalWrite(SAIDA, estadoServidor ? ON:OFF);
    }
}
