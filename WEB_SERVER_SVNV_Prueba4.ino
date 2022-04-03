#include <ESP8266WiFi.h>

const char* ssid = "GEORGE";
const char* password = "PASSWORD";


WiFiServer server(80);

// Assign output variables to GPIO pins
const int output5 = 5;

//Flag variables
int state = 0; // Flag
int count = 0;

//Assign ADC variables
const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0
int sensorValue = 0;  // value read from the pot

// Auxiliar variables to store the current output state
String output5State = "Desactivada";
String output4State = "off";

void setup() {
  Serial.begin(9600);
  delay(10);

  //Configuración  del GPIO2
  pinMode(2, OUTPUT);
  digitalWrite(2,LOW);

  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  
  Serial.println();
  Serial.println();
  Serial.print("Conectandose a red : ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password); //Conexión a la red
  
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  
  
  server.begin(); //Iniciamos el servidor
  Serial.println("Servidor Iniciado");


  Serial.println("Ingrese desde un navegador web usando la siguiente IP:");
  Serial.println(WiFi.localIP()); //Obtenemos la IP
}

void loop() {
  WiFiClient client;
  if (state == 6)
  {
    // read the analog in value
    sensorValue = analogRead(analogInPin);
    count++;
  }
  else
  {
    client = server.available();
  }
  if (sensorValue > 140)
  {
    digitalWrite(output5, HIGH);
  }
  
  if (client) //Si hay un cliente presente
  { 
    Serial.println("Nuevo Cliente");
    
    //esperamos hasta que hayan datos disponibles
    while(!client.available()&&client.connected())
    {
    delay(1);
    }
    
    // Leemos la primera línea de la petición del cliente.
    String linea1 = client.readStringUntil('r');
    Serial.println(linea1);

    // Here goes the buttons code
    if (linea1.indexOf("OFF")>0) //Buscamos un LED=ON en la 1°Linea
    {
      digitalWrite(2,HIGH);
    }
    if (linea1.indexOf("ON")>0)//Buscamos un LED=OFF en la 1°Linea
    {
      digitalWrite(2,LOW);
    }

    //Bottons of the SVNV
    
    // State machine, calibration and alarm state selector
    if ((linea1.indexOf("State1") >= 0) && (state ==0)) {
         Serial.println("Calibrar ruido");
         state = 1;
    } else if ((linea1.indexOf("State3") >= 0) && (state == 2)) {
         Serial.println("estableciendo referencia de no uso");
         state = 3;
    } else if ((linea1.indexOf("State5") >= 0) && (state >= 4)) {
         Serial.println("estableciendo referencia de uso");
         state = 5;
         output4State = "on";
         count = 1;
    } else if ((linea1.indexOf("off") >= 0) && ((state == 5)|| (state == 6))) {
    Serial.println("Alarma apagada");
         output4State = "off";
         digitalWrite(output5, LOW);
    } else if ((linea1.indexOf("on") >= 0) && ((state == 5)|| (state == 6))) {
         Serial.println("Alarma encendida");
         output4State = "on";
         count = 1;
    }

    // Here start the server
    client.flush(); 
                
    Serial.println("Enviando respuesta...");   
    //Encabesado http    
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");// La conexión se cierra después de finalizar de la respuesta
    client.println();
    //Pagina html  para en el navegador
    // Display the HTML web page
    client.println("<!DOCTYPE html><html>");
    client.println("<head><title>George SVNV</title>");
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    // CSS to style the on/off buttons 
    // Feel free to change the background-color and font-size attributes to fit your preferences
    client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
    client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
    client.println(".button2 {background-color: #77878A;}</style></head>");
            
    // Web Page Heading
    client.println("<body><h1>Sistema de vigilancia no visual</h1>");
    
    //Test of ESP8266 led
    client.println("<div style='text-align:center;'>");
    client.println("<br />");            
    client.println("<p><a href=\"/ON\"><button class=\"button\">ON</button></a></p>");           
    client.println("<p><a href=\"/OFF\"><button class=\"button\">OFF</button></a></p>");
    client.println("<br />");
    client.println("</div>");

    //Code of the SVNV

    if(state == 0) {
    // Display current state, and ON/OFF buttons for GPIO 5  
    client.println("<p>Calibracion " + output5State + "</p>");
    // If the output5State is off, it displays the ON button       
    client.println("<p><a href=\"/5/State1\"><button class=\"button\">Iniciar</button></a></p>");
    }

    else if (state == 1){
    // Display the instructions  
    client.println("<p>Calibracion Inicida, porfavor espere 5 segundos y actualice la pagina </p>");
    delay(500);
    state = 2;         
    }

    else if (state == 2){
    // Display the instructions  
    client.println("<p>Para la ultima prueba, favor de abrir y cerrar la puerta o ventana por 5 segundos </p>");
    client.println("<p><a href=\"/5/State3\"><button class=\"button\">Iniciar</button></a></p>");
    }

    else if (state == 3){
    // Display the instructions  
    client.println("<p>Calibracion iniciada, espere 5 segundos y actualice la pagina </p>");
    delay(500);
    state = 4;
    }

    else if (state == 4){
    // Display the instructions  
    client.println("<p>Calibracion exitosa, su dispositivo esta listo para utilizarse </p>");
    client.println("<p>El boton siguiente muestra el estado de su alarma, favor de encenderla </p>");
    client.println("<p><a href=\"/5/State5\"><button class=\"button\">Encender</button></a></p>");         
    }

    else if ((state == 5)||(state == 6)){
      // Display the instructions  
      client.println("<p>La alarma esta " + output4State + "</p>");
      if (output4State=="off") {
        client.println("<p><a href=\"/4/on\"><button class=\"button\">ENCENDER</button></a></p>");
      } 
      else {
        client.println("<p><a href=\"/4/off\"><button class=\"button button2\">APAGAR</button></a></p>");
      } 
      state = 6;       
    }

    //Gives the structure to the page
    client.println("</body></html>");
    client.println();
    
    delay(1);
    Serial.println("respuesta enviada");
    Serial.println();

  }
 
  
}
