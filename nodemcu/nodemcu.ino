#include <Wire.h>
#include <DHT.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>

//Configs Wifi
const char* WIFI_REDE = "FaustinoNet";
const char* SENHA_WIFI = "password";

IPAddress ip(192,168,15,80);
IPAddress gateway(192,168,15,1);
IPAddress subnet(255,255,255,0);

#define DHTPIN 0 //PINO DIGITAL UTILIZADO PELO DHT22
#define DHTTYPE DHT22 //DEFINE O MODELO DO SENSOR (DHT22 / AM2302)

//Wifi Server
WiFiServer server(80);

//Definições do display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

//Instancia o Wifi
WiFiClient espClient;

DHT dht(DHTPIN, DHTTYPE);

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
    lcd.begin();
    lcd.backlight(); //Ligar a luz do fundo do LCD
}

void manterWifi(){

    //Verifica se já está conectado
    if(WiFi.status() == WL_CONNECTED)
        return;

    //Iniciar o procedimento de desconexão
    WiFi.disconnect();
    
    //Conectar
    escreveLCD(1,"Conectando...");
    escreveLCD(2,WIFI_REDE);
    WiFi.config(ip, gateway, subnet);
    WiFi.begin(WIFI_REDE,SENHA_WIFI);

    while(WiFi.status() != WL_CONNECTED){
        delay(100);
    }

    escreveLCD(1,"Conectado!!");
    escreveLCD(2, ipToString(WiFi.localIP()));

    //Iniciar o servidor
    server.begin();

    delay(1000);

    lcd.clear();

}

void escreveLCD(int linha, String str){

    str = (String) str + "                ";
    
    int j=0;
    
    for(int i=0;i<16;i++){
        lcd.setCursor(j++,linha-1);
        if(((String)str[i]+str[i+1])=="º"){
            lcd.print((char)223);
            i++;
        } else
            lcd.print(str[i]);
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

void lerTemperatura(){

  escreveLCD(1, (String)"Temp: " + (String)dht.readTemperature() + "ºC");
  escreveLCD(2, (String)"Umidade: " + (String)dht.readHumidity() + "%");

}

void loop(){

    manterWifi();

    lerTemperatura();

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

    delay(2500);
    
}
