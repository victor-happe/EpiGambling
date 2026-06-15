#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

// --- CONFIGURATION RESEAU ---
const char* ssid = "EpiGambling_sys";
const char* password = NULL; 

// IP Fixe pour l'administration (192.168.4.1)
IPAddress apIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

// --- PINS ACTIVES ---
const int buttonPin = 4;
// Seulement 4 servos
const int NUM_SERVOS = 4; 
const int servoPins[NUM_SERVOS] = {12, 14, 13, 32};

Servo servos[NUM_SERVOS];
WebServer server(80);

// --- REGLAGES SERVOS (SECURITE) ---
const int POS_REPOS = 10;   
const int POS_ACTIVE = 160; 
// Temps d'impulsion pour le servo 360 (ajuster si besoin)
const int DUREE_IMPULSION_360 = 350; // Millisecondes pour faire un petit mouvement

int currentServoIndex = 0;
bool oldButtonState = HIGH;

// --- PAGE WEB ADMIN (Simplifiée) ---
String getHtml() {
  String html = "<!DOCTYPE HTML><html><body style='font-family:sans-serif; text-align:center; background:#222; color:#fff'>";
  html += "<h1>ESP32 Goodies Admin</h1>";
  html += "<p>IP Fixe : 192.168.4.1</p>";
  for(int i=0; i<NUM_SERVOS; i++) {
    html += "<p><a href='/test?id=" + String(i) + "' style='color:yellow'>TEST SERVO " + String(i+1) + " (GPIO " + String(servoPins[i]) + ")</a></p>";
  }
  html += "</body></html>";
  return html;
}

// Fonction pour bouger un servo (avec gestion 180° vs 360°)
void moveServoSequence(int id) {
  if(id < 0 || id >= NUM_SERVOS) return;

  Serial.print("Action Servo "); Serial.println(id + 1);

  // 1. On attache (on donne du jus)
  servos[id].attach(servoPins[id], 1000, 2000);
  
  // --- CAS SPECIAL : SERVO ROTATION CONTINUE (PIN 14) ---
  if (servoPins[id] == 14) { 
    // Sur un 360 : 180 fait tourner à fond dans un sens.
    servos[id].write(180); 
    delay(DUREE_IMPULSION_360); // On tourne juste le temps nécessaire
    
    // Retour à position neutre pour éviter les blocages
    servos[id].write(90);
    delay(200);
  }
  // --- CAS STANDARD : SERVO 180 DEGRES (PIN 12) ---
  else { 
    // Mouvement (Drop)
    servos[id].write(POS_ACTIVE);
    delay(800); 
    
    // Retour (Reset)
    servos[id].write(POS_REPOS);
    delay(800); 
  }

  // 4. ON COUPE (Silence total pour tous les servos)
  // Attendre un peu avant de détacher pour laisser le servo se stabiliser
  delay(100);
  servos[id].detach(); 
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT_PULLUP);

  // Initialisation des servos
  Serial.println("Initialisation Servos...");
  for(int i=0; i<NUM_SERVOS; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(servoPins[i], 1000, 2000);
    // Position initiale sécuritaire
    servos[i].write(POS_REPOS); 
    delay(100); 
    servos[i].detach(); 
  }

  // Configuration WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, gateway, subnet);
  WiFi.softAP(ssid, password);

  Serial.println("\n--- ESP32 PRET ---");
  Serial.print("IP ADDR: "); Serial.println(WiFi.softAPIP());

  // Routes Web
  server.on("/", [](){ server.send(200, "text/html", getHtml()); });
  server.on("/test", [](){
    if(server.hasArg("id")) {
      // Pour éviter les plantages, on ne fait pas de .toInt() direct, on vérifie.
      int id = server.arg("id").toInt();
      if(id >= 0 && id < NUM_SERVOS) {
        moveServoSequence(id);
        server.send(200, "text/plain", "OK");
        return;
      }
    }
    server.send(400, "text/plain", "Invalid ID");
  });

  server.begin();
}

void loop() {
  server.handleClient();

  // 1. Gestion du Bouton Physique
  bool currentButtonState = digitalRead(buttonPin);
  if(currentButtonState == LOW && oldButtonState == HIGH) {
    Serial.println("BUTTON_PRESSED");
    delay(300); 
  }
  oldButtonState = currentButtonState;

  // 2. Réception des ordres du Mac (via USB)
  if(Serial.available()) {
    String data = Serial.readStringUntil('\n');
    data.trim();
    
    if(data == "WIN_TRIGGER") {
      moveServoSequence(currentServoIndex);
      
      // On passe au suivant (modulage par 2 maintenant)
      currentServoIndex = (currentServoIndex + 1) % NUM_SERVOS; 
    }
    else if(data == "RESET_NEUTRAL") {
      Serial.println("RESET_NEUTRAL received");
      for(int i=0; i<NUM_SERVOS; i++) {
        servos[i].attach(servoPins[i], 1000, 2000);
        servos[i].write(POS_REPOS);
        delay(50);
        servos[i].detach();
      }
    }
  }
}
