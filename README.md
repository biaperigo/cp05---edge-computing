# Checkpoint 05

## Vinheria Agnello: Monitoramento IoT com ESP32, MQTT e FIWARE

Edge Computing e Computer Systems — FIAP — Turma 1ESPV

## 1. Integrantes

- Beatriz de Araujo Périgo — RM 569654
- Guilherme Mazzini — RM 573183
- Luan Schinello Garbin — RM 571276
- Rafael Taboada Sobral — RM 569527

## 2. Descrição do projeto

A Vinheria Agnello precisa monitorar as condições de armazenamento dos seus vinhos, pois luminosidade, temperatura e umidade influenciam diretamente a qualidade da bebida. O ideal é manter o ambiente em penumbra, com temperatura próxima de 13 °C e umidade próxima de 70 %.

Este projeto simula, no Wokwi, um dispositivo IoT baseado em ESP32 que lê a temperatura e a umidade com o sensor DHT22 e a luminosidade com um módulo LDR. A cada leitura, o ESP32 inclui o timestamp (obtido por NTP, no fuso UTC-3) e envia os dados pela rede Wi-Fi, usando o protocolo MQTT, para a plataforma FIWARE. O dispositivo também recebe comandos pelo MQTT para ligar e desligar um LED, o que demonstra a comunicação nos dois sentidos. A leitura e a escrita dos dados são feitas pelo aplicativo MyMQTT.

## 3. Tecnologias utilizadas

ESP32 simulado no Wokwi, sensor DHT22, módulo LDR e LED com resistor. A comunicação usa MQTT com a biblioteca PubSubClient e o formato Ultralight 2.0, e os dados chegam ao FIWARE (IoT Agent MQTT e Orion Context Broker, padrão NGSIv2). O aplicativo MyMQTT, no iOS, é usado para monitorar e controlar o dispositivo. O código foi escrito em C++.

## 4. Funcionamento e tópicos MQTT

O ESP32 conecta-se à rede virtual do Wokwi, chamada `Wokwi-GUEST`, e ao broker MQTT `3.135.234.101`, na porta `1883`. O dispositivo é identificado como `device010` e usa o Client ID `fiware_010`.

As leituras são publicadas no tópico `/TEF/device010/attrs`. Os comandos são recebidos no tópico `/TEF/device010/cmd`, e a confirmação de cada comando é publicada em `/TEF/device010/cmdexe`.

As leituras são enviadas a cada 5 segundos no formato Ultralight 2.0, em que `t` é a temperatura em °C, `h` é a umidade em % e `l` é a luminosidade relativa de 0 a 100. Um exemplo de mensagem é `t|24.0|h|40.0|l|57|TimeInstant|2026-10-21T12:30:00-03:00`. O estado do LED é publicado como `s|on` ou `s|off`.

Para controlar o LED, publicamos `device010@on|` para ligar ou `device010@off|` para desligar no tópico de comandos.

## 5. Como replicar o projeto

### Simulação no Wokwi

1. Acesse o projeto pelo link da seção 8, ou crie um novo projeto ESP32 no Wokwi e copie os arquivos `sketch.ino`, `diagram.json` e `libraries.txt`.
2. Confira as ligações: DHT22 no GPIO 15, saída analógica (AO) do LDR no GPIO 34 e LED com resistor no GPIO 2, com o catodo no GND. Alimente os sensores com 3V3 e GND.
3. Confira se as bibliotecas `DHT sensor library` e `PubSubClient` estão em `libraries.txt` e clique em Play.
4. No monitor serial devem aparecer `Wi-Fi conectado!` e `Conectado ao broker MQTT!`. A cada 5 segundos surge uma linha `Publicado: ...` com as leituras e o timestamp.
5. Para testar, clique no DHT22 e no LDR dentro do Wokwi e altere os valores de temperatura, umidade e luminosidade.

### Configuração do MyMQTT

1. Abra o app e acesse a tela de configuração da conexão (Settings).
2. Preencha o broker `3.135.234.101` e a porta `1883`. Deixe usuário e senha vazios.
3. Na aba Subscribe, assine os tópicos `/TEF/device010/attrs` e `/TEF/device010/cmdexe`. As leituras e o estado do LED passam a chegar no app.
4. Na aba Publish, use o tópico `/TEF/device010/cmd` e envie `device010@on|` ou `device010@off|`.
5. O LED acende ou apaga no Wokwi, o monitor serial mostra `Mensagem recebida` e a confirmação chega no tópico `cmdexe`.

## 6. Alinhamento com Smart Data Models (NGSIv2)

Os dados seguem o padrão NGSIv2 do FIWARE. O ESP32 envia as medidas com nomes curtos (`t`, `h` e `l`), e o IoT Agent as converte em atributos de uma entidade NGSIv2, inspirada no modelo WeatherObserved do Smart Data Models (smartdatamodels.org). A temperatura corresponde a `temperature`, a umidade a `relativeHumidity`, a luminosidade a `illuminance` e o timestamp a `TimeInstant`. A luminosidade enviada é uma escala relativa de 0 a 100 obtida do LDR, e não um valor em lux. Esse mapeamento é configurado no cadastro do dispositivo no IoT Agent, que faz parte do back-end FIWARE.

## 7. Arquivos do projeto

- `sketch.ino`: código-fonte do ESP32.
- `diagram.json`: diagrama e conexões dos componentes no Wokwi.
- `libraries.txt`: bibliotecas utilizadas.
- `README.md`: documentação do projeto.

## 8. Links

- Simulação no Wokwi: link do projeto.
- Vídeo de demonstração: inserir o link do vídeo.
