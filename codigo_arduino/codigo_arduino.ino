// SISTEMA DE MONITOREO EN TIEMPO REAL DE LA CALIDAD DEL AGUA
//                INTEGRANTES: 
//    David Alejandro Ballen Sánchez – 2254053
//    Ramiro Andrés Cárdenas Velásquez – 2254059
//    Jania Kathalina De La Peña Roqueme – 2254057
//
//             REFERENCIAs INTERNET:
//      - https://docs-arduino-cc.translate.goog/libraries/liquidcrystal/?_x_tr_sl=en&_x_tr_tl=es&_x_tr_hl=es&_x_tr_pto=tc&_x_tr_hist=true
//      - https://www.taloselectronics.com/blogs/tutoriales/como-usar-modulo-ph-4502-con-arduino
//      - https://how2electronics.com/tds-sensor-arduino-interfacing-water-quality-monitoring/
//      - https://fgcoca.github.io/Sensores-actuadores-y-shield-tipo-Arduino/turbidez/

// ====================================
// MEDIDOR COMPLETO DE CALIDAD DE AGUA
// HC-SR04 + TDS + TURBIDEZ + pH 
// LCD I2C 16x2 + LEDs + 2 LED RGB + 1 LED normal
// ✅ VERSIÓN FINAL v3.2 - OPTIMIZADO
// ====================================


#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Configuración del LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pines del sensor ultrasónico HC-SR04
const int pinTrig = 13;
const int pinEcho = 12;

// Pines sensores de calidad
const int pinTDS = A3;        // Sensor TDS
const int pinTurbidez = A1;   // Sensor de Turbidez
const int pinPH = A2;         // Sensor de pH ✅ NUEVO

// Pines de los LEDs (sensor ultrasónico)
const int ledVerde = 6;
const int ledAmarillo = 9;
const int ledRojo = 10;

// Pin del LED Verde simple (sensor TDS) - CAMBIADO A LED NORMAL ✅
const int ledVerdeTDS = 3;  // Solo 1 pin, LED verde normal

// Pines del LED RGB #2 (sensor Turbidez) - Color #e900ff
const int ledRGB2_R = 7;
const int ledRGB2_G = 8;
const int ledRGB2_B = 2;

// Pines del LED RGB #3 (sensor pH) - Color #FFAA00 ✅ NUEVO
const int ledRGB3_R = 4;   // Pin 4 (liberado) ✅
const int ledRGB3_G = 5;   // Pin 5 (liberado) ✅
const int ledRGB3_B = 11;  // Pin 11 (PWM)

// Variables para medición de nivel
long duracion = 0;
float distancia = 0;
int nivelAgua = 0;
int porcentaje = 0;

// Variables para sensor TDS
int valorTDS = 0;
float voltajeTDS = 0;
float ppmTDS = 0;
float temperatura = 25.0;

// Variables para sensor de Turbidez
int valorTurbidez = 0;
float voltajeTurbidez = 0;
float ntuTurbidez = 0;

// Variables para sensor de pH ✅ NUEVO
int valorPH = 0;
float voltajePH = 0;
float pH = 7.0;

// Configuración del tanque (en centímetros)
const int alturaMaxTanque = 30;
const int distanciaVacio = 50;
const int distanciaLleno = 20;

// Límites para encender LEDs RGB
const float limiteTDS_minimo = 150.0;      // 150 PPM (TDS)
const float limiteTurbidez_minimo = 5.0;   // 5 NTU (Turbidez)
const float limitePH_minimo = 6.5;         // pH mínimo seguro ✅ NUEVO
const float limitePH_maximo = 8.5;         // pH máximo seguro ✅ NUEVO

// Variable para alternar pantallas en LCD (ahora 4 pantallas)
unsigned long tiempoAnterior = 0;
int pantallaActual = 0;  // 0 = Nivel, 1 = TDS, 2 = Turbidez, 3 = pH ✅

void setup() {
  Serial.begin(9600);
  
  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  // Configurar LEDs de nivel
  pinMode(ledVerde, OUTPUT);
  pinMode(ledAmarillo, OUTPUT);
  pinMode(ledRojo, OUTPUT);
  
  // Configurar LED Verde simple (TDS) ✅
  pinMode(ledVerdeTDS, OUTPUT);
  
  // Configurar LED RGB #2 (Turbidez)
  pinMode(ledRGB2_R, OUTPUT);
  pinMode(ledRGB2_G, OUTPUT);
  pinMode(ledRGB2_B, OUTPUT);
  
  // Configurar LED RGB #3 (pH) ✅ NUEVO
  pinMode(ledRGB3_R, OUTPUT);
  pinMode(ledRGB3_G, OUTPUT);
  pinMode(ledRGB3_B, OUTPUT);
  
  // Configurar sensores
  pinMode(pinTrig, OUTPUT);
  pinMode(pinEcho, INPUT);
  pinMode(pinTDS, INPUT);
  pinMode(pinTurbidez, INPUT);
  pinMode(pinPH, INPUT);  // ✅ NUEVO
  
  // Apagar todos los LEDs
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
  digitalWrite(ledVerdeTDS, LOW);  // LED simple TDS ✅
  apagarLedRGB2();
  apagarLedRGB3();  // ✅ NUEVO
  
  // Mensaje de inicio
  lcd.setCursor(0, 0);
  lcd.print("SISTEMA AGUA");
  lcd.setCursor(0, 1);
  lcd.print("4 SENSORES v3.0");
  
  Serial.println("====================================");
  Serial.println("  MEDIDOR COMPLETO DE AGUA");
  Serial.println("  Nivel + TDS + Turbidez + pH");
  Serial.println("  ✅ VERSIÓN FINAL v3.0");
  Serial.println("====================================");
  Serial.println();
  Serial.print("LED Verde (TDS): > ");
  Serial.print(limiteTDS_minimo, 0);
  Serial.println(" PPM (LED simple)");
  Serial.print("LED RGB #2 (Turbidez): > ");
  Serial.print(limiteTurbidez_minimo, 0);
  Serial.println(" NTU");
  Serial.print("LED RGB #3 (pH): fuera de rango ");
  Serial.print(limitePH_minimo, 1);
  Serial.print(" - ");
  Serial.print(limitePH_maximo, 1);
  Serial.println();
  delay(2500);
  
  // Test de LEDs de nivel
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test LEDs Nivel");
  Serial.println("Test de LEDs de nivel...");
  
  lcd.setCursor(0, 1);
  lcd.print("Rojo");
  digitalWrite(ledRojo, HIGH);
  delay(700);
  digitalWrite(ledRojo, LOW);
  
  lcd.setCursor(0, 1);
  lcd.print("Amarillo        ");
  digitalWrite(ledAmarillo, HIGH);
  delay(700);
  digitalWrite(ledAmarillo, LOW);
  
  lcd.setCursor(0, 1);
  lcd.print("Verde           ");
  digitalWrite(ledVerde, HIGH);
  delay(700);
  digitalWrite(ledVerde, LOW);
  
  // Test LED Verde simple (TDS) ✅
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test LED TDS");
  lcd.setCursor(0, 1);
  lcd.print("LED Verde");
  Serial.println("Test LED Verde (TDS)...");
  digitalWrite(ledVerdeTDS, HIGH);
  delay(2000);
  digitalWrite(ledVerdeTDS, LOW);
  
  // Test LED RGB #2 (Turbidez)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test RGB #2");
  lcd.setCursor(0, 1);
  lcd.print("Turbidez:#e900ff");
  Serial.println("Test LED RGB #2 (Turbidez)...");
  setColorRGB2(233, 0, 255);
  delay(2000);
  apagarLedRGB2();
  
  // Test LED RGB #3 (pH) ✅ NUEVO
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test RGB #3");
  lcd.setCursor(0, 1);
  lcd.print("pH: #FF1E00");
  Serial.println("Test LED RGB #3 (pH)...");
  setColorRGB3(255, 30, 0);
  delay(2000);
  apagarLedRGB3();
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Test OK!");
  Serial.println("Test completado");
  Serial.println();
  delay(1000);
  lcd.clear();
  
  tiempoAnterior = millis();
}

void loop() {
  // ===== LEER SENSORES =====
  // Sensor ultrasónico
  duracion = readUltrasonicDistance();
  distancia = duracion * 0.034 / 2;
  
  if (distancia < 2 || distancia > 400 || duracion == 0) {
    distancia = distanciaVacio;
  }
  
  distancia = constrain(distancia, distanciaLleno, distanciaVacio);
  nivelAgua = distanciaVacio - distancia;
  nivelAgua = constrain(nivelAgua, 0, alturaMaxTanque);
  porcentaje = map(nivelAgua, 0, alturaMaxTanque, 0, 100);
  porcentaje = constrain(porcentaje, 0, 100);
  
  // Sensor TDS
  leerSensorTDS();
  
  // Sensor de Turbidez
  leerSensorTurbidez();
  
  // Sensor de pH ✅ NUEVO
  leerSensorPH();
  
  // ===== ALTERNAR PANTALLAS EN LCD (4 pantallas) =====
  if (millis() - tiempoAnterior >= 3000) {
    pantallaActual = (pantallaActual + 1) % 4;  // Ahora son 4 pantallas
    tiempoAnterior = millis();
  }
  
  // Mostrar pantalla correspondiente
  if (pantallaActual == 0) {
    mostrarNivelEnLCD();
  } else if (pantallaActual == 1) {
    mostrarTDSEnLCD();
  } else if (pantallaActual == 2) {
    mostrarTurbidezEnLCD();
  } else {
    mostrarPHEnLCD();  // ✅ NUEVO
  }
  
  // Mostrar en Monitor Serial
  mostrarEnSerial();
  
  // Controlar LEDs
  controlarLEDs();
  controlarLedVerdeTDS();  // LED simple TDS ✅
  controlarLedRGB2();      // Turbidez
  controlarLedRGB3();      // pH ✅ NUEVO
  
  delay(1000);
}

// Función para leer sensor ultrasónico HC-SR04
long readUltrasonicDistance() {
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(5);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);
  long dur = pulseIn(pinEcho, HIGH, 30000);
  return dur;
}

// Función para leer sensor TDS v1.0
void leerSensorTDS() {
  valorTDS = 0;
  for (int i = 0; i < 10; i++) {
    valorTDS += analogRead(pinTDS);
    delay(10);
  }
  valorTDS = valorTDS / 10;
  
  voltajeTDS = valorTDS * (5.0 / 1024.0);
  
  float coeficienteCompensacion = 1.0 + 0.02 * (temperatura - 25.0);
  float voltajeCompensado = voltajeTDS / coeficienteCompensacion;
  
  ppmTDS = (133.42 * voltajeCompensado * voltajeCompensado * voltajeCompensado 
            - 255.86 * voltajeCompensado * voltajeCompensado 
            + 857.39 * voltajeCompensado) * 0.5;
  
  if (ppmTDS < 0) {
    ppmTDS = 0;
  }
}

// Función para leer sensor de Turbidez
void leerSensorTurbidez() {
  valorTurbidez = 0;
  for (int i = 0; i < 10; i++) {
    valorTurbidez += analogRead(pinTurbidez);
    delay(10);
  }
  valorTurbidez = valorTurbidez / 10;
  
  voltajeTurbidez = valorTurbidez * (5.0 / 1024.0);
  
  if (voltajeTurbidez > 4.0) {
    ntuTurbidez = 0 + (5.0 - voltajeTurbidez) * 2;
  } 
  else if (voltajeTurbidez > 3.0) {
    ntuTurbidez = 2 + (4.0 - voltajeTurbidez) * 10;
  } 
  else if (voltajeTurbidez > 2.0) {
    ntuTurbidez = 12 + (3.0 - voltajeTurbidez) * 30;
  } 
  else if (voltajeTurbidez > 1.0) {
    ntuTurbidez = 42 + (2.0 - voltajeTurbidez) * 50;
  } 
  else {
    ntuTurbidez = 92 + (1.0 - voltajeTurbidez) * 100;
  }
  
  if (ntuTurbidez < 0) {
    ntuTurbidez = 0;
  }
}

// ✅ NUEVA - Función para leer sensor de pH
void leerSensorPH() {
  // Leer valor analógico (promedio de 10 lecturas)
  valorPH = 0;
  for (int i = 0; i < 10; i++) {
    valorPH += analogRead(pinPH);
    delay(10);
  }
  valorPH = valorPH / 10;
  
  // Convertir a voltaje (0-1023 = 0-5V)
  voltajePH = valorPH * (5.0 / 1024.0);
  
  // Convertir voltaje a pH
  // Fórmula de calibración estándar para sensores de pH analógicos
  // IMPORTANTE: Estos valores pueden necesitar calibración según tu sensor
  // pH = 7 cuando voltaje = 2.5V (punto neutro)
  // Cada 0.18V representa aproximadamente 1 unidad de pH
  
  // Calibración básica:
  // Voltaje 2.5V = pH 7 (neutro)
  // Pendiente: -5.7 (aproximado, puede variar por sensor)
  pH = 7.0 - ((voltajePH - 2.5) / 0.18);
  
  // Limitar valores de pH al rango válido (0-14)
  if (pH < 0) pH = 0;
  if (pH > 14) pH = 14;
}

// Función para mostrar nivel en LCD
void mostrarNivelEnLCD() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Nivel:");
  lcd.print(nivelAgua);
  lcd.print("cm ");
  lcd.print(porcentaje);
  lcd.print("%");
  
  lcd.setCursor(0, 1);
  
  if (porcentaje >= 70) {
    lcd.print("ALTO ");
    int barras = map(porcentaje, 70, 100, 5, 11);
    for (int i = 0; i < barras; i++) {
      lcd.write(0xFF);
    }
  } 
  else if (porcentaje >= 30) {
    lcd.print("MEDIO ");
    int barras = map(porcentaje, 30, 69, 3, 10);
    for (int i = 0; i < barras; i++) {
      lcd.write(0xFF);
    }
  } 
  else {
    lcd.print("BAJO! ");
    int barras = map(porcentaje, 0, 29, 0, 10);
    for (int i = 0; i < barras; i++) {
      lcd.write(0xFF);
    }
  }
}

// Función para mostrar TDS en LCD
void mostrarTDSEnLCD() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("TDS:");
  
  if (ppmTDS < 10) {
    lcd.print(" ");
  }
  lcd.print((int)ppmTDS);
  lcd.print(" PPM");
  
  lcd.setCursor(0, 1);
  
  if (ppmTDS < 100) {
    lcd.print("Agua destilada");
  } else if (ppmTDS < 150) {
    lcd.print("Agua filtrada");
  } else if (ppmTDS < 300) {
    lcd.print("Agua llave");
  } else {
    lcd.print("MINERALIZADA");
  }
}

// Función para mostrar Turbidez en LCD
void mostrarTurbidezEnLCD() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("Turbidez:");
  
  if (ntuTurbidez < 10) {
    lcd.print(" ");
  }
  lcd.print((int)ntuTurbidez);
  lcd.print(" NTU");
  
  lcd.setCursor(0, 1);
  
  if (ntuTurbidez < 5) {
    lcd.print("Agua clara");
  } else if (ntuTurbidez < 25) {
    lcd.print("Ligeramente");
  } else if (ntuTurbidez < 50) {
    lcd.print("Turbio");
  } else {
    lcd.print("MUY TURBIO!");
  }
}

// ✅ NUEVA - Función para mostrar pH en LCD
void mostrarPHEnLCD() {
  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.print("pH: ");
  lcd.print(pH, 1);
  
  lcd.setCursor(0, 1);
  
  if (pH < 4.0) {
    lcd.print("MUY ACIDO!");
  } else if (pH < 6.5) {
    lcd.print("Acido");
  } else if (pH <= 8.5) {
    lcd.print("pH SEGURO");
  } else if (pH < 10.0) {
    lcd.print("Alcalino");
  } else {
    lcd.print("MUY ALCALINO!");
  }
}

// Función para mostrar información en Monitor Serial
void mostrarEnSerial() {
  Serial.println("====================================");
  
  // Información de nivel
  Serial.print("Distancia:     ");
  Serial.print(distancia, 1);
  Serial.println(" cm");
  
  Serial.print("Nivel agua:    ");
  Serial.print(nivelAgua);
  Serial.print(" cm (");
  Serial.print(porcentaje);
  Serial.println("%)");
  
  Serial.print("Indicador:     [");
  int barras = porcentaje / 5;
  for (int i = 0; i < 20; i++) {
    if (i < barras) {
      Serial.print("█");
    } else {
      Serial.print("-");
    }
  }
  Serial.print("] ");
  Serial.print(porcentaje);
  Serial.println("%");
  
  Serial.print("Estado nivel:  ");
  if (porcentaje >= 70) {
    Serial.println("ALTO ✓ (LED Verde)");
  } else if (porcentaje >= 30) {
    Serial.println("MEDIO ⚠ (LED Amarillo)");
  } else {
    Serial.println("BAJO ⚠⚠ (LED Rojo)");
  }
  
  Serial.println();
  
  // Información TDS
  Serial.print("TDS valor:     ");
  Serial.print(valorTDS);
  Serial.print(" (crudo 0-1023)");
  Serial.println();
  
  Serial.print("TDS voltaje:   ");
  Serial.print(voltajeTDS, 3);
  Serial.println(" V");
  
  Serial.print("TDS PPM:       ");
  Serial.print(ppmTDS, 1);
  Serial.println(" PPM");
  
  Serial.print("Estado TDS:    ");
  if (ppmTDS < limiteTDS_minimo) {
    Serial.print("Agua baja en minerales (");
    Serial.print(ppmTDS, 1);
    Serial.println(" PPM) - LED Verde apagado");
  } else {
    Serial.print("⚠ AGUA MINERALIZADA - ");
    Serial.print(ppmTDS, 1);
    Serial.println(" PPM (LED Verde ENCENDIDO)");
  }
  
  Serial.println();
  
  // Información Turbidez
  Serial.print("Turbidez valor:");
  Serial.print(valorTurbidez);
  Serial.print(" (crudo 0-1023)");
  Serial.println();
  
  Serial.print("Turbidez volt: ");
  Serial.print(voltajeTurbidez, 3);
  Serial.println(" V");
  
  Serial.print("Turbidez NTU:  ");
  Serial.print(ntuTurbidez, 1);
  Serial.println(" NTU");
  
  Serial.print("Estado Turbidez:");
  if (ntuTurbidez < limiteTurbidez_minimo) {
    Serial.print("Agua clara (");
    Serial.print(ntuTurbidez, 1);
    Serial.println(" NTU) - LED RGB #2 apagado");
  } else {
    Serial.print("⚠ AGUA TURBIA - ");
    Serial.print(ntuTurbidez, 1);
    Serial.println(" NTU (LED RGB #2 ENCENDIDO)");
  }
  
  Serial.println();
  
  // ✅ NUEVA - Información pH
  Serial.print("pH valor:      ");
  Serial.print(valorPH);
  Serial.print(" (crudo 0-1023)");
  Serial.println();
  
  Serial.print("pH voltaje:    ");
  Serial.print(voltajePH, 3);
  Serial.println(" V");
  
  Serial.print("pH medido:     ");
  Serial.print(pH, 2);
  Serial.println();
  
  Serial.print("Estado pH:     ");
  if (pH >= limitePH_minimo && pH <= limitePH_maximo) {
    Serial.print("✓ pH SEGURO (");
    Serial.print(pH, 1);
    Serial.println(") - LED RGB #3 apagado");
  } else {
    Serial.print("⚠ pH FUERA DE RANGO - ");
    Serial.print(pH, 1);
    if (pH < limitePH_minimo) {
      Serial.println(" (ÁCIDO) - LED RGB #3 ENCENDIDO");
    } else {
      Serial.println(" (ALCALINO) - LED RGB #3 ENCENDIDO");
    }
  }
  
  Serial.println();
}

// Función para controlar LEDs de nivel
void controlarLEDs() {
  digitalWrite(ledVerde, LOW);
  digitalWrite(ledAmarillo, LOW);
  digitalWrite(ledRojo, LOW);
  
  delay(50);
  
  if (porcentaje >= 70) {
    digitalWrite(ledVerde, HIGH);
  } 
  else if (porcentaje >= 30) {
    digitalWrite(ledAmarillo, HIGH);
  } 
  else {
    digitalWrite(ledRojo, HIGH);
  }
}

// ✅ NUEVA - Función para controlar LED Verde simple (TDS - >150 PPM)
void controlarLedVerdeTDS() {
  if (ppmTDS >= limiteTDS_minimo) {
    digitalWrite(ledVerdeTDS, HIGH);  // Encender LED verde
  } else {
    digitalWrite(ledVerdeTDS, LOW);   // Apagar LED verde
  }
}

// Función para controlar LED RGB #2 (Turbidez - >5 NTU)
void controlarLedRGB2() {
  if (ntuTurbidez >= limiteTurbidez_minimo) {
    setColorRGB2(233, 0, 255);  // #e900ff
  } else {
    apagarLedRGB2();
  }
}

// ✅ NUEVA - Función para controlar LED RGB #3 (pH fuera de 6.5-8.5)
void controlarLedRGB3() {
  // El LED se enciende cuando el pH está FUERA del rango seguro
  if (pH < limitePH_minimo || pH > limitePH_maximo) {
    setColorRGB3(255, 30, 0);  // #FF1E00 (naranja)
  } else {
    apagarLedRGB3();  // Apagado = pH seguro
  }
}

// Funciones para LED RGB #2 (Turbidez)
void setColorRGB2(int rojo, int verde, int azul) {
  analogWrite(ledRGB2_R, rojo);
  analogWrite(ledRGB2_G, verde);
  analogWrite(ledRGB2_B, azul);
}

void apagarLedRGB2() {
  analogWrite(ledRGB2_R, 0);
  analogWrite(ledRGB2_G, 0);
  analogWrite(ledRGB2_B, 0);
}

// ✅ NUEVAS - Funciones para LED RGB #3 (pH)
void setColorRGB3(int rojo, int verde, int azul) {
  analogWrite(ledRGB3_R, rojo);
  analogWrite(ledRGB3_G, verde);
  analogWrite(ledRGB3_B, azul);
}

void apagarLedRGB3() {
  analogWrite(ledRGB3_R, 0);
  digitalWrite(ledRGB3_G, LOW);
  digitalWrite(ledRGB3_B, LOW);
}