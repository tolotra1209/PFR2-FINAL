#include <AFMotor.h>
#include <SoftwareSerial.h>

SoftwareSerial bluetoothSerial(10, 11); // RX, TX

#define TRIGGER_AVANT A1
#define ECHO_AVANT A0
#define TRIGGER_GAUCHE A4
#define ECHO_GAUCHE A5
#define TRIGGER_DROIT A2
#define ECHO_DROIT A3

const int distanceCible = 25; // Seuil de distance en cm
const int maxDistance = 400; // Distance maximale pour considération valide

#define LEFT 0
#define RIGHT 1

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

void setup() {
  Serial.begin(9600); // Initialisation de la communication série
  bluetoothSerial.begin(9600);  // Communication Bluetooth
  pinMode(TRIGGER_AVANT, OUTPUT);
  pinMode(ECHO_AVANT, INPUT);
  pinMode(TRIGGER_GAUCHE, OUTPUT);
  pinMode(ECHO_GAUCHE, INPUT);
  pinMode(TRIGGER_DROIT, OUTPUT);
  pinMode(ECHO_DROIT, INPUT);
  
  // Initialisation des moteurs
  initMotors();
}

void loop() {
  if (bluetoothSerial.available()) {
    char command = bluetoothSerial.read();
    Serial.print("Received: ");
    Serial.println(command);
    executeCommand(command);
  }
  if (Serial.available()) {
    char sendChar = Serial.read();
    bluetoothSerial.print(sendChar);
  }
}

void initMotors() {
  motor1.setSpeed(0);
  motor1.run(RELEASE);
  motor2.setSpeed(0);
  motor2.run(RELEASE);
  motor3.setSpeed(0);
  motor3.run(RELEASE);
  motor4.setSpeed(0);
  motor4.run(RELEASE);
  Serial.println("Motors initialized");
}

long mesurerDistance(int triggerPin, int echoPin) {
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  long duration = pulseIn(echoPin, HIGH);
  long distance = duration * 0.034 / 2;
  if (distance == 0 || distance > maxDistance) {
    distance = maxDistance;
  }
  Serial.print("Measured distance at pin ");
  Serial.print(triggerPin);
  Serial.print(": ");
  Serial.println(distance);
  return distance;
}

void setSpeed(int vitesseGauche, int vitesseDroit) {
  motor1.setSpeed(vitesseGauche);
  motor2.setSpeed(vitesseDroit);
  motor3.setSpeed(vitesseDroit);
  motor4.setSpeed(vitesseGauche);
  Serial.print("Set speed - Left: ");
  Serial.print(vitesseGauche);
  Serial.print(" Right: ");
  Serial.println(vitesseDroit);
}

void avancer(int vitesseGauche, int vitesseDroit) {
  Serial.println("Avancer");
  setSpeed(vitesseGauche, vitesseDroit);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  delay(200);
  arreter();
}

void reculer(int vitesseGauche, int vitesseDroit, int temps) {
  Serial.println("Reculer");
  arreter();
  setSpeed(vitesseGauche, vitesseDroit);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  delay(temps);
  arreter();
}

void arreter() {
  Serial.println("Arrêter");
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
  delay(200);
}

void tourner(int direction, int vitesse, int temps) {
  arreter();
  setSpeed(vitesse, vitesse);
  if (direction == LEFT) {
    Serial.println("Tourner à gauche");
    motor1.run(BACKWARD);
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    motor4.run(BACKWARD);
  } else if (direction == RIGHT) {
    Serial.println("Tourner à droite");
    motor1.run(FORWARD);
    motor2.run(BACKWARD);
    motor3.run(BACKWARD);
    motor4.run(FORWARD);
  }
  delay(temps);
  arreter();
}

void suivreMur(int triggerPin, int echoPin, bool isLeft) {
  long distance = mesurerDistance(triggerPin, echoPin);
  float erreur = distance - distanceCible;

  Serial.print("Suivre mur - Distance: ");
  Serial.print(distance);
  Serial.print(" Erreur: ");
  Serial.println(erreur);

   // Vérifier si la distance mesurée est nulle ou très grande
  if (distance != 0 && distance < 400 ) {
    // Calculer les vitesses des moteurs en fonction de l'erreur
    float vitesseGauche, vitesseDroit;
    if(isLeft==true){
      if (erreur < 0) {
        vitesseGauche = 225;
        vitesseDroit = 100;
        avancer(vitesseGauche, vitesseDroit);
      } else if (erreur > 0) {
        vitesseGauche = 100;
        vitesseDroit = 225;
        avancer(vitesseGauche, vitesseDroit);
      } else {
        vitesseGauche = 200;
        vitesseDroit = 200;
        avancer(vitesseGauche, vitesseDroit);
      }
    } else {
      if (erreur > 0) {
        vitesseGauche = 225;
        vitesseDroit = 100;
        avancer(vitesseGauche, vitesseDroit);
      } else if (erreur < 0) {
        vitesseGauche = 100;
        vitesseDroit = 225;
        avancer(vitesseGauche, vitesseDroit);
      } else {
        vitesseGauche = 200;
        vitesseDroit = 200;
        avancer(vitesseGauche, vitesseDroit);
      }
    }
  }
}

void modeAuto() {
  while (true) {
    if (bluetoothSerial.available() && bluetoothSerial.read() == 's') {
      arreter();
      return;
    }

    long distanceAvant = mesurerDistance(TRIGGER_AVANT, ECHO_AVANT);
    Serial.print("Mode auto - Distance avant: ");
    Serial.println(distanceAvant);

    // Avancer jusqu'à ce qu'un mur soit détecté devant
    while (distanceAvant > distanceCible) {
      if (bluetoothSerial.available() && bluetoothSerial.read() == 's') {
        arreter();
        return;
      }
      avancer(200, 200);
      distanceAvant = mesurerDistance(TRIGGER_AVANT, ECHO_AVANT);
    }

    // Reculer un peu et décider de la direction à suivre
    reculer(150, 150, 200);

    long distanceGauche = mesurerDistance(TRIGGER_GAUCHE, ECHO_GAUCHE);
    long distanceDroit = mesurerDistance(TRIGGER_DROIT, ECHO_DROIT);

    Serial.print("Distances - Gauche: ");
    Serial.print(distanceGauche);
    Serial.print(" Droite: ");
    Serial.println(distanceDroit);

    if (distanceDroit > distanceGauche) {
      tourner(RIGHT, 242, 500);
      while ((distanceAvant = mesurerDistance(TRIGGER_AVANT, ECHO_AVANT)) > distanceCible) {
        if (bluetoothSerial.available() && bluetoothSerial.read() == 's') {
          arreter();
          return;
        }
        suivreMur(TRIGGER_GAUCHE, ECHO_GAUCHE, true);
      }
    } else {
      tourner(LEFT, 242, 500);
      while ((distanceAvant = mesurerDistance(TRIGGER_AVANT, ECHO_AVANT)) > distanceCible) {
        if (bluetoothSerial.available() && bluetoothSerial.read() == 's') {
          arreter();
          return;
        }
        suivreMur(TRIGGER_DROIT, ECHO_DROIT, false);
      }
    }
  }
}

void executeCommand(char cmd) {
  Serial.print("Executing command: ");
  Serial.println(cmd);
  
  if (cmd == 'f') {
    long distanceAvant = mesurerDistance(TRIGGER_AVANT, ECHO_AVANT);
    while (distanceAvant > distanceCible) {
      if (bluetoothSerial.available() && bluetoothSerial.read() == 's') {
        arreter();
        return;
      }
      avancer(200, 200);
      distanceAvant = mesurerDistance(TRIGGER_AVANT, ECHO_AVANT);
    }
  } else if (cmd == 'b') {
    reculer(200, 200, 500);
  } else if (cmd == 'l') {
    tourner(LEFT, 242, 500);
  } else if (cmd == 'r') {
    tourner(RIGHT, 242, 500);
  } else if (cmd == 's') {
    arreter();
  } else if (cmd == 'a') {
    modeAuto();
  }
}
