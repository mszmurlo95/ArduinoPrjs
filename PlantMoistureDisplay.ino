/*
 *This code is intended to work with the Linksprite 1602 16x2 liquid crystal display with keypad.
 *Linksprite wiki: https://www.linksprite.com/wiki/index.php5?title=16_X_2_LCD_Keypad_Shield_for_Arduino_V2
 *Please ensure you have the LiquidCrystal library installed for this code to work as intended.
 *Display analog input data from a maximum of 5 SparkFun soil moisture sensors https://www.sparkfun.com/products/13322
 *Each sensor is tied to one digital output in this code. The digital output turns on after a very 
 *long time to preserve lifetime of sensor.
 *Main menu features currently only supports toggling of backlight and returning back to plant(s) display screen
 *
 *This program is inteded to be free to use as you wish and can be redistributed and/or modified to your hearts content.
 *
 *CAUTION: if you are changing the number of sensors, be sure to update displaySize to size of plantNamePointer.
 *
 */

#include <LiquidCrystal.h>
//define liquid crystal digital pins

const int lcdRS{8}, lcdEnable{9}, lcdD4{4}, lcdD5{5}, lcdD6{6}, lcdD7{7};
LiquidCrystal lcd(lcdRS, lcdEnable, lcdD4, lcdD5, lcdD6, lcdD7);

const int backlight{10};
const int displaySize {5};
int moistureArray [displaySize] {};
bool initilizer {0}; //needed for first loop for transition into displays.
char *plantNamePointer [] {"Ficus", "Fiddle", "Fern", "Monstera","Umbrella"};

bool modeSelect{1}; //if power is cycled default into display screen
int plantDisplaySelect{0};
bool sample{0};

unsigned long previousTime{0}; 
unsigned long previousTime2{0};
unsigned long previousTime3{0};
const unsigned long displayInterval{1000};
const unsigned long sensorEnableInterval{3580000}; //power sensors every hour
const unsigned long sensorUpdateInterval{3600000}; //lag reading by 20 seconds of enable

//{3580000} 59 minutes and 40 seconds
//{3600000} 1 hour

//define analog inputs

int moist1Ain = A1;
int moist2Ain = A2;
int moist3Ain = A3;
int moist4Ain = A4;
int moist5Ain = A5;

//define digital output for sensor power enable

int moist_EnableCmd {2};

// String sensorA1LiveData{};
// static bool an1LiveData{0};


//function prototypes

bool toggleMenu ();
bool toggleBacklight ();
int plantNameToDisplay (int incSize = displaySize);
int moistureAdcToPct(int AIN);
int saturation(int val, int upper, int lower);
double interpolateToPct(double Y1, double Y2, double signal);
double averagingFilter(int AIN, int bufferSize);

//function definitions

bool toggleMenu (){
  static int count {1}; //hold previous value with function call
  count++;
  if (count % 2 == 1)
    return 0;
  else
    return 1;

}

bool toggleBacklight (){
  static int count {0};
  count++;
  if (count % 2 == 1)
    return 0;
  else
    return 1;
  
}

int plantNameToDisplay (int incSize){
  static int inc{0};
  inc++;
  if(inc < incSize){
    return inc;
  }else{
    inc = 0;
    return inc;
  }
}

double averagingFilter(int bufferSize, int AIN){

  int tempAdcArray [bufferSize] {}; //create temp empty array of bufferSize
  for(int i{}; i < bufferSize; ++i){
    tempAdcArray[i] = analogRead(AIN); //assign tempAdcArray to analog measurement each iteration
  }
  int sum{0};
  for(int i{}; i < bufferSize; ++i){
    sum += tempAdcArray[i]; //sum all elements in the tempAdcArray and assign to sum variable
  }
  return (sum / bufferSize);
}

int saturation(int val, int upper, int lower){
  if(val >=upper)
    return upper;
  else if (val <= lower)
    return lower;
  else
    return val;
}

double interpolateToPct(double Y1, double Y2, double signal){
  double val = (100 * (signal - Y1))/(Y2-Y1);
  return val;
}

int moistureAdcToPct(int AIN){
   
  double averageFilteredData = averagingFilter(30, AIN);
  int moistPct = static_cast<int>(interpolateToPct(200.0, 852.0, averageFilteredData));
  int moisture = saturation(moistPct, 100, 0);
  return moisture;
  
}

//setup

void setup() {

  Serial.begin(9600);
  // Serial.println("Enter one of the following commands:");
  // Serial.println("************************");
  // Serial.println("testmode");
  // Serial.println("************************");

  

  pinMode(backlight, OUTPUT);
  pinMode(moist_EnableCmd, OUTPUT);
  digitalWrite(moist_EnableCmd, LOW);
  digitalWrite(backlight, LOW); //default display backlight low on power cycle

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  
}

//main loop

void loop() {

  
  int val = analogRead(A0);
  bool selectButton = (val>=735 && val <=745);
  bool leftButton = (val>=500 && val <=510);
  bool rightButton = (val>=-5 && val<=10);
  bool upButton = (val>=140 && val <=150);
  bool downButton = (val>= 325 && val <= 335);
  
  unsigned long currentTime = millis(); 

  
  // if(Serial.available()){
  //   sensorA1LiveData = Serial.readStringUntil('\n');
  //   if(sensorA1LiveData == "testmode"){
  //     an1LiveData = 1;
  //   }
  // }
  
  
  
  // if(an1LiveData == 1){
  //   digitalWrite(moist_EnableCmd, HIGH);
  //   Serial.println(moistureAdcToPct(moist1Ain));
  //   delay(500);
  // }
  

  // Serial.println(interpolateToPct(200, 852, 800));

  if (selectButton){ //analog value of select button
    lcd.clear();
    modeSelect = toggleMenu(); //call toggleMenu function
    delay(500); //Needed to delay button press to prevent many function calls.
    // Serial.println(modeSelect);
  }

//main menu

  if (modeSelect == 0){
    initilizer = 1;
      
      if(currentTime - previousTime >= displayInterval || initilizer == 0){
        lcd.clear();
        lcd.print("[Home]  Plants->");
        lcd.setCursor(0,1);
        lcd.print("<-Light    Cal->");
        previousTime = currentTime; //remember the time
      }

      if(leftButton){    //if left button is pressed within main menu
        bool backlightSelect = toggleBacklight();
        delay(500);
        Serial.println(backlightSelect);
        if(backlightSelect == false){
          digitalWrite(backlight, LOW);
          lcd.clear();
          lcd.print("Backlight Off _*");
          delay(2000);
          
        }else if (backlightSelect == true){
          digitalWrite(backlight, HIGH);
          lcd.clear();
          lcd.print("Backlight ON *");
          delay(2000);
        }
      }

      // if(rightButton){   //if right button is pressed within main menu
      //   bool calMode{true}; //set true for do while loop
      //   lcd.clear(); //clear once

      //   do{

          

      //   //allow to select plant number upon button press up or down
      //   //once plant number is selected, allow to store upper bound and display stored ADC value for 3 seconds respectivly
      //   //upper bound would be the sensor placed in perfectly wet but drained soil sampled from specific plant
      //   //lower bound would be sensor placed in perfectly dry soil sampled from specific plant
      //   //the upper and lower bound should be stored into EEPROM to preserve on power cycles
      //   //ensure this only gets written once when a button is toggled
          
          
      //   }while(calMode == true);

  
      // }
      
//plant moisture display

  }else{
    
    if(currentTime - previousTime >= displayInterval*3 || initilizer){
      plantDisplaySelect = plantNameToDisplay(); //calls plantNameToDisplay() function and sets equal to variable
      lcd.clear();
      lcd.print(plantNamePointer[plantDisplaySelect]); //prints plant name
      lcd.print(": ");
      lcd.print(moistureArray[plantDisplaySelect]); //prints percent value
      lcd.print("% ");
      if(moistureArray[plantDisplaySelect] < 40){
        lcd.setCursor(0,1);
        lcd.print("   Water Me!");
      }
      initilizer = 0;
      previousTime = currentTime; //remember the time
      }

    if(currentTime - previousTime2 >= sensorEnableInterval){
      digitalWrite(moist_EnableCmd, HIGH);
      

      previousTime2 = currentTime;

    }

    if(currentTime - previousTime3 >= sensorUpdateInterval){
      moistureArray[0] = moistureAdcToPct(moist1Ain);
      moistureArray[1] = moistureAdcToPct(moist2Ain);
      moistureArray[2] = moistureAdcToPct(moist3Ain);
      moistureArray[3] = moistureAdcToPct(moist4Ain);
      moistureArray[4] = moistureAdcToPct(moist5Ain);
      digitalWrite(moist_EnableCmd, LOW);
      
      previousTime3 = currentTime; //remember the time
      
    }


    }
        
  }

  
  
    





