#include "AFMotor.h"

#define Broche_Echo A0 // Broche Echo du HC-SR04 sur A0
#define Broche_Trigger A1 // Broche Trigger du HC-SR04 sur A1

#define Distance_seuil 20 // Seuil de distance à partir duquel le robot s'arrête (en cm)

AF_DCMotor motor1(1); // haut gauche
AF_DCMotor motor2(2); // bas gauche
AF_DCMotor motor3(3); // haut droit
AF_DCMotor motor4(4); // bas droit

int MesureMaxi = 300; // Distance maxi a mesurer
int MesureMini = 3; // Distance mini a mesurer
long Duree;
long Distance;

void setup() {
  pinMode(Broche_Trigger, OUTPUT); // Broche Trigger en sortie
  pinMode(Broche_Echo, INPUT); // Broche Echo en entree
  Serial.begin(9600);
  motor1.setSpeed(255);
  motor2.setSpeed(255);
  motor3.setSpeed(255);
  motor4.setSpeed(255);
}

void loop() {
  // Mesure de la distance
  digitalWrite(Broche_Trigger, LOW);
  delayMicroseconds(2);
  digitalWrite(Broche_Trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(Broche_Trigger, LOW);
  Duree = pulseIn(Broche_Echo, HIGH);
  Distance = Duree * 0.034 / 2;
      droit();
  // Contrôle des moteurs en fonction de la distance
  if (Distance >= MesureMaxi || Distance <= MesureMini) {
    Serial.println("Distance de mesure en dehors de la plage (3 cm à 3 m)");
  } else {
    Serial.print("Distance mesuree :");
    Serial.print(Distance);
    Serial.println("cm");
    
    if (Distance <= Distance_seuil) {
      // Arrêter les moteurs si un mur est détecté
      motor1.run(RELEASE);
      motor2.run(RELEASE);
      motor3.run(RELEASE);
      motor4.run(RELEASE);
    } else {
      // Avancer si aucune obstacle n'est détecté
      droit();
      
    }
  }
  delay(100); // Ajouter un court délai entre chaque itération de la boucle
 

}
void droit(){
  motor1.setSpeed(255); //Define maximum velocity
  motor1.run(FORWARD);  //rotate the motor clockwise
  motor2.setSpeed(255); //Define maximum velocity
  motor2.run(BACKWARD);  //rotate the motor clockwise
  motor3.setSpeed(255); //Define maximum velocity
  motor3.run(BACKWARD); //rotate the motor anti-clockwise
  motor4.setSpeed(255); //Define maximum velocity
  motor4.run(FORWARD); //rotate the motor anti-clockwise
}
 void forward()
{
      motor1.run(FORWARD);
      motor2.run(BACKWARD);
      motor3.run(FORWARD);
      motor4.run(BACKWARD);
}