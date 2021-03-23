#include <Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

//Configs Wifi
const char* WIFI_REDE = "FaustinoNet";
const char* SENHA_WIFI = "password";
bool modoOffline = false;

//Configs rede
IPAddress ip(192,168,15,80);
IPAddress gateway(192,168,15,1);
IPAddress subnet(255,255,255,0);

//Configs do sensor de temperatura e umidade
#define DHTPIN 0 //Pino digital do DHT22
#define DHTTYPE DHT22 //Modelo do sensor DHT22 ou AM2302
DHT dht(DHTPIN, DHTTYPE); //Inicia o sensor

//Wifi Server
WiFiServer server(80);

//Definições do display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Instancia o Wifi
WiFiClient espClient;

void setup(){

    //Monitor Serial
    Serial.begin(115200);

    //Inicializar o display LCD
    iniciarLCD();

    //Iniciar o wifi
    WiFi.disconnect();
    manterWifi();

    //Inicia o DHT22
    dht.begin();

    //Beep
    beep();
    
}

void iniciarLCD(){
    lcd.begin(); //Inicia o LCD
    lcd.backlight(); //Ligar a luz do fundo do LCD
}

void manterWifi(){

    //Verifica se já está conectado ou se está em modo offiline
    if(WiFi.status() == WL_CONNECTED || modoOffline == true){
        return;
    }

    //Iniciar o procedimento de desconexão
    WiFi.disconnect();
    
    //Conectar à rede WIFI
    escreveLCD(1,"Conectando...");
    escreveLCD(2,WIFI_REDE);
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(WIFI_REDE, SENHA_WIFI);

    //Timout da conexão
    int delay_wifi = 1000; //ms
    int timeout = delay_wifi * 15; //ms

    //Avalia se já está conectado ou timout foi atingido
    while(WiFi.status() != WL_CONNECTED && timeout > 0){
        timeout -= delay_wifi;
        delay(delay_wifi);
    }

    //Entrar no modo offline
    if(timeout <= 0){
        modoOffline = true;
        escreveLCD(1,"Wifi timeout!");
    } else {
        escreveLCD(1,"Conectado!!");
        escreveLCD(2, ipToString(WiFi.localIP()));
            
        //Iniciar o servidor
        server.begin();
    }

    delay(1000);

    lcd.clear();

}

void escreveLCD(int linha, String str){

    //Limpa a linha
    str = (String) str + "                ";
    
    int j=0;
    
    for(int i=0;i<16;i++){
        lcd.setCursor(j++,linha-1);
        if(((String)str[i]+str[i+1])=="º"){ //Correção para o caracter º
            lcd.print((char)223);
            i++;
        } else{
            lcd.print(str[i]);
        }
    }
    
}

String ipToString(IPAddress ip){
    String s = "";
    for (int i=0;i<4;i++)
        s += i ? "." + String(ip[i]) : String(ip[i]);
    return s;
}

void beep(){
    tone(15, 1000);
    delay(100);
    tone(15, 1500);
    delay(100);
    tone(15, 2000);
    delay(100);
    noTone(15);    
}

void lerDados(){

    char buffer[10];

    //Temperatura
    sprintf(buffer, "'%.1f'", dht.readTemperature());
    String temperatura = String(buffer);
    temperatura.replace("'","");

    //Umidade
    sprintf(buffer, "'%.1f'", dht.readHumidity());
    String umidade = String(buffer);
    umidade.replace("'","");

    //Escrever no LCD
    escreveLCD(1, "Temp: " + temperatura + " ºC");
    escreveLCD(2, "Umidade: " + umidade + " %");
    
}

void loop(){

    manterWifi();

    lerDados();

    WiFiClient client = server.available();
    if(client){

      String request = client.readStringUntil('\r');
      client.flush();

      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/json");
      client.println("");

      client.println("{");
      client.println("\"temperatura\": " + (String)dht.readTemperature() + ",");
      client.println("\"temperatura-unidade\": \"ºC\",");
      client.println("\"umidade\": " + (String)dht.readHumidity() + ",");
      client.println("\"umidade-unidade\": \"%\"");
      client.println("}");

      delay(1);
      
      return;
    }

    delay(1800);
    
}
