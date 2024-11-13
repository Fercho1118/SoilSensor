// Bibliotecas necesarias para conectar con WiFi, manejar un servidor web y HTTP.
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "Adafruit_seesaw.h"

// Instancia del sensor Adafruit Seesaw.
Adafruit_seesaw ss;

// URL al script PHP que maneja la inserción de datos en la base de datos.
String URL = "http://IP/seesaw_project/test_data.php";

// Credenciales de la red WiFi.
const char* ssid = "<Nombre de la red"; 
const char* password = "Contraseña de internet"; 

// Variables para almacenar el estado del sensor y la última vez que se reportaron datos.
uint32_t tsLastReport = 0;
float temperature = 0;
uint16_t humidity = 0;
uint8_t D23 = 23;
WebServer server(80);     

// Configura la conexión WiFi, el servidor web y el sensor. 
void setup() {
  Serial.begin(115200);
  pinMode(D23, INPUT);
  delay(100);   
 
  Serial.println("Conectando a ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("¡WiFi conectado!");
  Serial.print("IP: ");  Serial.println(WiFi.localIP());
 
  server.on("/", handle_OnConnect);
  server.on("/data", handle_Data);
  server.onNotFound(handle_NotFound);
 
  server.begin();
  Serial.println(" Servidor HTTP empezado");

  if (!ss.begin(0x36)) {
    Serial.println("ERROR! seesaw no encontrado");
    while(1) delay(1);
  }
}
 
// Ciclo principal: maneja clientes del servidor web y lee datos del sensor.
void loop() {
    server.handleClient();
    if (digitalRead(D23) == LOW) {
        temperature = ss.getTemp();
        humidity = ss.touchRead(0);

        HTTPClient http;
        http.begin(URL);
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        
        String postData = "temperature=" + String(temperature) + "&humidity=" + String(humidity);
        int httpCode = http.POST(postData);

        if (httpCode > 0) { 
            String payload = http.getString();
            Serial.println(httpCode);
            Serial.println(payload);
        } else {
            Serial.println("Error enviando POST: " + String(httpCode));
        }

        http.end(); 
    } else {
        temperature = 0;
        humidity = 0;
    }
    delay(2000); 
}

// Maneja las solicitudes HTTP GET a la raíz.
void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(temperature, humidity));
}

// Maneja las solicitudes GET a /data, devolviendo los datos actuales en formato JSON.
void handle_Data() {
  String jsonData = "{\"temperature\":" + String(temperature) + ",\"humidity\":" + String(humidity) + "}";
  server.send(200, "application/json", jsonData);
}

// Maneja cualquier ruta no encontrada.
void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

// Genera y devuelve el código HTML para la página web.
String SendHTML(float temperature, int humidity) {
  String ptr = "<!DOCTYPE html><html><head>";
  ptr += "<title>ESP32 Soil Sensor</title>";
  ptr += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  ptr += "<link href='https://fonts.googleapis.com/css?family=Open+Sans:300,400,600' rel='stylesheet'>";
  ptr += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  ptr += "<style>";
  ptr += "html, body { font-family: 'Open Sans', sans-serif; text-align: center; margin: 0; padding: 0;}";
  ptr += "h1 { margin: 50px auto 30px; }";
  ptr += ".side-by-side { display: table-cell; vertical-align: middle; position: relative; }";
  ptr += ".text { font-weight: 600; font-size: 19px; width: 200px; }";
  ptr += ".reading { font-weight: 300; font-size: 50px; padding-right: 25px; }";
  ptr += ".temperature .reading { color: #F29C1F; }";
  ptr += ".humidity .reading { color: #3B97D3; }";
  ptr += ".superscript { font-size: 17px; font-weight: 600; position: absolute; top: 10px; }";
  ptr += ".data { padding: 10px; }";
  ptr += ".container { display: table; margin: 0 auto; }";
  ptr += ".icon { width: 65px; }";
  ptr += ".chart-container { width: 45%; min-width: 300px; height: 300px; margin-top: 20px; float: left; }";
  ptr += "canvas { width: 100% !important; height: auto !important; }";
  ptr += "</style></head><body>";
  ptr += "<h1>Sensor de temperatura y humedad capacitivo del suelo con ESP32</h1>";
  ptr += "<div class='container'>";
  ptr += "<div class='data temperature'>";
  ptr += "<div class='side-by-side icon'>";
  ptr += "<svg enable-background='new 0 0 19.438 54.003'height=54.003px id=Layer_1 version=1.1 viewBox='0 0 19.438 54.003'width=19.438px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><g><path d='M11.976,8.82v-2h4.084V6.063C16.06,2.715,13.345,0,9.996,0H9.313C5.965,0,3.252,2.715,3.252,6.063v30.982";
  ptr +="C1.261,38.825,0,41.403,0,44.286c0,5.367,4.351,9.718,9.719,9.718c5.368,0,9.719-4.351,9.719-9.718";
  ptr +="c0-2.943-1.312-5.574-3.378-7.355V18.436h-3.914v-2h3.914v-2.808h-4.084v-2h4.084V8.82H11.976z M15.302,44.833";
  ptr +="c0,3.083-2.5,5.583-5.583,5.583s-5.583-2.5-5.583-5.583c0-2.279,1.368-4.236,3.326-5.104V24.257C7.462,23.01,8.472,22,9.719,22";
  ptr +="s2.257,1.01,2.257,2.257V39.73C13.934,40.597,15.302,42.554,15.302,44.833z'fill=#F29C21 /></g></svg>";
  ptr +="</div>";
  ptr += "<div class='side-by-side text'>Temperature</div>";
  ptr += "<div class='side-by-side reading' id='tempValue'>" + String((int)temperature) + "<span class='superscript'>&deg;C</span></div>";
  ptr += "</div>";
  ptr += "<div class='data humidity'>";
  ptr += "<div class='side-by-side icon'>";
  ptr +="<svg enable-background='new 0 0 29.235 40.64'height=40.64px id=Layer_1 version=1.1 viewBox='0 0 29.235 40.64'width=29.235px x=0px xml:space=preserve xmlns=http://www.w3.org/2000/svg xmlns:xlink=http://www.w3.org/1999/xlink y=0px><path d='M14.618,0C14.618,0,0,17.95,0,26.022C0,34.096,6.544,40.64,14.618,40.64s14.617-6.544,14.617-14.617";
  ptr +="C29.235,17.95,14.618,0,14.618,0z M13.667,37.135c-5.604,0-10.162-4.56-10.162-10.162c0-0.787,0.638-1.426,1.426-1.426";
  ptr +="c0.787,0,1.425,0.639,1.425,1.426c0,4.031,3.28,7.312,7.311,7.312c0.787,0,1.425,0.638,1.425,1.425";
  ptr +="C15.093,36.497,14.455,37.135,13.667,37.135z'fill=#3C97D3 /></svg>";
  ptr +="</div>";
  ptr += "<div class='side-by-side text'>Capacitive</div>";
  ptr += "<div class='side-by-side reading' id='humidValue'>" + String(humidity) + "<span class='superscript'>%</span></div>";
  ptr += "</div>";
  ptr += "</div>";
  ptr += "<div class='chart-container'><canvas id='tempChart'></canvas></div>";
  ptr += "<div class='chart-container'><canvas id='humidityChart'></canvas></div>";
  ptr += "<script>";
  ptr += "var tempCtx = document.getElementById('tempChart').getContext('2d');";
  ptr += "var humidityCtx = document.getElementById('humidityChart').getContext('2d');";
  ptr += "var tempChart = new Chart(tempCtx, { type: 'line', data: { labels: [], datasets: [{ label: 'Temperature', data: [], borderColor: 'rgb(255, 99, 132)' }] }, options: { scales: { y: { beginAtZero: true } } } });";
  ptr += "var humidityChart = new Chart(humidityCtx, { type: 'line', data: { labels: [], datasets: [{ label: 'Humidity', data: [], borderColor: 'rgb(54, 162, 235)' }] }, options: { scales: { y: { beginAtZero: true } } } });";
  ptr += "function addData(chart, data) { chart.data.labels.push(''); chart.data.datasets.forEach((dataset) => { dataset.data.push(data); }); chart.update(); }";
  ptr += "setInterval(function() { fetch('/data').then(response => response.json()).then(data => { document.getElementById('tempValue').innerHTML = data.temperature + '&deg;C'; document.getElementById('humidValue').innerHTML = data.humidity + '%'; addData(tempChart, data.temperature); addData(humidityChart, data.humidity); }).catch(error => console.error('Error fetching data:', error)); }, 2000);";
  ptr += "</script></body></html>";
  return ptr;
}  