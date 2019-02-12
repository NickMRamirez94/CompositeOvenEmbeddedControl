 //#include <Arduino.h>
 #include <SD.h>
 #include <openGLCD.h>
 #include <Adafruit_MAX31856.h>
 #include <Adafruit_MAX31855.h>

//Define static variables
#define B_UP          11
#define B_DOWN        12
#define B_ENTER       13
#define B_BACK        14
#define BUFFERSIZE    64
#define RELAY1        0
#define RELAY2        1
#define PART_SENSOR1  A4
#define PART_SENSOR2  A5
#define AIR_SENSOR1   A6
#define AIR_SENSOR2   A7
#define DELTA_T       100
#define PREHEAT_TEMP  100

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
  //Init peripherals
  Serial.begin(9600, SERIAL_8N1); //Initialize Serial Connection
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
  GLCD.print("v  0.2");
  //Check for SD Card
  if (!SD.begin()) {
    GLCD.CursorTo(0, 7);
    GLCD.print("No SD Card Found.");
    while(1);    
  }

  //Count number of cure cycles on SD Card
  File cycleDir;
  cycleDir = SD.open("/cycles");
  if(!cycleDir) {
    SD.mkdir("/cycles");
    cycleDir = SD.open("/cycles");
    GLCD.CursorTo(0,5);
    GLCD.print("Creating Cycle Directory");
    delay(500);
  }
    
  //Count Total Files
  while(1) {
    File entry = cycleDir.openNextFile();
    if(!entry) { break; }
    totalCycles++;
    entry.close();
  }
  cycleDir.close();

  //Count number of data logs on SD Card
  File dataDir;
  dataDir = SD.open("/data");
  if(!dataDir) {
    SD.mkdir("/data");
    dataDir = SD.open("/data");
    GLCD.CursorTo(0,6);
    GLCD.print("Creating Data Directory");
    delay(500);
  }
  
  //Count Total Files
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
  GLCD.CursorTo(9, 0);  
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
  while(!digitalRead(B_ENTER));
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
  prevMenu = 3;
}

void runCycle(){
  GLCD.ClearScreen();
  String dirC = "/cycles/";
  String dirD = "/data/";
  String ext = ".dat";
  String cycleFile = dirC + currentFile;
  String dataFileName;
  String name = "";
  String airDisplay;
  String partDisplay;
  uint16_t holdTemp;
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
  Adafruit_MAX31856 part1(A6);
  Adafruit_MAX31856 part2(A7);
  Adafruit_MAX31855 air1(A4);
  Adafruit_MAX31855 air2(A5);
  part1.begin();
  part2.begin();
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
  name += ext;
  dataFileName = dirD + name;

  if(SD.exists(dataFileName))
    SD.remove(dataFileName);
  
  File dataFile = SD.open(dataFileName, FILE_WRITE);
  dataFile.write(title, 20);
  totalData++;

  //Generate Cycle Screen
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
      bool isOn;
      unsigned long rampStart = millis();
      holdTemp = temp; //If next instruction is a hold, this records the temp from previous ramp
      partTemp = tempConversion(convertToF(part1.readThermocoupleTemperature()), convertToF(part2.readThermocoupleTemperature()));
      delay(200);
      GLCD.CursorTo(0, 7);
      GLCD.print("Ramp to " + String(temp) + " at " + String(rate) + "          ");
      while(partTemp < temp && (millis() - rampStart) < (temp / rate) * 60000) { //Loop until ramp temp is met or expected time of ramp is met
        unsigned long time = millis();
        currentTemp = tempConversion(convertToF(part1.readThermocoupleTemperature()), convertToF(part2.readThermocoupleTemperature()));
        delay(200);
        while(millis() < time + 60000) { //Run this loop for 60 seconds

          //Grab temp, set relays, write to display every second, write to SD card every quarter second
          ct = ((millis() - startTime) / 125) % 2;
          if (lt == HIGH && ct == LOW) 
          {

          p1 = convertToF(part1.readThermocoupleTemperature());
          p2 = convertToF(part2.readThermocoupleTemperature());
          a1 = air1.readFarenheit();
          a2 = air2.readFarenheit();
          partTemp = tempConversion(p1, p2);
          airTemp = tempConversion(a1, a2);

          
          ct1 = ((millis() - startTime) / 500) % 2;
          if (lt1 == HIGH && ct1 == LOW) 
          {
            if((partTemp >= currentTemp + rate) ||
              (airTemp >= currentTemp + rate + DELTA_T)) {
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              isOn = false;
            } else if(((partTemp < currentTemp + rate) && 
                      (partTemp >= currentTemp + rate - 2) &&
                      isOn == false) ||
                      ((airTemp < currentTemp + rate + DELTA_T) && 
                      (airTemp >= currentTemp + rate + DELTA_T - 2) &&
                      isOn == false)){
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              isOn = false;
            } else {            
              digitalWrite(RELAY1, LOW);
              GLCD.CursorTo(7, 0);
              GLCD.print("On  ");
              digitalWrite(RELAY2, LOW);
              GLCD.CursorTo(7, 1);
              GLCD.print("On  ");
              isOn = true;
            }
          
            GLCD.CursorTo(13, 3);
            GLCD.print(String(airTemp) + "         ");
            GLCD.CursorTo(10, 4);
            GLCD.print(String(partTemp) + "         ");

          }
          lt1 = ct1;

          GLCD.CursorTo(17, 0);          
          if((millis() - startTime) / 3600000 < 10)
            GLCD.print("0");
          GLCD.print(String((millis() - startTime) / 3600000) + ":");
          if(((millis() - startTime) / 60000) % 60 < 10)
            GLCD.print("0");
          GLCD.print(String(((millis() - startTime) / 60000) % 60) + ":");
          if(((millis() - startTime) / 1000) % 60 < 10)
            GLCD.print("0");
          GLCD.print(String(((millis() - startTime) / 1000) % 60) + "   ");

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

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(6, 3);
            GLCD.print("Cycle Cancelled");
            delay(3000);
            isExit = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value

        }
        if(isExit) { break; }        
      }   
    //Hold Code
    } else if(buffer[0] == 72) {
      uint16_t partTemp;
      uint16_t airTemp;
      uint16_t p1, p2, a1, a2;
      bool isOn;
      unsigned long time = millis();
      GLCD.CursorTo(0, 7);
      GLCD.print("Hold for " + String(rate) + " minutes    ");
      while(millis() - time < (unsigned long)(rate * 60000)) {

        ct = ((millis() - startTime) / 125) % 2;
        if (lt == HIGH && ct == LOW) 
        {

        p1 = convertToF(part1.readThermocoupleTemperature());
        p2 = convertToF(part2.readThermocoupleTemperature());
        a1 = air1.readFarenheit();
        a2 = air2.readFarenheit();
        partTemp = tempConversion(p1, p2);
        airTemp = tempConversion(a1, a2);

        ct1 = ((millis() - startTime) / 500) % 2;
        if (lt1 == HIGH && ct1 == LOW) 
        {
          if(partTemp >= holdTemp) {
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              isOn = false;
            } else if((partTemp < holdTemp + rate) && 
                      (partTemp >= holdTemp + rate - 2) &&
                      isOn == false){
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              isOn = false;
            } else {            
              digitalWrite(RELAY1, LOW);
              GLCD.CursorTo(7, 0);
              GLCD.print("On  ");
              digitalWrite(RELAY2, LOW);
              GLCD.CursorTo(7, 1);
              GLCD.print("On  ");
              isOn = true;
            }

            GLCD.CursorTo(13, 3);
            GLCD.print(String(airTemp) + "         ");
            GLCD.CursorTo(10, 4);
            GLCD.print(String(partTemp) + "         ");
          }
          lt1 = ct1;
          
          GLCD.CursorTo(17, 0);
          if((millis() - startTime) / 3600000 < 10)
            GLCD.print("0");
          GLCD.print(String((millis() - startTime) / 3600000) + ":");
          if(((millis() - startTime) / 60000) % 60 < 10)
            GLCD.print("0");
          GLCD.print(String(((millis() - startTime) / 60000) % 60) + ":");
          if(((millis() - startTime) / 1000) % 60 < 10)
            GLCD.print("0");
          GLCD.print(String(((millis() - startTime) / 1000) % 60) + "   ");

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

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(6, 3);
            GLCD.print("Cycle Cancelled");
            delay(3000);
            isExit = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value 
      }
      if(isExit) { break; }
    //Deramp Code
    } else if(buffer[0] == 68) {
      uint16_t currentTemp;
      uint16_t partTemp;
      uint16_t airTemp;
      uint16_t p1, p2, a1, a2;
      bool isOn;
      unsigned long derampStart = millis();
      holdTemp = temp; //If next instruction is a hold, this records the temp from previous ramp
      partTemp = convertToF(tempConversion(part1.readThermocoupleTemperature(), part2.readThermocoupleTemperature()));
      delay(200);
      GLCD.CursorTo(0, 7);
      GLCD.print("Deramp to " + String(temp) + " at " + String(rate) + "          ");
      while(partTemp > temp && (millis() - derampStart) < (temp / rate) * 60000) { //Loop until ramp temp is met
        unsigned long time = millis();
        currentTemp = convertToF(tempConversion(part1.readThermocoupleTemperature(), part2.readThermocoupleTemperature()));
        delay(200);
        while(millis() < time + 60000) {

          ct = ((millis() - startTime) / 125) % 2;
          if (lt == HIGH && ct == LOW) 
          {

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
              isOn = true;
            } else if(((partTemp > currentTemp - rate) && 
                      (partTemp <= currentTemp - rate + 2) &&
                      isOn == true) ||
                      ((airTemp > currentTemp - rate - DELTA_T) && 
                      (airTemp <= currentTemp - rate - DELTA_T - 2) &&
                      isOn == true)){
              digitalWrite(RELAY1, LOW);
              GLCD.CursorTo(7, 0);
              GLCD.print("On  ");
              digitalWrite(RELAY2, LOW);
              GLCD.CursorTo(7, 1);
              GLCD.print("On  ");
              isOn = true;
            } else {            
              digitalWrite(RELAY1, HIGH);
              GLCD.CursorTo(7, 0);
              GLCD.print("Off");
              digitalWrite(RELAY2, HIGH);
              GLCD.CursorTo(7, 1);
              GLCD.print("Off");
              isOn = false;
            }

            GLCD.CursorTo(13, 3);
            GLCD.print(String(airTemp) + "    ");
            GLCD.CursorTo(10, 4);
            GLCD.print(String(partTemp) + "    ");
          }
          lt1 = ct1;

          GLCD.CursorTo(17, 0);
          if((millis() - startTime) / 3600000 < 10)
            GLCD.print("0");
          GLCD.print(String((millis() - startTime) / 3600000) + ":");
          if(((millis() - startTime) / 60000) % 60 < 10)
            GLCD.print("0");
          GLCD.print(String(((millis() - startTime) / 60000) % 60) + ":");
          if(((millis() - startTime) / 1000) % 60 < 10)
            GLCD.print("0");
          GLCD.print(String(((millis() - startTime) / 1000) % 60) + "   ");

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


          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == LOW && currentButton_back == HIGH) //if it was pressed…
          {
            GLCD.ClearScreen();
            GLCD.CursorTo(6, 3);
            GLCD.print("Cycle Cancelled");
            delay(3000);
            isExit = true;
            break;
          }
          lastButton_back = currentButton_back; //reset button value
          
        }
        if (isExit) { break; }    
      }
    }

    if (isExit) { break; }
  }

  if(!isExit) {
    GLCD.ClearScreen();
    GLCD.CursorTo(6, 3);
    GLCD.print("Cycle Complete!");
    GLCD.CursorTo(4, 4);
    GLCD.print("Total Time: ");
    if((millis() - startTime) / 3600000 < 10)
      GLCD.print("0");
    GLCD.print(String((millis() - startTime) / 3600000) + ":");
    if(((millis() - startTime) / 60000) % 60 < 10)
      GLCD.print("0");
    GLCD.print(String(((millis() - startTime) / 60000) % 60) + ":");
    if(((millis() - startTime) / 1000) % 60 < 10)
      GLCD.print("0");
    GLCD.print(String(((millis() - startTime) / 1000) % 60) + "     ");
    delay(3000);
  }
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);
  cycle.close();
  dataFile.close();
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