#include <Stepper.h>
Hi!!!
#define STEPS_PER_REV 2048

Stepper motor1(STEPS_PER_REV, 13,14,12,27);
Stepper motor2(STEPS_PER_REV, 26,33,25,32);
Stepper motor3(STEPS_PER_REV, 23,21,22,19);
Stepper motor4(STEPS_PER_REV, 18,4,5,2);

float resistance[4];
float maxResistance[4] = {1000000.0, 100000.0, 10000.0, 5000.0};

int targetSteps[4];

void clearBuffer(){
  while(Serial.available())
    Serial.read();
}

void setup() {
  Serial.begin(115200);

  motor1.setSpeed(10);
  motor2.setSpeed(10);
  motor3.setSpeed(10);
  motor4.setSpeed(10);
  pinMode(15,INPUT);
  Serial.println("Enter 4 Resistance Values:");
}

void loop() {

  // 👉 INPUT
        Serial.println("Enter 4 Resistance Values (space separated):");

      // Wait until full line arrives
      while(Serial.available() == 0);

      String input = Serial.readStringUntil('\n');

      // Parse manually
      int index = 0;
      char *ptr = strtok((char*)input.c_str(), " ");

      while(ptr != NULL && index < 4){
        resistance[index] = atof(ptr);
        ptr = strtok(NULL, " ");
        index++;
      }

      // Safety check
      if(index < 4){
        Serial.println("Invalid Input! Enter 4 values.");
        return;
      }

  // 👉 CALCULATE TARGET STEPS
  for(int i=0;i<4;i++){
    int angle = resistance[i] * (300.0 / maxResistance[i]);
    Serial.print(angle);
    Serial.print(" ");
    targetSteps[i] = angle / 0.17578125;

  }
  Serial.println();
  Serial.println();
  for(int i=0;i<4;i++){

    Serial.print(targetSteps[i]);
    Serial.print(" ");
   
  }
   Serial.println();
   Serial.println();
   for(int i=0;i<4;i++){
     Serial.print(resistance[i]);
    Serial.print(" ");
    
  }
  Serial.println();

  Serial.println("Moving Motors to Target Position...");

  // 👉 FORWARD MOVEMENT (SEQUENTIAL)
  motor1.step(targetSteps[0]);
  disableAllMotors();
  motor2.step(targetSteps[1]);
  disableAllMotors();
  motor3.step(targetSteps[2]);
  disableAllMotors();
  motor4.step(targetSteps[3]);
  disableAllMotors();
  
  Serial.println("Target Position Reached!");

  // 👉 WAIT FOR STOP COMMAND
  Serial.println("Press S to return to HOME");

  while(true){
    if(Serial.available()){
      char cmd = Serial.read();

      if(cmd=='S' || cmd=='s'){
        Serial.println("Returning to Initial Position...");

        // 👉 RETURN BACK (SEQUENTIAL)
        /*motor1.step(-targetSteps[0]);
        disableAllMotors();
        motor2.step(-targetSteps[1]);
        disableAllMotors();
        motor3.step(-targetSteps[2]);
        disableAllMotors();
        motor4.step(-targetSteps[3]);
        disableAllMotors();*/
        while(digitalRead(15) == 0)
        {
            motor1.step(-1);
        }
        // 👉 RESET VALUES
        for(int i=0;i<4;i++){
          targetSteps[i] = 0;
          resistance[i] = 0;
        }

        Serial.println("All Motors Back to Home!");
        disableAllMotors();
        break;
      }
    }
  }
}

// 👉 Disable all motors
void disableAllMotors(){
  digitalWrite(13,LOW); digitalWrite(14,LOW); digitalWrite(12,LOW); digitalWrite(27,LOW);
  digitalWrite(26,LOW); digitalWrite(33,LOW); digitalWrite(25,LOW); digitalWrite(32,LOW);
  digitalWrite(22,LOW); digitalWrite(21,LOW); digitalWrite(23,LOW); digitalWrite(19,LOW);
  digitalWrite(18,LOW); digitalWrite(4,LOW); digitalWrite(5,LOW); digitalWrite(2,LOW);
}
