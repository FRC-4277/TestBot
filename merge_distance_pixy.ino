#include <Pixy2.h>
#include <Wire.h>

// INITIAL VARIABLES FOR DISTANCE

int avg_range1 = 0; // Average range from one distance sensor (initial = 0)
int avg_range2 = 0; // Average range from other distance sensor (initial = 0)
/*@4*/int angle = 0; // Angle (initial = 0)
int angle_adjacent = 12; // Distance between sensors in cm
int max_range = 100; // Maximum sensing range in cm
int min_range = 10; // Minimum sensing range in cm (set to distance from sensor to front bumper)
int max_detectable_angle = 20;
long duration;
long distance_data[2][5];
/* Distance Pins */
int sensor1 = 4;
int sensor2 = 2;

// INITIAL VARIABLES FOR PIXY
Pixy2 pixy;
/*@1*/String x = "-1";
/*@2*/String y = "-1";
/*@3*/String areaString = "-1";

// OUTPUT TO RIO
String rioOutput = "";

void setup() {
  // put your setup code here, to run once:
  setupDistance();
  setupPixy();
  // Serial Setup
  Serial.begin(9600); // 9600 bps
  // I2C Setup
  Wire.begin(4);
  Wire.onRequest(requestI2CEvent);
}

void setupDistance() {
  pinMode(sensor1, INPUT);
  pinMode(sensor2, INPUT);
}

void setupPixy() {
  pixy.init();
}

void loop() {
  // put your main code here, to run repeatedly:
  distanceLoop();
  pixyLoop();
  outputLoop();
  delay(70);
}

void distanceLoop() {
  /* Sensor One */
  distance_data[0][4] = distance_data[0][3];
  distance_data[0][3] = distance_data[0][2];
  distance_data[0][2] = distance_data[0][1];
  distance_data[0][1] = distance_data[0][0];
  /* Sensor 2*/
  distance_data[1][4] = distance_data[1][3];
  distance_data[1][3] = distance_data[1][2];
  distance_data[1][2] = distance_data[1][1];
  distance_data[1][1] = distance_data[1][0];
  // Detect Sensor 1 Start
  pinMode(sensor1, OUTPUT);
  digitalWrite(sensor1, HIGH);
  delayMicroseconds(20);
  digitalWrite(sensor1, LOW);

  // Detect Sensor 1 - Read Pulse
  pinMode(sensor1, INPUT);
  duration = pulseIn(sensor1, HIGH);
  distance_data[0][0] = (duration/2) / 29.1;
  //Detect Sensor 1  End
  
  delay(30);

  // Detect Sensor 2 Start
  pinMode(sensor2, OUTPUT);
  digitalWrite(sensor2, HIGH);
  delayMicroseconds(20);
  digitalWrite(sensor2, LOW);

  // Detect Sensor 2 - Read Pulse
  pinMode(sensor2, INPUT);
  duration = pulseIn(sensor2, HIGH);
  distance_data[1][0] = (duration/2) / 29.1;
  
  //Detect Sensor 2 End
  
  // Calculate Average Range over 5 readings
  avg_range1 = ((distance_data[0][0] + distance_data[0][1] + distance_data[0][2] + distance_data[0][3] + distance_data[0][4]) /5) ;
  avg_range2 = ((distance_data[1][0] + distance_data[1][1] + distance_data[1][2] + distance_data[1][3] + distance_data[1][4]) /5) ;
  // Calculate Angle
  angle = round( atan2 ((avg_range1 - avg_range2), angle_adjacent) * 180/3.14159265 ); // radians to degrees and rounding  
  /*Serial.println("====");
  Serial.print("Average Range 1: ");
  Serial.println(avg_range1);
  Serial.print("Average Range 2: ");
  Serial.println(avg_range2);
  Serial.print("Angle: ");
  Serial.println(angle);*/
}

void pixyLoop() {
  uint16_t blocks = pixy.ccc.getBlocks();//use this line to get every available object the pixy sees
  //^^^not sure what exactly this is for, honestly
  int biggest = -1;
  double area = 0, temp;
  for(int i=0;i<blocks;i++){
    //if(pixy.blocks[i].signature == 3) //if checking for an object and have more than one "type" or color to choose from
                         //use this line and choose the signature or "color" you want
      temp = pixy.ccc.blocks[i].m_width * pixy.ccc.blocks[i].m_height;
      if(temp > area){
        area = temp;
        biggest = i;
      }
      
      //that loops finds the biggest object since sometimes the object you are looking for becomes 
      //broken up into multiple, smaller, objects
  }
  if (blocks) {
     x = String(pixy.ccc.blocks[biggest].m_x);
     y = String(pixy.ccc.blocks[biggest].m_y);
     areaString = String(area);
  }
}

void outputLoop() {
  rioOutput = "";
  rioOutput += x;
  rioOutput += "|";
  rioOutput += y;
  rioOutput += "|";
  rioOutput += areaString;
  rioOutput += "|";
  rioOutput += angle;
  Serial.print(rioOutput);
}

void requestI2CEvent() {
   Wire.write(rioOutput.c_str());
}
