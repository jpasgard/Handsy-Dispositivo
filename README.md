# Dispositivo Handsy

Handsy é um dispositivo com o intuito de facilitar a vida das pessoas. Como forma de interação o dispositivo é controlado remotamente através do aplicação mobile [Handsy](https://github.com/nitaicharan/APP-Handsy).
Handsy foi criado utilizando um NodeMCU, e um relé para acionar dispositivos.

Handsy funciona através do estilo de arquitetura *Representational State Transfer (REST)* através de requisições a servidores WEB para atualizar os estados dos dispositivos relé (portas) gerenciados pelo componente NodeMCU.

Requisições são realizadas através do verbo `GET` do protocolo *HTTP* como o exemplo abaixo:
```
http://<domínio>:<porta>/<caminho>/<idnode>
```
A resposta da requisição é em formado JSON como o exemplo a baixo:
```json
{
  "id": 1,
  "0": true,
  "1": false
}
```
### Dependências
   - [ESP8266 Community](https://www.filipeflop.com/blog/programar-nodemcu-com-ide-arduino/)
   - [ArduinoJson v6](https://arduinojson.org/v6/doc/installation/)
