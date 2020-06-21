// AUTHOR: Federico Terzi
// Arduino sketch for the gesture controlled keyboard.
// This sketch is the BASIC version, and does not include Bluetooth capabilities,
// If you want Bluetooth, use the normal version instead.
// Part of this sketch is taken from an example made by JohnChi

#include <Wire.h>


#include <MPU6050_tockn.h>
MPU6050 mpu6050(Wire);
long timer = 0;



// Pins used for I/O
const int btnPin1 = PIN_SW0;
const int ledPin    = PIN_LED_13; // the number of the LED pin

// I2C address of the MPU-6050
const int MPU_addr=0x68;
// Variables that will store sensor data
float AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

// Status variables, used with buttons
int precBtn1 = HIGH;


int8_t mode = 0;
int8_t co = 0;

float tr_accx[50];
float tr_accy[50];
float tr_accz[50];

float te_accx[50];
float te_accy[50];
float te_accz[50];

float DTW[50][50];

float dx = 0;
float dy = 0;
float dz = 0;


float threshold = 0;

void setup(){
  // Set the pin mode of the buttons using the internal pullup resistor
  pinMode(btnPin1, INPUT_PULLUP);

  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);

  // Start the comunication with the MPU-6050 sensor
  Wire.begin();
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x6B);  // PWR_MGMT_1 register
  Wire.write(0);     // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // Start the serial communication
  Serial.begin(38400);

  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}
void loop(){
  // turn off LED ,HIGH is off
  digitalWrite(ledPin, HIGH); 
  // Read the values of the buttons
  int resultBtn1 = digitalRead(btnPin1);

  if(mode == 0 && co == 0){
    Serial.println("\nPress botton to record training data");
    co++;
  }
  
  // ON btn1 pressed, start the batch and light up the yellow LED
  if (precBtn1 == HIGH && resultBtn1 == LOW)
  {
    mode++;
    Serial.print("\nMODE:");
    Serial.println(mode);
    delay(500);
  }

  // If the btn1 is pressed, reads the data from the sensor and sends it through the communication channel
  if (mode==1)
  {   
      Serial.println("RECORD FOR TRAINING PART 1: ");
      //turn on LED lights
      digitalWrite(ledPin, LOW); 
      mode1();
      mode++;
      Serial.println("Press botton to record test data to compute DTW threshold");
  }

  if (mode == 3)
  {
      Serial.println("RECORD FOR TRAINING PART 2: ");
      digitalWrite(ledPin, LOW);
      mode3();
      mode++;
      Serial.println("\nPress botton to comptute DTW threshold value");
  }

  if(mode == 5){
    digitalWrite(ledPin, LOW);
    threshold = DTW_THRESHOLD();
    mode++;
    Serial.print("\nThreshold is ");Serial.println(threshold);
    Serial.println("\nPRESS TO START TEST");
  }

  if(mode >= 7 && (mode % 2) == 1){
    digitalWrite(ledPin, LOW);
    Serial.print("TEST ");Serial.print(mode / 2 - 2);Serial.println(" START");
    mode3();
    
    Serial.println("waiting for DTW computation...");
    
    float var = DTW_THRESHOLD();
    if ( (var >= 0 && var <= threshold + 5) ) { //gesture recognized
      Serial.print("\nGESTURE OF TEST ");Serial.print(mode / 2 - 2);Serial.println(" RECOGNIZED");
      digitalWrite(ledPin, HIGH);
      delay(1000);
      digitalWrite(ledPin, LOW);
      delay(500);
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
      digitalWrite(ledPin, HIGH);
      delay(500);
      digitalWrite(ledPin, LOW);
      delay(500);
      digitalWrite(ledPin, HIGH);
      
    }
    else {
      Serial.print("\nGESTURE OF TEST ");Serial.print(mode / 2 - 2);Serial.println(" UNRECOGNIZED");
    }

    Serial.println("\nPRESS TO START NEXT TEST");
    mode++;
  }
  
  precBtn1 = resultBtn1;
}

void mode1(){
    Serial.println("TEMPLATE RECORD START: PLEASE RECORD IN 1.5s!!!!!!!!");
    for (int i = 0 ; i < 50 ; i++){
      delay(30); //adjust delay in mode1() and mode3() to get different time length for recording a single gesture
      mpu6050.update();
      tr_accx[i] = 200*mpu6050.getAccX();
//      Serial.println(tr_accx[i]);  
      tr_accy[i] = 200*mpu6050.getAccY(); 
//      Serial.println(tr_accy[i]);
      tr_accz[i] = 200*mpu6050.getAccZ(); 
//      Serial.println(tr_accz[i]);
//      Serial.print(i);Serial.println(" data point has been recorded");
    }
    Serial.println("TEMPLATE RECORD FINISHED\n");
    // turn off LED ,HIGH is off
    digitalWrite(ledPin, HIGH); 
    
}

void mode3(){
    Serial.println("COMPARISON RECORD START: PLEASE RECORD IN 1.5s!!!!!!!!");
    for (int8_t i = 0 ; i < 50 ; i++){
      delay(30); //adjust delay in mode1() and mode3() to get different time length for recording a single gesture
      mpu6050.update();
      te_accx[i] = 200*mpu6050.getAccX();
//      Serial.println(te_accx[i]);   
      te_accy[i] = 200*mpu6050.getAccY();
//      Serial.println(te_accy[i]);  
      te_accz[i] = 200*mpu6050.getAccZ(); 
//      Serial.println(te_accz[i]); 
//      Serial.print(i);Serial.println(" data point has been recorded\n");
    }
    Serial.println("COMPARISON RECORD FINISHED");
    // turn off LED ,HIGH is off
    digitalWrite(ledPin, HIGH); 
    
}

float DTW_THRESHOLD(){
    Serial.println("\nSTART DTW");
    Serial.println("DTW COMPUTING...");
    float scale = 0.5;
    float dir = 0; 
    //------------process boundary:--------------
    for(int8_t i = 0 ; i < 50 ; i++){

      if (i == 0){
        dx = tr_accx[i] - te_accx[i];
        dy = tr_accy[i] - te_accy[i];
        dz = tr_accz[i] - te_accz[i];
        dir = (tr_accx[i]*te_accx[i] + tr_accy[i]*te_accy[i] + tr_accz[i]*te_accz[i])/ ( NORM(tr_accx[i],tr_accy[i],tr_accz[i])* NORM(te_accx[i],te_accy[i],te_accz[i]) + 0.0000001);
        DTW[i][i] = (1-scale*dir) * NORM(dx,dy,dz);  
      }
      else{
        dx = tr_accx[i] - te_accx[0];
        dy = tr_accy[i] - te_accy[0];
        dz = tr_accz[i] - te_accz[0];
        dir = (tr_accx[i]*te_accx[0] + tr_accy[i]*te_accy[0] + tr_accz[i]*te_accz[0])/ ( NORM(tr_accx[i],tr_accy[i],tr_accz[i])* NORM(te_accx[0],te_accy[0],te_accz[0]) + 0.0000001);
        DTW[i][0] = (1-scale*dir) * NORM(dx,dy,dz) + DTW[i-1][0];

        dx = tr_accx[0] - te_accx[i];
        dy = tr_accy[0] - te_accy[i];
        dz = tr_accz[0] - te_accz[i];
        dir = (tr_accx[0]*te_accx[i] + tr_accy[0]*te_accy[i] + tr_accz[0]*te_accz[i])/ ( NORM(tr_accx[0],tr_accy[0],tr_accz[0])* NORM(te_accx[i],te_accy[i],te_accz[i]) + 0.0000001);
        DTW[0][i] = (1-scale*dir) * NORM(dx,dy,dz) + DTW[0][i-1];
      }
//      Serial.print("\nBoundary: ");Serial.print("tr_acc ");Serial.print(i);Serial.print("/0");Serial.print(" and");Serial.print(" te_acc ");Serial.print("0/");Serial.print(i);Serial.println(" computed");
    }

    //---------------computing:-----------------
    for(int8_t i = 1 ; i < 50 ; i++){
      for(int8_t j = 1 ; j < 50 ; j++){
        dx = tr_accx[i] - te_accx[j];
        dy = tr_accy[i] - te_accy[j];
        dz = tr_accz[i] - te_accz[j];
//        Serial.println(costx);
//        Serial.println(costy);
//        Serial.println(costz);
        dir = (tr_accx[i]*te_accx[j] + tr_accy[i]*te_accy[j] + tr_accz[i]*te_accz[j])/ ( NORM(tr_accx[i],tr_accy[i],tr_accz[i])* NORM(te_accx[j],te_accy[j],te_accz[j]) + 0.0000001);  
        DTW[i][j] = (1-scale*dir) * NORM(dx,dy,dz) + MIN(DTW[i-1][ j  ],DTW[i  ][ j-1],DTW[i-1][ j-1]);    
  
//        Serial.println(DTW[i][j]);
//        Serial.print("tr_acc ");Serial.print(i);Serial.print(" and");Serial.print(" te_acc ");Serial.print(j);Serial.println(" computed");
      }
    }

    //---------------back track:----------------
    Serial.println("Back track start:");
//    delay(1000);
    int8_t i = 49;
    int8_t j = 49;
    int8_t count = 0;
    float d[100];
    while(true){
      if(i>0 && j>0){
        float m = MIN(DTW[i-1][ j  ],DTW[i  ][ j-1],DTW[i-1][ j-1]);
        if(m == DTW[i-1][j-1]){
          d[count] = DTW[i][j] - DTW[i-1][j-1];
          i = i-1;
          j = j-1;
          count++;
        }
        else if(m == DTW[i][j-1]){
          d[count] = DTW[i][j] - DTW[i][j-1];
          j = j-1;
          count++;
        }
        else if(m == DTW[i-1][ j  ]){
          d[count] = DTW[i][j] - DTW[i-1][j];
          i = i-1;
          count++;
        }
      }
      else if(i == 0 && j == 0){
        d[count] = DTW[i][j];
        count++;
        break;
      }
      else if(i == 0){
        d[count] = DTW[i][j] - DTW[i][j-1];
        j = j-1;
        count++;
      }
      else if(j == 0){
        d[count] = DTW[i][j] - DTW[i-1][j];
        i = i-1;
        count++;
      }
    }
    Serial.print("Back track finished, ");Serial.print(count);Serial.println(" steps used in back track");
//    uncomment to enable this:   
//    limit repeated matching points in bakc track route when testing, good for reject wrong guesture but affect right gesture a little some time  
//    if (mode >= 7 && count > 62){ 
//      Serial.println("Back track constraint violated!!");
//      return -1;
//    }
    //---------------compute variance-----------------
    Serial.println("Cost function computation start");
//    delay(1000);
    float mean = 0;
    for (int i = 0 ; i < count ; i++){
//      Serial.print("The ");Serial.print(i);Serial.print(" difference: ");Serial.println(d[i]);
      mean += d[i];
    }
    mean = mean / count;
//    float variance = 0;
//    for (int i = 0 ; i < count ; i++){
//      variance += (d[i]-mean)*(d[i]-mean)/count;
//    }
    Serial.print("Threshold is: ");Serial.println(threshold);
    Serial.print("Cost function value: ");Serial.println(mean);
    return mean;
}

float MIN(float &a, float &b ,float &c){
  if (a < b){
    if (a < c)return a;
    else return c;
  }
  else {
    if (b < c)return b;
    else return c;
  }
}

float NORM( float dx, float dy, float dz){
  return sqrt(dx*dx + dy*dy + dz*dz);
}
