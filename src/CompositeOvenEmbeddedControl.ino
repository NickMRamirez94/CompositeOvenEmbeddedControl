// #include <Arduino.h>
 #include <SD.h>
 #include <openGLCD.h>

//Set buttons to arduino input pins
#define B_UP      11
#define B_DOWN    12
#define B_ENTER   13
#define B_BACK    14
#define BUFFERSIZE 64

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
bool lastButton_up = LOW;
bool currentButton_up = LOW;
bool lastButton_enter = LOW;
bool currentButton_enter = LOW;
bool lastButton_down = LOW;
bool currentButton_down = LOW;
bool lastButton_back = LOW;
bool currentButton_back = LOW;

//Function declarations
void mainMenu();
void dataLog();
void cureCycles();
void cureCycleOptions();
void dataLogOptions();
void downloadData();
void deleteData();
void serialFlush();

void setup() {
  Serial.begin(9600, SERIAL_8N1); //Initialize Serial Connection
  
  //Set uC pin modes
  pinMode(B_UP, INPUT_PULLUP);
  pinMode(B_DOWN, INPUT_PULLUP);
  pinMode(B_ENTER, INPUT_PULLUP);
  pinMode(B_BACK, INPUT_PULLUP);
  pinMode(SS, OUTPUT);
  
  //Print Title Screen
  GLCD.Init();
  GLCD.SelectFont(Iain5x7);
  GLCD.ClearScreen();
  GLCD.CursorTo(7, 3);
  GLCD.print("Oven M.I.T.T.");
  GLCD.CursorTo(10, 4);
  GLCD.print("v  0.1");
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
  
  dataDir.close();

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
  
  while(!digitalRead(B_ENTER));
  delay(50);
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
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
    if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
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
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
    {
      switch(indexMain - 1) {
        case(1):
          cureCycles();
          break;
        case(2):
          upload();
          break;
        case(3):
          dataLog();
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
        String str(title);
        GLCD.print(str);
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
    
    while(!digitalRead(B_ENTER));
    while(!digitalRead(B_BACK));
    delay(50);
    while(1) {
      currentButton_up = digitalRead(B_UP); //read button state
      if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
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
      if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
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
      if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
      {
        //This loop cycles through the files on previous pages
        for (int k = 0; k < (pageNumber - 1) * 8; k++) {
          File entry = cycleDir.openNextFile();
          if(!entry) { break; }
          entry.close();
        }
        for (int i = 0; i < indexCC + 1; i++) {
            File entry = cycleDir.openNextFile();
            if(!entry) { break; }
            entry.read(title, 20);
            String str(title);
            currentName = str;
            currentFile = entry.name();
            row++;
            entry.close();
          }
        prevMenu = 3;
        break;
      }
      lastButton_enter = currentButton_enter; //reset button value

      currentButton_back = digitalRead(B_BACK); //read button state
      if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
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
  prevMenu = 3;
  
  while(!digitalRead(B_ENTER));
  while(!digitalRead(B_DOWN));
  while(!digitalRead(B_BACK));
  delay(50);
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
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
    if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
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
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
    {
      switch(indexCCO - 1) {
        case(1):
          runCycle();
          break;
        case(2):
          viewCycle();
          break;
        case(3):
          deleteCycle();
          break;
      }
      break;
    }
    lastButton_enter = currentButton_enter; //reset button value

    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
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

  File cycleDir = SD.open("/data");
  if (totalData != 0) {
    //This loop cycles through the files on previous pages so that the current page can be displayed
    for (int k = 0; k < (pageNumber2 - 1) * 8; k++) {
      File entry = cycleDir.openNextFile();
      if(!entry) { break; }
      entry.close();
    }
    for (int i = 0; i < 8; i++) {
        File entry = cycleDir.openNextFile();
        if(!entry) { break; }
        entry.read(title, 20);
        GLCD.CursorTo(2, row);
        String str(title);
        GLCD.print(str);
        row++;
        entry.close();
      }

    cycleDir.rewindDirectory();
    if(pageNumber2 == (((totalData - 1) / 8) + 1) && (indexData + 1) > (totalData % 8)) { //For if you delete the last file in the list
      GLCD.CursorTo(0, indexData - 1);
      GLCD.print("->"); //Initial Location
      indexData--;  
    } else {
      GLCD.CursorTo(0, indexData);
      GLCD.print("->"); //Initial Location
    }
    prevMenu = 2;
    
    while(!digitalRead(B_ENTER));
    while(!digitalRead(B_BACK));
    delay(50);
    while(1) {
      currentButton_up = digitalRead(B_UP); //read button state
      if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
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
      if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
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
      if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
      {
        //This loop cycles through the files on previous pages
        for (int k = 0; k < (pageNumber2 - 1) * 8; k++) {
          File entry = cycleDir.openNextFile();
          if(!entry) { break; }
          entry.close();
        }
        for (int i = 0; i < indexData + 1; i++) {
            File entry = cycleDir.openNextFile();
            if(!entry) { break; }
            entry.read(title, 20);
            String str(title);
            currentName = str;
            currentFile = entry.name();
            row++;
            entry.close();
          }
        prevMenu = 5;
        break;
      }
      lastButton_enter = currentButton_enter; //reset button value

      currentButton_back = digitalRead(B_BACK); //read button state
      if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
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
  cycleDir.close();
}

void dataLogOptions() {
  GLCD.ClearScreen();
  GLCD.CursorTo(2, 0);
  GLCD.print(currentName);
  GLCD.CursorTo(2, 2);
  GLCD.print("Download Data");
  GLCD.CursorTo(2, 3);
  GLCD.print("Delete Data");

  GLCD.CursorTo(0, indexDLO);
  GLCD.print("->"); //Initial Location
  
  while(!digitalRead(B_ENTER));
  while(!digitalRead(B_DOWN));
  while(!digitalRead(B_BACK));
  delay(50);

  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
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
    if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
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
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
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
    if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
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

  while(!digitalRead(B_ENTER));
  delay(100);
  while(1) {
    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
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
    if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
    {
      break;
    }
    lastButton_back = currentButton_back; //reset button value
  }

  prevMenu = 5;  
}

void deleteData() {
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 3);
  GLCD.print("Are you sure you want");
  GLCD.CursorTo(0, 4);
  GLCD.print("to delete " + currentFile + "?");
  while(!digitalRead(B_ENTER));
  delay(50);
  while(1) {
    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
    {
      prevMenu = 5;
      break;
    }
    lastButton_back = currentButton_back; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
    {
      SD.remove("/data/" + currentFile);
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
          if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
          {
            SD.remove(fullPath);
            break;
          }
          lastButton_enter = currentButton_enter; //reset button value

          currentButton_back = digitalRead(B_BACK); //read button state
          if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
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
    if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
    {
      break;
    }
    lastButton_back = currentButton_back; //reset button value
  }
}

void runCycle(){}

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
        line = "Ramp to " + String(rate) + " at " + String(temp);
      } else {
        line = "Deramp to " + String(rate) + " at " + String(temp);
      }
      
      GLCD.CursorTo(0, row);
      GLCD.print(String(lineNum) + ". " + line);
      row++;
      lineNum++;
    }
    GLCD.CursorTo(21, 7);
    GLCD.print("Down");
    while(!digitalRead(B_DOWN));
    while(1) {
      currentButton_down = digitalRead(B_DOWN); //read button state
      if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
      {
        break;
      }
      lastButton_down = currentButton_down; //reset button value

      currentButton_back = digitalRead(B_BACK); //read button state
      if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
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
}

void deleteCycle() {
  GLCD.ClearScreen();
  GLCD.CursorTo(0, 3);
  GLCD.print("Are you sure you want");
  GLCD.CursorTo(0, 4);
  GLCD.print("to delete " + currentFile + "?");
  while(!digitalRead(B_ENTER));
  delay(50);
  while(1) {
    currentButton_back = digitalRead(B_BACK); //read button state
    if (lastButton_back == HIGH && currentButton_back == LOW) //if it was pressed…
    {
      prevMenu = 3;
      break;
    }
    lastButton_back = currentButton_back; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
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

}

void serialFlush() {
  while(Serial.available() > 0)
    char f = Serial.read();
}