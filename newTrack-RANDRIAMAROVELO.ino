#include <Pixy2.h>
#include <AFMotor.h>

// Initialisation de la caméra Pixy2
Pixy2 pixy;

// Définir les moteurs en utilisant la bibliothèque AFMotor
AF_DCMotor motor1(1); // Motor 1 is connected to M1 port on the shield
AF_DCMotor motor2(2); // Motor 2 is connected to M2 port on the shield
AF_DCMotor motor3(3); // Motor 3 is connected to M3 port on the shield
AF_DCMotor motor4(4); // Motor 4 is connected to M4 port on the shield

// Constantes pour le seuil de direction et la distance
const int centerThreshold = 20; // Seuil pour considérer que l'objet est centré
const int closeDistance = 50;   // Largeur du bloc considérée comme proche
const int signatureToTrack = 2; // Signature spécifique de l'objet à suivre

// Constantes pour le mouvement des servos
const int panMin = 0;
const int panMax = 1000;
const int tiltMin = 500;
const int tiltMax = 1000;
int panPos = 500;
int tiltPos = 750;
int panIncrement = 100;
int tiltIncrement = 50;

void setup() {
  // Initialisation des broches des moteurs
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  // Initialisation de la communication série et de la caméra Pixy2
  Serial.begin(9600);
  pixy.init();

  // Allumer la lampe de la caméra Pixy2
  pixy.setLamp(1, 1);

  // Initialiser les positions des servos
  pixy.setServos(panPos, tiltPos);
}

// Fonction pour faire avancer le robot
void forward() {
  motor1.setSpeed(150); // Set the speed of the motors
  motor2.setSpeed(150);
  motor3.setSpeed(150);
  motor4.setSpeed(150);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

// Fonction pour arrêter le robot
void stopRobot() {
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

// Fonction pour faire tourner le robot à gauche
void turnLeft() {
  motor1.setSpeed(150);
  motor2.setSpeed(150);
  motor3.setSpeed(150);
  motor4.setSpeed(150);
  motor1.run(BACKWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(BACKWARD);
  delay(500);
}

// Fonction pour faire tourner le robot à droite
void turnRight() {
  motor1.setSpeed(150);
  motor2.setSpeed(150);
  motor3.setSpeed(150);
  motor4.setSpeed(150);
  motor1.run(FORWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(FORWARD);
  delay(500);
}

// Fonction pour faire un balayage à gauche et à droite
void scanForObject() {
  static bool turningLeft = true;
  static bool tiltingUp = true;
  static unsigned long lastTurnTime = 0;
  const unsigned long turnDuration = 500; // Durée du balayage en millisecondes

  if (millis() - lastTurnTime > turnDuration) {
    if (turningLeft) {
      panPos += panIncrement;
      if (panPos > panMax) {
        panPos = panMax;
        turningLeft = false;
      }
    } else {
      panPos -= panIncrement;
      if (panPos < panMin) {
        panPos = panMin;
        turningLeft = true;
      }
    }

    if (tiltingUp) {
      tiltPos += tiltIncrement;
      if (tiltPos > tiltMax) {
        tiltPos = tiltMax;
        tiltingUp = false;
      }
    } else {
      tiltPos -= tiltIncrement;
      if (tiltPos < tiltMin) {
        tiltPos = tiltMin;
        tiltingUp = true;
      }
    }

    pixy.setServos(panPos, tiltPos);
    lastTurnTime = millis();
  }
}

void loop() {
  // Récupération des blocs détectés par la Pixy2
  pixy.ccc.getBlocks();

  if (pixy.ccc.numBlocks) {
    bool objectFound = false;
    // Chercher le bloc ayant la signature spécifique
    for (int i = 0; i < pixy.ccc.numBlocks; i++) {
      if (pixy.ccc.blocks[i].m_signature == signatureToTrack) {
        objectFound = true;
        // Si un bloc avec la signature spécifique est détecté
        int x = pixy.ccc.blocks[i].m_x;
        int width = pixy.ccc.blocks[i].m_width;

        // Calculez la position centrale de l'image
        int imageCenter = pixy.frameWidth / 2;

        // Distance approximative de l'objet (basée sur la largeur du bloc)
        if (width < closeDistance) {
          forward();
        } else {
          stopRobot();
        }

        // Direction (à ajuster si nécessaire)
        if (x < imageCenter - centerThreshold) {
          turnLeft();
        } else if (x > imageCenter + centerThreshold) {
          turnRight();
        } else {
          // Si l'objet est centré, avancer tout droit
          forward();
        }
        break; // Sortir de la boucle une fois le bloc trouvé
      }
    }
    if (!objectFound) {
      // Si aucun bloc avec la signature spécifique n'est trouvé, effectuer un balayage
      scanForObject();
    }
  } else {
    // Si aucun bloc n'est détecté, effectuer un balayage
    scanForObject();
  }

  delay(100);
}
