 //#include <Arduino.h>
 #include <SD.h>
 #include <openGLCD.h>
 #include <Adafruit_MAX31856.h>
 #include <PID_v1.h>
 //#include <Adafruit_MAX31855.h>

//Define static variables
#define B_UP          11
#define B_DOWN        10
#define B_ENTER       13
#define B_BACK        14
#define BUFFERSIZE    32
#define RELAY1        0
#define RELAY2        1
#define PART_SENSOR1  A6
#define PART_SENSOR2  A7
#define AIR_SENSOR1   2
#define AIR_SENSOR2   3
#define P_SENSOR      A4
#define SPEAKER       12
#define DELTA_T       50
#define PREHEAT_TEMP  120

//Misc global variables
byte prevMenu = 1;
byte pageNumber = 1;
byte pageNumber2 = 1;
byte totalCycles = 0;
byte totalData = 0;
byte indexMain = 2;
byte indexCC = 0;
byte indexCCO = 2;
byte indexData = 0;
byte indexDLO = 2;
String currentFile;
String currentName;
char timeBuff[9];

//Button state flags. Used for detecting button presses.
bool lastButton_up = HIGH;
bool currentButton_up = HIGH;
bool lastButton_enter = HIGH;
bool currentButton_enter = HIGH;
bool lastButton_down = HIGH;
bool currentButton_down = HIGH;
bool lastButton_back = HIGH;
bool currentButton_back = HIGH;

void setup() {
  Serial.begin(9600);
  //Init peripherals
  GLCD.Init();
  GLCD.SelectFont(Iain5x7);
  
  //Set uC pin modes
  pinMode(B_UP, INPUT_PULLUP);
  pinMode(B_DOWN, INPUT_PULLUP);
  pinMode(B_ENTER, INPUT_PULLUP);
  pinMode(B_BACK, INPUT_PULLUP);
  pinMode(SS, OUTPUT);
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(PART_SENSOR1, OUTPUT);
  pinMode(PART_SENSOR2, OUTPUT);
  pinMode(AIR_SENSOR1, OUTPUT);
  pinMode(AIR_SENSOR1, OUTPUT);
  //Initialize with oven heating elements off
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  digitalWrite(PART_SENSOR1, HIGH);
  digitalWrite(PART_SENSOR2, HIGH);
  digitalWrite(AIR_SENSOR1, HIGH);
  digitalWrite(AIR_SENSOR2, HIGH);
  
  //Print Title Screen
  GLCD.ClearScreen();
  GLCD.CursorTo(7, 3);
  GLCD.print("Oven M.I.T.T.");
  GLCD.CursorTo(10, 4);
  GLCD.print("v  0.6");
  //Check for SD Card
  if (!SD.begin()) {
    GLCD.CursorTo(0, 7);
    GLCD.print("No SD Card Found.");
    while(1);    
  }

  //Check if cycle directory exists. If no, create it
  File cycleDir;
  cycleDir = SD.open("/cycles");
  if(!cycleDir) {
    SD.mkdir("/cycles");
    cycleDir = SD.open("/cycles");
    GLCD.CursorTo(0,5);
    GLCD.print("Creating Cycle Directory");
    delay(500);
  }
    
  //Count number of cure cycles on SD Card
  while(1) {
    File entry = cycleDir.openNextFile();
    if(!entry) { break; }
    totalCycles++;
    entry.close();
  }
  cycleDir.close();

  //Check if data directory exists. If no, create it
  File dataDir;
  dataDir = SD.open("/data");
  if(!dataDir) {
    SD.mkdir("/data");
    dataDir = SD.open("/data");
    GLCD.CursorTo(0,6);
    GLCD.print("Creating Data Directory");
    delay(500);
  }
  
  //Count number of data logs on SD Card
  while(1) {
    File entry = dataDir.openNextFile();
    if(!entry) { break; }
    totalData++;
    entry.close();
  }
  dataDir.close();

  //Create Temp Directory if it doesn't exist, else clean the directory
  File tempFile;
  tempFile = SD.open("/temp");
  if(!tempFile) {
    SD.mkdir("/temp");
    GLCD.CursorTo(0,7);
    GLCD.print("Creating Temp Directory");
    delay(500);
  } else {
    //Clean the temp directory
    int totalFiles = 0;
    while(1) {
      File entry = tempFile.openNextFile();
      if(!entry) { break; }
      totalFiles++;
      entry.close();
    }
    tempFile.rewindDirectory();
    String fileArray[totalFiles];
    for(int i = 0; i < totalFiles; i++) {
        File entry = tempFile.openNextFile();
        fileArray[i] = entry.name();  
        entry.close();      
    }
    for(int j = 0; j < totalFiles; j++) {
      SD.remove("/temp/" + fileArray[j]);
    } 
  }
  tempFile.close();

  //Create log Directory if it doesn't exist  
  File logDir;
  logDir = SD.open("/log");
  if(!logDir) {
    SD.mkdir("/log");
    logDir = SD.open("/log");
  }
  logDir.close();  
  
  delay(1000);
  GLCD.ClearScreen();
 }

 void loop() {
  switch (prevMenu) {
    case (1):
      mainMenu();
      break;
    case (2):
      cureCycles();
      break;
    case (3):
      cureCycleOptions();
      break;
    case (4):
      dataLog();
      break;
    case (5):
      dataLogOptions();
      break;
    case (6):
      upload();
      break;
    case (7):
      runCycle();
      break;
    case (8):
      viewCycle();
      break;
    case (9):
      deleteCycle();
      break;
    default:
      mainMenu();  
  }
  
}

void mainMenu() {
  GLCD.ClearScreen();
  GLCD.CursorTo(8, 0);  
  GLCD.print("Main  Menu");
  GLCD.CursorTo(2, 2);
  GLCD.print("List  Saved  Cure  Cycles");
  GLCD.CursorTo(2, 3);
  GLCD.print("Upload  New  Cure  Cycle");
  GLCD.CursorTo(2, 4);
  GLCD.print("Data  Log  Management");

  GLCD.CursorTo(0, indexMain);
  GLCD.print("->"); //Initial Location
  prevMenu = 1;
  
  //while(!digitalRead(B_ENTER));
  delay(100);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == LOW && currentButton_up == HIGH) //if it was pressed…
    {
      if(indexMain != 2) {
        GLCD.CursorTo(0, indexMain);
        GLCD.print("   ");
        GLCD.CursorTo(0, indexMain - 1);
        GLCD.print("->");
        --indexMain;
      }
    }
    lastButton_up = currentButton_up; //reset button value

    currentButton_down = digitalRead(B_DOWN); //read button state
    if (lastButton_down == LOW && currentButton_down == HIGH) //if it was pressed…
    {
      if(indexMain != 4) {
        GLCD.CursorTo(0, indexMain);
        GLCD.print("   ");
        GLCD.CursorTo(0, indexMain + 1);
        GLCD.print("->");
        ++indexMain;
      }
    }
    lastButton_down = currentButton_down; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
    {
      switch(indexMain - 1) {
        case(1):
          prevMenu = 2;
          break;
        case(2):
          prevMenu = 6;
          break;
        case(3):
          prevMenu = 4;
          break;
      }
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value
  }

}

void cureCycles() {
    
  int row = 0;
  char title[20];
  
  GLCD.ClearScreen();

  File cycleDir = SD.open("/cycles");
  if (totalCycles != 0) {
    //This loop cycles through the files on previous pages so that the current page can be displayed
    for (int k = 0; k < (pageNumber - 1) * 8; k++) {
      File entry = cycleDir.openNextFile();
      if(!entry) { break; }
      entry.close();
    }
    for (int i = 0; i < 8; i++) {
        File entry = cycleDir.openNextFile();
        if(!entry) { break; }
        entry.read(title, 20);
        GLCD.CursorTo(2, row);
        for(int a = 0; a < 20; a++) {
          GLCD.print(title[a]);
        }
        row++;
        entry.close();
      }

    cycleDir.rewindDirectory();
    if(pageNumber == (((totalCycles - 1) / 8) + 1) && (indexCC + 1) > (totalCycles % 8)) { //For if you delete the last file in the list
      GLCD.CursorTo(0, indexCC - 1);
      GLCD.print("->"); //Initial Location
      indexCC--;  
    } else {
      GLCD.CursorTo(0, indexCC);
      GLCD.print("->"); //Initial Location
    }
    prevMenu = 2;
    
    delay(100);
    lastButton_enter = HIGH;
    currentButton_enter = HIGH;
    lastButton_back = HIGH;
    currentButton_back = HIGH; 
    while(1) {
      currentButton_up = digitalRead(B_UP); //read button state
      if (lastButton_up == LOW && currentButton_up == HIGH) //if it was pressed…
      {
        if(pageNumber == 1) { //if in first page
          if(indexCC != 0) { //if not the first entry
            GLCD.CursorTo(0, indexCC);
            GLCD.print("   ");
            GLCD.CursorTo(0, indexCC - 1);
            GLCD.print("->");
            --indexCC;
          }
        } else { //if not in first page
        if(indexCC != 0) {
          GLCD.CursorTo(0, indexCC);
          GLCD.print("   ");
          GLCD.CursorTo(0, indexCC - 1);
          GLCD.print("->");
          --indexCC;
          } else {
          --pageNumber;
          indexCC = 7;
          break;
          }
        }
      }
      lastButton_up = currentButton_up; //reset button value

      currentButton_down = digitalRead(B_DOWN); //read button state
      if (lastButton_down == LOW && currentButton_down == HIGH) //if it was pressed…
      {
        if(pageNumber == ((totalCycles - 1) / 8) + 1) { //if in last page
          if(indexCC != (totalCycles - 1) % 8) { //if not the last entry
            GLCD.CursorTo(0, indexCC);
            GLCD.print("   ");
            GLCD.CursorTo(0, indexCC + 1);
            GLCD.print("->");
            ++indexCC;
          }
        } else { //if not in last page
        if(indexCC != 7) {
          GLCD.CursorTo(0, indexCC);
          GLCD.print("   ");
          GLCD.CursorTo(0, indexCC + 1);
          GLCD.print("->");
          ++indexCC;
          } else {
          ++pageNumber;
          indexCC = 0;
          break;
          }
        }
      }
      lastButton_down = currentButton_down; //reset button value

      currentButton_enter = digitalRead(B_ENTER); //read button state
      if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
      {
        //This loop cycles through the files on previous pages
        for (int k = 0; k < (pageNumber - 1) * 8; k++) {
          File entry = cycleDir.openNextFile();
          if(!entry) { break; }
          entry.close();
        }
        for (int i = 0; i < indexCC; i++) {
            File entry = cycleDir.openNextFile();
            if(!entry) { break; }
            entry.close();
          }
        File entry = cycleDir.openNextFile();
        entry.read(title, 20);
        
        currentName = "";
        for(int a = 0; a < 20; a++) {
          currentName += title[a];
        }
        
        currentFile = entry.name();
        entry.close();
        prevMenu = 3;
        break;
      }
      lastButton_enter = currentButton_enter; //reset button value

      currentButton_back = digitalRead(B_BACK); //read button state
      if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
      {
        prevMenu = 1;
        break;
      }
      lastButton_back = currentButton_back; //reset button value
    }
  } else {
    GLCD.CursorTo(0, 0);
    GLCD.print("There are no cycles on");
    GLCD.CursorTo(0, 1);
    GLCD.print("the SD Card.");
    delay(2000);
    prevMenu = 1;
  }
  cycleDir.close();
}

void cureCycleOptions() {
  GLCD.ClearScreen();
  GLCD.CursorTo(2, 0);
  GLCD.print(currentName);
  GLCD.CursorTo(2, 2);
  GLCD.print("Run  Cycle");
  GLCD.CursorTo(2, 3);
  GLCD.print("View  Cycle  Instructions");
  GLCD.CursorTo(2, 4);
  GLCD.print("Delete  Cycle");
  
  GLCD.CursorTo(0, indexCCO);
  GLCD.print("->"); //Initial Location
  //prevMenu = 3;
  
  delay(100);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  lastButton_down = HIGH;
  currentButton_down = HIGH;
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == LOW && currentButton_up == HIGH) //if it was pressed…
    {
      if(indexCCO != 2) {
        GLCD.CursorTo(0, indexCCO);
        GLCD.print("   ");
        GLCD.CursorTo(0, indexCCO - 1);
        GLCD.print("->");
        --indexCCO;
      }
    }
    lastButton_up = currentButton_up; //reset button value

    currentButton_down = digitalRead(B_DOWN); //read button state
    if (lastButton_down == LOW && currentButton_down == HIGH) //if it was pressed…
    {
      if(indexCCO != 4) {
        GLCD.CursorTo(0, indexCCO);
        GLCD.print("   ");
        GLCD.CursorTo(0, indexCCO + 1);
        GLCD.print("->");
        ++indexCCO;
      }
    }
    lastButton_down = currentButton_down; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
    {
      switch(indexCCO - 1) {
        case(1):
          prevMenu = 7;
          break;
        case(2):
          prevMenu = 8;
          break;
        case(3):
          prevMenu = 9;
          break;
      }
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value

    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
    {
      prevMenu = 2;
      break;
    }
    lastButton_back = currentButton_back; //reset button value
  }
}

void dataLog() {
  
  int row = 0;
  char title[20];
  
  GLCD.ClearScreen();

  File dataDir = SD.open("/data");
  if (totalData != 0) {
    //This loop cycles through the files on previous pages so that the current page can be displayed
    for (int k = 0; k < (pageNumber2 - 1) * 8; k++) {
      File entry = dataDir.openNextFile();
      if(!entry) { break; }
      entry.close();
    }
    for (int i = 0; i < 8; i++) {
        File entry = dataDir.openNextFile();
        if(!entry) { break; }
        entry.read(title, 20);
        GLCD.CursorTo(2, row);
        for(int a = 0; a < 20; a++) {
          GLCD.print(title[a]);
        }        
        row++;
        entry.close();
      }

    dataDir.rewindDirectory();
    if(pageNumber2 == (((totalData - 1) / 8) + 1) && (indexData + 1) > (totalData % 8)) { //For if you delete the last file in the list
      GLCD.CursorTo(0, indexData - 1);
      GLCD.print("->"); //Initial Location
      indexData--;  
    } else {
      GLCD.CursorTo(0, indexData);
      GLCD.print("->"); //Initial Location
    }
    prevMenu = 2;
    
    delay(100);
    lastButton_enter = HIGH;
    currentButton_enter = HIGH;
    lastButton_back = HIGH;
    currentButton_back = HIGH;
    while(1) {
      currentButton_up = digitalRead(B_UP); //read button state
      if (lastButton_up == LOW && currentButton_up == HIGH) //if it was pressed…
      {
        if(pageNumber2 == 1) { //if in first page
          if(indexData != 0) { //if not the first entry
            GLCD.CursorTo(0, indexData);
            GLCD.print("   ");
            GLCD.CursorTo(0, indexData - 1);
            GLCD.print("->");
            --indexData;
          }
        } else { //if not in first page
        if(indexData != 0) {
          GLCD.CursorTo(0, indexData);
          GLCD.print("   ");
          GLCD.CursorTo(0, indexData - 1);
          GLCD.print("->");
          --indexData;
          } else {
          --pageNumber2;
          indexData = 7;
          break;
          }
        }
      }
      lastButton_up = currentButton_up; //reset button value

      currentButton_down = digitalRead(B_DOWN); //read button state
      if (lastButton_down == LOW && currentButton_down == HIGH) //if it was pressed…
      {
        if(pageNumber2 == ((totalData - 1) / 8) + 1) { //if in last page
          if(indexData != (totalData - 1) % 8) { //if not the last entry
            GLCD.CursorTo(0, indexData);
            GLCD.print("   ");
            GLCD.CursorTo(0, indexData + 1);
            GLCD.print("->");
            ++indexData;
          }
        } else { //if not in last page
        if(indexData != 7) {
          GLCD.CursorTo(0, indexData);
          GLCD.print("   ");
          GLCD.CursorTo(0, indexData + 1);
          GLCD.print("->");
          ++indexData;
          } else {
          ++pageNumber2;
          indexData = 0;
          break;
          }
        }
      }
      lastButton_down = currentButton_down; //reset button value

      currentButton_enter = digitalRead(B_ENTER); //read button state
      if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
      {
        //This loop cycles through the files on previous pages
        for (int k = 0; k < (pageNumber2 - 1) * 8; k++) {
          File entry = dataDir.openNextFile();
          if(!entry) { break; }
          entry.close();
        }
        for (int i = 0; i < indexData; i++) {
            File entry = dataDir.openNextFile();
            if(!entry) { break; }
            entry.close();
          }
        File entry = dataDir.openNextFile();
        entry.read(title, 20);
        currentName = "";
        for(int a = 0; a < 20; a++) {
          currentName += title[a];
        }
        currentFile = entry.name();
        entry.close();
        
        prevMenu = 5;
        break;
      }
      lastButton_enter = currentButton_enter; //reset button value

      currentButton_back = digitalRead(B_BACK); //read button state
      if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
      {
        prevMenu = 1;
        break;
      }
      lastButton_back = currentButton_back; //reset button value
    }
  } else {
    GLCD.CursorTo(0, 0);
    GLCD.print("There are no data files on");
    GLCD.CursorTo(0, 1);
    GLCD.print("the SD Card.");
    delay(2000);
    prevMenu = 1;
  }
  dataDir.close();
}

void dataLogOptions() {
  GLCD.ClearScreen();
  GLCD.CursorTo(2, 0);
  GLCD.print(currentName);
  GLCD.CursorTo(2, 2);
  GLCD.print("Download  Data");
  GLCD.CursorTo(2, 3);
  GLCD.print("Delete  Data");

  GLCD.CursorTo(0, indexDLO);
  GLCD.print("->"); //Initial Location

  delay(100);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == LOW && currentButton_up == HIGH) //if it was pressed…
    {
      if(indexDLO != 2) {
        GLCD.CursorTo(0, indexDLO);
        GLCD.print("   ");
        GLCD.CursorTo(0, indexDLO - 1);
        GLCD.print("->");
        --indexDLO;
      }
    }
    lastButton_up = currentButton_up; //reset button value

    currentButton_down = digitalRead(B_DOWN); //read button state
    if (lastButton_down == LOW && currentButton_down == HIGH) //if it was pressed…
    {
      if(indexDLO != 3) {
        GLCD.CursorTo(0, indexDLO);
        GLCD.print("   ");
        GLCD.CursorTo(0, indexDLO + 1);
        GLCD.print("->");
        ++indexDLO;
      }
    }
    lastButton_down = currentButton_down; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
    {
      switch(indexDLO - 1) {
        case(1):
          downloadData();
          break;
        case(2):
          deleteData();
          break;
      }
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value

    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
    {
      prevMenu = 4;
      break;
    }
    lastButton_back = currentButton_back; //reset button value
  }
}

void downloadData() {
  Serial.begin(38400, SERIAL_8N1); //Initialize Serial Connection
  String dirD = "/data/";
  String name = "";
  int data;
  byte buffer[BUFFERSIZE];
  GLCD.ClearScreen();
  GLCD.CursorTo(6, 0);
  GLCD.print("Download Data");
  GLCD.CursorTo(1, 2);
  GLCD.print("Press enter when ready");
  GLCD.CursorTo(1, 3);
  GLCD.print("to send");

  delay(100);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  while(1) {
    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
    {
      GLCD.CursorTo(1, 5);
      GLCD.print("Sending...");
      name = dirD + currentFile;
      File dataFile = SD.open(name);
      while((data = dataFile.read(buffer, BUFFERSIZE)) > 0)
        Serial.write(buffer, data);
      GLCD.CursorTo(1, 6);
      GLCD.print("Transfer complete");
      dataFile.close();
      delay(2000);
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value

    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
    {
      break;
    }
    lastButton_back = currentButton_back; //reset button value
  }

  Serial.end();
  prevMenu = 5;  
}

void deleteData() {
  String dataFile;
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 3);
  GLCD.print("Are you sure you want");
  GLCD.CursorTo(0, 4);
  GLCD.print("to delete " + currentFile + "?");

  delay(100);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  while(1) {
    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
    {
      prevMenu = 5;
      break;
    }
    lastButton_back = currentButton_back; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
    {
      dataFile = "/data/" + currentFile;
      SD.remove(dataFile);
      totalData--;
      prevMenu = 4;
      if(indexData == 0 && pageNumber2 > 1 && totalData % 8 == 0) {
        indexData = 8;
        pageNumber2--;
      }
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value

  }
}

void upload() {
  Serial.begin(9600, SERIAL_8N1);
  String dirC = "/cycles/";
  String ext = ".mit";
  String name = "";
  String fullPath;
  bool isNoOverWrite = false;
  char title[20];
  byte buffer[BUFFERSIZE];
  int data;
  File dataFile;
  File tempFile;
  serialFlush();
  GLCD.ClearScreen();
  GLCD.CursorTo(4, 2);
  GLCD.print("Ready to receive file...");
  delay(50);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  if(SD.exists("/temp/temp")) { SD.remove("/temp/temp"); }
  tempFile = SD.open("/temp/temp", FILE_WRITE);
   while(1) {
    if(Serial.available() > 0) {
      GLCD.CursorTo(4, 3);
      GLCD.print("Receiving Data...");

      while ((data = Serial.readBytes(buffer, BUFFERSIZE)) > 0) {
        tempFile.write(buffer, data);
      }

      tempFile.seek(0);
      tempFile.read(title, 20);
      int count = 0;
      int digits = 0;
      while (count < 8) {
        if((title[digits] > 47 && title[digits] < 58)
           || (title[digits] > 64 && title[digits] < 91)
           || (title[digits] > 96 && title[digits] < 123)) {
          name += title[digits];
          count++;
         }        
         digits++;
      }

      name += ext;
      fullPath = dirC + name;

      //This if-statement checks if the file exists
      if(SD.exists(fullPath)) {
        GLCD.CursorTo(4, 5);
        GLCD.print("File Exists.");
        GLCD.CursorTo(4, 6);
        GLCD.print("Overwrite?");
        GLCD.CursorTo(4, 7);
        GLCD.print("Enter=Y  Back=N");
        
        while(1) {
          currentButton_enter = digitalRead(B_ENTER); //read button state
          if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
          {
            SD.remove(fullPath);
            break;
          }
          lastButton_enter = currentButton_enter; //reset button value

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            isNoOverWrite = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value
        }
      }

      if(!isNoOverWrite) { 
          GLCD.CursorTo(4, 4);
          GLCD.print("Saving...");
          GLCD.CursorTo(0, 5);
          GLCD.println("");
          GLCD.CursorTo(0, 6);
          GLCD.println("");
          GLCD.CursorTo(0, 7);
          GLCD.println("");          
          GLCD.CursorTo(4, 5);
          GLCD.print("Filename:");
          GLCD.CursorTo(4, 6);
          GLCD.print(name);

          dataFile = SD.open(dirC + name, FILE_WRITE);
          tempFile.seek(0);

          while((data = tempFile.read(buffer, BUFFERSIZE)) > 0)
            dataFile.write(buffer, data);
          
          dataFile.close();
          tempFile.close();
          totalCycles++;

          SD.remove("/temp/temp");

          delay(3000);
      }
      break;
    }

    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
    {
      break;
    }
    lastButton_back = currentButton_back; //reset button value
  }
  prevMenu = 1;
  Serial.end();
}

void viewCycle(){
  int row = 0;
  int lineNum = 1;
  bool leave = LOW;
  GLCD.ClearScreen();
  String dir = "/cycles/";
  String file = dir + currentFile;
  File cycle = SD.open(file, FILE_READ);
  if (!cycle) { Serial.write("Can't open file"); }
  cycle.seek(20);
  while (cycle.available()) {
    lastButton_down = HIGH;
    currentButton_down = HIGH;
    while (row != 8 && cycle.available()) {  
      String line;
      byte buff[5];
      cycle.read(buff, 5);
      uint16_t rate, temp;
      rate = ((uint16_t)buff[1] << 8) | (uint16_t)buff[2];
      temp = ((uint16_t)buff[3] << 8) | (uint16_t)buff[4];
      if(buff[0] == 72) {
        line = "Hold for " + String(rate) + " min";
      } else if (buff[0] == 82) {
        line = "Ramp to " + String(temp) + " at " + String(rate);
      } else {
        line = "Deramp to " + String(temp) + " at " + String(rate);
      }
      
      GLCD.CursorTo(0, row);
      GLCD.print(String(lineNum) + ". " + line);
      row++;
      lineNum++;
    }
    GLCD.CursorTo(21, 7);
    GLCD.print("Down");

    delay(100);

    while(1) {
      currentButton_down = digitalRead(B_DOWN); //read button state
      if (lastButton_down == LOW && currentButton_down == HIGH) //if it was pressed…
      {
        break;
      }
      lastButton_down = currentButton_down; //reset button value

      currentButton_back = digitalRead(B_BACK); //read button state
      if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
      {
        leave = HIGH;
        break;
      }
      lastButton_back = currentButton_back; //reset button value
    }
    
    GLCD.ClearScreen();
    row = 0;
    if (leave) { break; }
  }
  prevMenu = 3;
}

void deleteCycle() {
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 3);
  GLCD.print("Are you sure you want");
  GLCD.CursorTo(0, 4);
  GLCD.print("to delete " + currentFile + "?");

  delay(100);
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  while(1) {
    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
    {
      prevMenu = 3;
      break;
    }
    lastButton_back = currentButton_back; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == LOW && currentButton_enter == HIGH) //if it was pressed…
    {
      SD.remove("/cycles/" + currentFile);
      totalCycles--;
      prevMenu = 2;
      if(indexCC == 0 && pageNumber > 1 && totalCycles % 8 == 0) {
        indexCC = 8;
        pageNumber--;
      }
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value

  }
  prevMenu = 2;
}

void runCycle(){
  GLCD.ClearScreen();
  String dirC = "/cycles/";
  String dirD = "/data/";
  String dirL = "/log/";
  String dataExt = ".dat";
  String logExt = ".log";
  String cycleFile = dirC + currentFile;
  String dataFileName;
  String logFileName;
  String name = "";
  String airDisplay;
  String partDisplay;
  uint16_t holdTemp;
  double input = PREHEAT_TEMP, output = 50, setpoint = PREHEAT_TEMP;
  double kp = 8.66, ki = 0.02, kd = 1100.66;
  bool ct = false;
  bool lt = false;
  bool ct1 = false;
  bool lt1 = false;
  bool isExit = false;
  int data;
  char title[20];
  byte buffer[5];
  byte dataBuffer[8];
  uint16_t rate, temp;
  lastButton_enter = HIGH;
  currentButton_enter = HIGH;
  lastButton_back = HIGH;
  currentButton_back = HIGH;
  PID myPID(&input, &output, &setpoint, kp, ki, kd, DIRECT);
  myPID.SetMode(AUTOMATIC);
  Adafruit_MAX31856 air1(AIR_SENSOR1);
  Adafruit_MAX31856 air2(AIR_SENSOR2);  
  Adafruit_MAX31856 part1(PART_SENSOR1);
  Adafruit_MAX31856 part2(PART_SENSOR2);
  air1.begin();
  air2.begin();
  part1.begin();
  part2.begin();
  air1.setThermocoupleType(MAX31856_TCTYPE_K);
  air2.setThermocoupleType(MAX31856_TCTYPE_K);
  part1.setThermocoupleType(MAX31856_TCTYPE_K);
  part2.setThermocoupleType(MAX31856_TCTYPE_K);
  File cycle = SD.open(cycleFile, FILE_READ);
  if (!cycle) { Serial.write("Can't open file"); }
  cycle.read(title, 20);
  //Get the title and save it to new data file
  int count = 0;
  int digits = 0;
  while (count < 8) {
    if((title[digits] > 47 && title[digits] < 58)
        || (title[digits] > 64 && title[digits] < 91)
        || (title[digits] > 96 && title[digits] < 123)) {
      name += title[digits];
      count++;
      }        
      digits++;
  }
  //name += ext;
  dataFileName = dirD + name + dataExt;
  logFileName = dirL + name + logExt;

  if(SD.exists(dataFileName))
    SD.remove(dataFileName);

  if(SD.exists(logFileName))
    SD.remove(logFileName);    
  
  File dataFile = SD.open(dataFileName, FILE_WRITE);
  dataFile.write(title, 20);
  totalData++; //Add to number of data files on SD

  File logFile = SD.open(logFileName, FILE_WRITE);
  logFile.write(title, 20);
  logFile.write("\n\nPreheat started\n");

  //Start Preheat Cycle
  GLCD.CursorTo(0,0);
  GLCD.print("Preheat Cycle");
  GLCD.CursorTo(0, 2);
  GLCD.print("Preheat Temp: " + String(PREHEAT_TEMP));
  GLCD.CursorTo(0, 3);
  GLCD.print("Ambient Temp: ");
  uint16_t currentTemp = convertToF(tempConversion(air1.readThermocoupleTemperature(), air2.readThermocoupleTemperature()));
  delay(200);
  if(currentTemp < PREHEAT_TEMP) {
    unsigned long startTime = millis();
    uint16_t a1, a2;
    digitalWrite(RELAY1, LOW);
    digitalWrite(RELAY2, LOW);
    while(currentTemp < PREHEAT_TEMP) {
      ct = ((millis() - startTime) / 500) % 2;
          if (lt == HIGH && ct == LOW) 
          {

          a1 = convertToF(air1.readThermocoupleTemperature());
          a2 = convertToF(air2.readThermocoupleTemperature());
          currentTemp = tempConversion(a1, a2);
        
          GLCD.CursorTo(12, 3);
          GLCD.print(String(currentTemp) + "         ");

          }
          lt = ct;

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(5, 3);
            GLCD.print("Preheat Cancelled");
            delay(3000);
            lastButton_back = HIGH;
            break;
          }
          lastButton_back = currentButton_back; //reset button value

    }
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);  
    GLCD.ClearScreen();
    GLCD.CursorTo(2,2);
    GLCD.print("Preheat Cycle complete!");
    GLCD.CursorTo(2,3);
    GLCD.print("Starting Cure Cycle");
    delay(3000);
  } else if(currentTemp > PREHEAT_TEMP + 20) {
    unsigned long startTime = millis();
    uint16_t a1, a2;
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);
    GLCD.CursorTo(0,5);
    GLCD.print("Oven too hot, reducing temp");
    GLCD.CursorTo(0,6);
    GLCD.print("before starting cure cycle");
    while(currentTemp > PREHEAT_TEMP + 20) {
      ct = ((millis() - startTime) / 500) % 2;
          if (lt == HIGH && ct == LOW) 
          {

          a1 = convertToF(air1.readThermocoupleTemperature());
          a2 = convertToF(air2.readThermocoupleTemperature());
          currentTemp = tempConversion(a1, a2);
        
          GLCD.CursorTo(12, 3);
          GLCD.print(String(currentTemp) + "         ");

          }
          lt = ct;

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(5, 3);
            GLCD.print("Preheat Cancelled");
            delay(3000);
            lastButton_back = HIGH;
            break;
          }
          lastButton_back = currentButton_back; //reset button value          
    }
    digitalWrite(RELAY1, HIGH);
    digitalWrite(RELAY2, HIGH);  
    GLCD.ClearScreen();
    GLCD.CursorTo(2,2);
    GLCD.print("Preheat Cycle complete!");
    GLCD.CursorTo(2,3);
    GLCD.print("Starting Cure Cycle");
    delay(3000);    
  } else {
    GLCD.ClearScreen();
    GLCD.CursorTo(0, 3);
    GLCD.print("Oven withiin temp bounds");
    GLCD.CursorTo(0, 4);
    GLCD.print("Starting oven cycle");
    delay(3000);
  }
  logFile.write("Preheat Finished\n");

  //Generate Cycle Screen
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 0);
  GLCD.print("Relay  1: ");
  GLCD.CursorTo(0, 1);
  GLCD.print("Relay  2: ");
  GLCD.CursorTo(0, 3);
  GLCD.print("Ambient  Temp: ");
  GLCD.CursorTo(0, 4);
  GLCD.print("Part Temp: ");
  GLCD.CursorTo(0, 6);
  GLCD.print("Current  Instruction: ");
  //Start of cycle
  unsigned long startTime = millis();
  logFile.write(readTime(startTime));
  logFile.write(" -- Start of Cure Cycle\n");
  while((data = cycle.read(buffer, 5)) > 0) {
    //Convert rate and temp bytes to variables
    rate = ((uint16_t)buffer[1] << 8) | (uint16_t)buffer[2];
    temp = ((uint16_t)buffer[3] << 8) | (uint16_t)buffer[4];
    
    //Ramp code
    if(buffer[0] == 82) {
      uint16_t currentTemp;
      uint16_t partTemp;
      uint16_t airTemp;
      uint16_t p1, p2, a1, a2;
      //unsigned long dutyTime;
      //unsigned long adjOutput;
      //bool isOn;
      //unsigned long rampStart = millis();      
      holdTemp = temp; //If next instruction is a hold, this records the temp from previous ramp
      partTemp = tempConversion(convertToF(part1.readThermocoupleTemperature()), convertToF(part2.readThermocoupleTemperature()));
      delay(200);
      GLCD.CursorTo(0, 7);
      GLCD.print("Ramp to " + String(temp) + " at " + String(rate) + "          ");
      GLCD.CursorTo(18, 4);
      GLCD.print("TT: ");
      GLCD.CursorTo(14, 1);
      GLCD.print("            ");
      logFile.write(readTime(startTime));
      logFile.write(" -- Start of Ramp Cycle: ");
      logFile.write("Ramp to ");
      logFile.write(String(temp).c_str());
      logFile.write(" at ");
      logFile.write(String(rate).c_str());
      logFile.write("\n"); 
      while(partTemp < temp - 30) { //Loop until ramp temp is met
        unsigned long time = millis();
        currentTemp = tempConversion(convertToF(part1.readThermocoupleTemperature()), convertToF(part2.readThermocoupleTemperature()));
        delay(200);
        logFile.write(readTime(startTime));
        logFile.write(" -- Running to target temp ");
        logFile.write(String(currentTemp + rate).c_str());
        logFile.write("\n");
        while(millis() < time + 60000) { //Run this loop for 60 seconds

          //Grab temp, set relays, write to display every second, write to SD card every half second
          ct = ((millis() - startTime) / 250) % 2;
          if (lt == HIGH && ct == LOW) 
          {
            
          p1 = convertToF(part1.readThermocoupleTemperature());
          p2 = convertToF(part2.readThermocoupleTemperature());
          partTemp = tempConversion(p1, p2);
          a1 = convertToF(air1.readThermocoupleTemperature());
          a2 = convertToF(air2.readThermocoupleTemperature());
          airTemp = tempConversion(a1, a2);
          // Serial.print("AirTemp: " + String(airTemp) + "\n");
          // Serial.print("PartTemp: " + String(partTemp) + "\n\n");
          // Serial.print("a1: " + String(a1) + "\n");
          // Serial.print("a2: " + String(a2) + "\n");
          // Serial.print("p1: " + String(p1) + "\n");
          // Serial.print("p2: " + String(p2) + "\n");          
          
          

          printTime(17, 0, startTime);

          //Write to SD card
          dataBuffer[0] = p1 >> 8;
          dataBuffer[1] = p1 & 255;
          dataBuffer[2] = p2 >> 8;
          dataBuffer[3] = p2 & 255;
          dataBuffer[4] = a1 >> 8;
          dataBuffer[5] = a1 & 255;
          dataBuffer[6] = a2 >> 8;
          dataBuffer[7] = a2 & 255;
          dataFile.write(dataBuffer, 8);

          }
          lt = ct;

          ct1 = ((millis() - startTime) / 500) % 2;
          if (lt1 == HIGH && ct1 == LOW) 
          {
            if(partTemp >= currentTemp + rate) {  //When part temp goes over target temp or ambient goes over target + delta T   
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              //isOn = false;
            } else if((partTemp < currentTemp + rate) && 
                      (partTemp >= currentTemp + rate - 2) 
                      //&& isOn == false
                      ) {  //This code prevents rapid on and off. If oven is off, then if must go 2 degrees below target to turn back on (part or ambient)
              digitalWrite(RELAY1, LOW);
              GLCD.CursorTo(7, 0);
              GLCD.print("On");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              //isOn = false;
            } else {            
              digitalWrite(RELAY1, LOW);
              GLCD.CursorTo(7, 0);
              GLCD.print("On  ");
              digitalWrite(RELAY2, LOW);
              GLCD.CursorTo(7, 1);
              GLCD.print("On  ");
              //isOn = true;
            }
          
            //Serial.println("Dave");
            GLCD.CursorTo(13, 3);
            GLCD.print(String(airTemp) + "         ");
            GLCD.CursorTo(10, 4);
            GLCD.print(String(partTemp) + "         ");
            GLCD.CursorTo(21, 4);
            GLCD.print(String(currentTemp + rate) + "  ");

          }
          lt1 = ct1;

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(6, 3);
            GLCD.print("Cycle Cancelled");
            logFile.write(readTime(startTime));
            logFile.write(" -- Cycle Cancelled during Ramp Instruction\n");
            delay(3000);
            isExit = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value

        }               
        if(isExit) { break; }
      }
      if(isExit) { break; }
      logFile.write(readTime(startTime));
      logFile.write(" -- End of Ramp Cycle: ");
      logFile.write("Ramp to ");
      logFile.write(String(temp).c_str());
      logFile.write(" at ");
      logFile.write(String(rate).c_str());
      logFile.write("\n");
    //Hold Code
    } else if(buffer[0] == 72) {
      uint16_t partTemp;
      uint16_t airTemp;
      uint16_t p1, p2, a1, a2;
      unsigned long dutyTime;
      unsigned long adjOutput;
      //bool isOn;
      unsigned long time = millis();
      GLCD.CursorTo(0, 7);
      GLCD.print("Hold for " + String(rate) + " minutes    ");
      GLCD.CursorTo(18, 4);
      GLCD.print("TT: " + String(holdTemp) + "     ");
      GLCD.CursorTo(14, 1);
      GLCD.print("TR: "); 
      logFile.write(readTime(startTime));
      logFile.write(" -- Start of Hold Cycle: ");
      logFile.write("Hold for ");
      logFile.write(String(rate).c_str());
      logFile.write(" minutes\n");
      //Just want to make sure these start low
      ct1 = LOW;
      ct = LOW;
      //Want to grab the temps before the loop because 1398 would be grabbing garbage before the temp is first grabbed at 1444
      partTemp = tempConversion(convertToF(part1.readThermocoupleTemperature()), convertToF(part2.readThermocoupleTemperature()));
      airTemp = tempConversion(convertToF(air1.readThermocoupleTemperature()), convertToF(air2.readThermocoupleTemperature()));
      setpoint = holdTemp;
      //bool flag = true;
      while(millis() - time < (unsigned long)(rate * 60000)) {

          //Grab temp, set relays, write to display every second, write to SD card every quarter second
          ct = ((millis() - startTime) / 250) % 2;
          if (lt == HIGH && ct == LOW) 
          {
            // p1 = part1.readThermocoupleTemperature();
            // p2 = part2.readThermocoupleTemperature();
            // a1 = air1.readThermocoupleTemperature();
            // a2 = air2.readThermocoupleTemperature();
            // partTemp = convertToF(tempConversion(p1, p2));
            // airTemp = convertToF(tempConversion(a1, a2));

            printTime(17, 0, startTime);
            printTime(17, 1, time, rate);

            //Write to SD card
            dataBuffer[0] = p1 >> 8;
            dataBuffer[1] = p1 & 255;
            dataBuffer[2] = p2 >> 8;
            dataBuffer[3] = p2 & 255;
            dataBuffer[4] = a1 >> 8;
            dataBuffer[5] = a1 & 255;
            dataBuffer[6] = a2 >> 8;
            dataBuffer[7] = a2 & 255;
            dataFile.write(dataBuffer, 8);

          }
          lt = ct;        

          ct1 = ((millis() - startTime) / 500) % 2;
          if (lt1 == HIGH && ct1 == LOW) 
          {
            p1 = convertToF(part1.readThermocoupleTemperature());
            p2 = convertToF(part2.readThermocoupleTemperature());
            a1 = convertToF(air1.readThermocoupleTemperature());
            a2 = convertToF(air2.readThermocoupleTemperature());
            partTemp = tempConversion(p1, p2);
            airTemp = tempConversion(a1, a2);
            // Duty cycle and PID control            
            input = partTemp;
            myPID.Compute();
            // Serial.println("Output: " + String(output));
            adjOutput = (output / 255) * 1000;
            // Serial.println("adjOutput: " + String(adjOutput));
            GLCD.CursorTo(7, 0);
            GLCD.print("On  ");
            GLCD.CursorTo(7, 1);
            GLCD.print("On  ");
            // Serial.println("partTemp: " + String(partTemp));
 
            if(output != 0) {         
              // Serial.println("Turning on relays");
              digitalWrite(0, LOW);
              digitalWrite(1, LOW);
            }
            dutyTime = millis();
            // Serial.println("dutyTime" + String(dutyTime));

            //Print temperatures to GLCD
            GLCD.CursorTo(13, 3);
            GLCD.print(String(airTemp) + "         ");
            GLCD.CursorTo(10, 4);
            GLCD.print(String(partTemp) + "         ");
            // This code was overwritting TT
            // GLCD.CursorTo(21, 4);
            // GLCD.print(String(currentTemp + rate) + "  ");
          }
          lt1 = ct1;

          //Duty cycle control
          if (millis() > dutyTime + adjOutput) {
            GLCD.CursorTo(7, 0);
            GLCD.print("Off  ");
            GLCD.CursorTo(7, 1);
            GLCD.print("Off  ");
            if(output != 255) {
              // Serial.println("Turning off relays");
              digitalWrite(0, HIGH);
              digitalWrite(1, HIGH);
            }
          }


 
          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(6, 3);
            GLCD.print("Cycle Cancelled");
            logFile.write(readTime(startTime));
            logFile.write(" -- Cycle Cancelled during Hold Instruction\n");            
            delay(3000);
            isExit = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value 
      }
      if(isExit) { break; }
      logFile.write(readTime(startTime));
      logFile.write(" -- End of Hold Cycle: ");
      logFile.write("Hold for ");
      logFile.write(String(rate).c_str());
      logFile.write(" minnutes\n");      
    //Deramp Code
    } else if(buffer[0] == 68) {
      uint16_t currentTemp;
      uint16_t partTemp;
      uint16_t airTemp;
      uint16_t p1, p2, a1, a2;
      //unsigned long dutyTime;
      //unsigned long adjOutput;
      //bool isOn;
      //unsigned long derampStart = millis();
      holdTemp = temp; //If next instruction is a hold, this records the temp from previous ramp
      partTemp = convertToF(tempConversion(part1.readThermocoupleTemperature(), part2.readThermocoupleTemperature()));
      delay(200);
      GLCD.CursorTo(0, 7);
      GLCD.print("Deramp to " + String(temp) + " at " + String(rate) + "          ");
      GLCD.CursorTo(18, 4);
      GLCD.print("TT: ");
      GLCD.CursorTo(14, 1);
      GLCD.print("            ");      
      logFile.write(readTime(startTime));
      logFile.write(" -- Start of Deramp Cycle: ");
      logFile.write("Deramp to ");
      logFile.write(String(temp).c_str());
      logFile.write(" at ");
      logFile.write(String(rate).c_str());
      logFile.write("\n"); 

      while(partTemp > temp) { //Loop until ramp temp is met
unsigned long time = millis();
        currentTemp = convertToF(tempConversion(part1.readThermocoupleTemperature(), part2.readThermocoupleTemperature()));
        delay(200);
        logFile.write(readTime(startTime));
        logFile.write(" -- Running to target temp ");
        logFile.write(String(currentTemp - rate).c_str());
        logFile.write("\n");        
        while(millis() < time + 60000) {

          ct = ((millis() - startTime) / 250) % 2;
          if (lt == HIGH && ct == LOW) 
          {

            p1 = convertToF(part1.readThermocoupleTemperature());
            p2 = convertToF(part2.readThermocoupleTemperature());
            a1 = convertToF(air1.readThermocoupleTemperature());
            a2 = convertToF(air2.readThermocoupleTemperature());
            partTemp = tempConversion(p1, p2);
            airTemp = tempConversion(a1, a2);

          

          printTime(17, 0, startTime);
          // GLCD.CursorTo(17, 0);
          // if((millis() - startTime) / 3600000 < 10)
          //   GLCD.print("0");
          // GLCD.print(String((millis() - startTime) / 3600000) + ":");
          // if(((millis() - startTime) / 60000) % 60 < 10)
          //   GLCD.print("0");
          // GLCD.print(String(((millis() - startTime) / 60000) % 60) + ":");
          // if(((millis() - startTime) / 1000) % 60 < 10)
          //   GLCD.print("0");
          // GLCD.print(String(((millis() - startTime) / 1000) % 60) + "   ");

          //Write to SD card

          dataBuffer[0] = p1 >> 8;
          dataBuffer[1] = p1 & 255;
          dataBuffer[2] = p2 >> 8;
          dataBuffer[3] = p2 & 255;
          dataBuffer[4] = a1 >> 8;
          dataBuffer[5] = a1 & 255;
          dataBuffer[6] = a2 >> 8;
          dataBuffer[7] = a2 & 255;
          dataFile.write(dataBuffer, 8); 
        
          }
          lt = ct; 

          ct1 = ((millis() - startTime) / 500) % 2;
          if (lt1 == HIGH && ct1 == LOW) 
          {  
            if((partTemp <= currentTemp - rate) ||
              (airTemp <= currentTemp - rate - DELTA_T)) {
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("On  ");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("On  ");
              //isOn = true;
            } else if(((partTemp > currentTemp - rate) && 
                      (partTemp <= currentTemp - rate + 2)
                      //&& isOn == true
                      ) ||
                      ((airTemp > currentTemp - rate - DELTA_T) && 
                      (airTemp <= currentTemp - rate - DELTA_T - 2)
                      // && isOn == true)
                      )){
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off ");
              digitalWrite(RELAY2, LOW);
              GLCD.CursorTo(7, 1);
              GLCD.print("On  ");
              //isOn = true;
            } else {            
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              //isOn = false;
            }

            GLCD.CursorTo(13, 3);
            GLCD.print(String(airTemp) + "    ");
            GLCD.CursorTo(10, 4);
            GLCD.print(String(partTemp) + "    ");
            GLCD.CursorTo(21, 4);
            GLCD.print(String(currentTemp - rate) + "  ");
          }
          lt1 = ct1;


          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(6, 3);
            GLCD.print("Cycle Cancelled");
            logFile.write(readTime(startTime));
            logFile.write(" -- Cycle Cancelled during Deramp Instruction\n");
            delay(3000);
            isExit = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value
          
        }
        if (isExit) { break; }
    
      }
        logFile.write(readTime(startTime));
        logFile.write(" -- End of Deramp Cycle: ");
        logFile.write("Deramp to ");
        logFile.write(String(temp).c_str());
        logFile.write(" at ");
        logFile.write(String(rate).c_str());
        logFile.write("\n"); 
    }

    if (isExit) { break; }

  }

  if(!isExit) {
    GLCD.ClearScreen();
    GLCD.CursorTo(6, 3);
    GLCD.print("Cycle Complete!");
    GLCD.CursorTo(4, 4);
    GLCD.print("Total Time: ");
    printTime(startTime);
    // if((millis() - startTime) / 3600000 < 10)
    //   GLCD.print("0");
    // GLCD.print(String((millis() - startTime) / 3600000) + ":");
    // if(((millis() - startTime) / 60000) % 60 < 10)
    //   GLCD.print("0");
    // GLCD.print(String(((millis() - startTime) / 60000) % 60) + ":");
    // if(((millis() - startTime) / 1000) % 60 < 10)
    //   GLCD.print("0");
    // GLCD.print(String(((millis() - startTime) / 1000) % 60) + "     ");
    logFile.write(readTime(startTime));
    logFile.write(" -- Cycle Completed\n");
    delay(3000);
  }
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  cycle.close();
  dataFile.close();
  logFile.close();
  prevMenu = 3;
}

void serialFlush() {
  while(Serial.available() > 0)
    Serial.read();
}

uint16_t tempConversion (uint16_t data1, uint16_t data2) {
  //Insert code to convert sensor data to a temperature
  //return data1;
  //return data2;
  return (data1 + data2) / 2;
}

uint16_t convertToF(uint16_t temp) {
  return (temp * 9 / 5) + 32;
}

void printTime(byte col, byte row, unsigned long time) {
  GLCD.CursorTo(col, row);          
  if((millis() - time) / 3600000 < 10)
    GLCD.print("0");
  GLCD.print(String((millis() - time) / 3600000) + ":");
  if(((millis() - time) / 60000) % 60 < 10)
    GLCD.print("0");
  GLCD.print(String(((millis() - time) / 60000) % 60) + ":");
  if(((millis() - time) / 1000) % 60 < 10)
    GLCD.print("0");
  GLCD.print(String(((millis() - time) / 1000) % 60) + "   ");
}

void printTime(byte col, byte row, unsigned long time, uint16_t rate) {
  GLCD.CursorTo(col, row); 
  if((((unsigned long)(rate * 60000) + time) - millis()) / 3600000 < 10)
    GLCD.print("0");
  GLCD.print(String((((unsigned long)(rate * 60000) + time) - millis()) / 3600000) + ":");
  if(((((unsigned long)(rate * 60000) + time) - millis()) / 60000) % 60 < 10)
    GLCD.print("0");
  GLCD.print(String(((((unsigned long)(rate * 60000) + time) - millis()) / 60000) % 60) + ":");
  if(((((unsigned long)(rate * 60000) + time) - millis()) / 1000) % 60 < 10)
    GLCD.print("0");
  GLCD.print(String(((((unsigned long)(rate * 60000) + time) - millis()) / 1000) % 60) + "   ");    
}

void printTime(unsigned long time) {
  if((millis() - time) / 3600000 < 10)
    GLCD.print("0");
  GLCD.print(String((millis() - time) / 3600000) + ":");
  if(((millis() - time) / 60000) % 60 < 10)
    GLCD.print("0");
  GLCD.print(String(((millis() - time) / 60000) % 60) + ":");
  if(((millis() - time) / 1000) % 60 < 10)
    GLCD.print("0");
  GLCD.print(String(((millis() - time) / 1000) % 60) + "   ");
}

char* readTime(unsigned long time) {
  String charTime;
  if((millis() - time) / 3600000 < 10)
    charTime += "0";
  charTime += String((millis() - time) / 3600000) + ":";   
  if(((millis() - time) / 60000) % 60 < 10)
    charTime += "0";
  charTime += String(((millis() - time) / 60000) % 60) + ":";   
  if(((millis() - time) / 1000) % 60 < 10)
    charTime += "0";
  charTime += String(((millis() - time) / 1000) % 60) + "   ";
  charTime.toCharArray(timeBuff, 9);
  return timeBuff;
}