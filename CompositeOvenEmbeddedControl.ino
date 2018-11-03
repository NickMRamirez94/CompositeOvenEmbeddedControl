//#include <SerialFlash.h>
//#include <SPI.h>
#include <SD.h>

#define GLCD   8

//Set buttons to arduino input pins
#define B_UP      A5
#define B_DOWN    A4
#define B_ENTER   7
#define B_BACK    4
#define LED       2
#define BUFFERSIZE 20

byte prevMenu = 1;
byte pageNumber = 1;
byte totalCycles = 0;
byte totalData = 0;
byte indexMain = 1;
byte indexCC = 0;
byte indexCCO = 1;
byte indexData = 1;
String currentFile;

//Button state flags. Used for detecting button presses.
bool lastButton_up = LOW;
bool currentButton_up = LOW;
bool lastButton_enter = LOW;
bool currentButton_enter = LOW;
bool lastButton_down = LOW;
bool currentButton_down = LOW;
bool lastButton_back = LOW;
bool currentButton_back = LOW;
bool ledState = LOW;

// File cycleDir;
// File dataDir;

void setup() {
  Serial.begin(9600, SERIAL_8N1);
  
  pinMode(GLCD, OUTPUT);
  pinMode(LED, OUTPUT);
  pinMode(B_UP, INPUT_PULLUP);
  pinMode(B_DOWN, INPUT_PULLUP);
  pinMode(B_ENTER, INPUT_PULLUP);
  pinMode(B_BACK, INPUT_PULLUP);
  
  digitalWrite(GLCD, HIGH);
  delay(500); //Waiting for LCD Controller to start up
  //Print Title Screen
  lcdPrint(7, 3, "Oven  M.I.T.T.");
  lcdPrint(11, 4, "v  0.1");
  if (!SD.begin(9)) {
    lcdPrint(0, 7, "No SD Card Found.");
    while(1);
  }
  //Count number of cure cycles on SD Card
  File cycleDir = SD.open("/cycles");
  if(!cycleDir) { lcdPrint(0, 0, "Can't open cycles folder"); while(1); }
  //Count Total Files
  while(1) {
    File entry = cycleDir.openNextFile();
    if(!entry) { break; }
    totalCycles++;
    entry.close();
  }
  cycleDir.close();

  //Count number of data logs on SD Card
  File dataDir = SD.open("/data");
  if(!dataDir) { lcdPrint(0, 0, "Can't open data folder"); while(1); }
  //Count Total Files
  while(1) {
    File entry = dataDir.openNextFile();
    if(!entry) { break; }
    totalData++;
    entry.close();
  }
  dataDir.close();

  //delay(1000);
  lcdClear();
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
      cureCycleOptions(currentFile);
      break;
    case (4):
      dataLog();
      break;
    default:
      mainMenu();  
  }
  
}

void mainMenu() {
  lcdClear();
  lcdPrint(9, 0, "Main  Menu");
  lcdPrint(2, 2, "List  Saved  Cure  Cycles");
  lcdPrint(2, 3, "Upload  New  Cure  Cycle");
  lcdPrint(2, 4, "Data  Log  Management");

  lcdPrint(0, indexMain + 1, "->"); //Initial Location
  prevMenu = 1;
  
  while(!digitalRead(B_ENTER));
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
    {
      if(indexMain != 1) {
        lcdPrint(0, indexMain + 1, "   ");
        lcdPrint(0, indexMain, "->");
        --indexMain;
      }
    }
    lastButton_up = currentButton_up; //reset button value

    currentButton_down = digitalRead(B_DOWN); //read button state
    if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
    {
      if(indexMain != 3) {
        lcdPrint(0, indexMain + 1, "   ");
        lcdPrint(0, indexMain + 2, "->");
        ++indexMain;
      }
    }
    lastButton_down = currentButton_down; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
    {
      switch(indexMain) {
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
  
  lcdClear();

  File cycleDir = SD.open("/cycles");
  if (totalCycles != 0) {
    for (int k = 0; k < (pageNumber - 1) * 8; k++) {
      File entry = cycleDir.openNextFile();
      if(!entry) { break; }
      entry.close();
    }
    for (int i = 0; i < 8; i++) {
        File entry = cycleDir.openNextFile();
        if(!entry) { break; }
        lcdPrint(2, row, entry.name());
        row++;
        entry.close();
      }

    cycleDir.rewindDirectory();
    if(pageNumber == (((totalCycles - 1) / 8) + 1) && (indexCC + 1) > (totalCycles % 8)) { //For if you delete the last file in the list
      lcdPrint(0, indexCC - 1, "->"); //Initial Location
      indexCC--;  
    } else {
      lcdPrint(0, indexCC, "->"); //Initial Location
    }
    prevMenu = 2;
    
    while(!digitalRead(B_ENTER));
    while(!digitalRead(B_BACK));
    while(1) {
      currentButton_up = digitalRead(B_UP); //read button state
      if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
      {
        if(pageNumber == 1) { //if in first page
          if(indexCC != 0) { //if not the first entry
            lcdPrint(0, indexCC, "   ");
            lcdPrint(0, indexCC - 1, "->");
            --indexCC;
          }
        } else { //if not in first page
        if(indexCC != 0) {
          lcdPrint(0, indexCC, "   ");
          lcdPrint(0, indexCC - 1, "->");
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
            lcdPrint(0, indexCC, "   ");
            lcdPrint(0, indexCC + 1, "->");
            ++indexCC;
          }
        } else { //if not in last page
        if(indexCC != 7) {
          lcdPrint(0, indexCC, "   ");
          lcdPrint(0, indexCC + 1, "->");
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
        for (int k = 0; k < (pageNumber - 1) * 8; k++) {
          File entry = cycleDir.openNextFile();
          if(!entry) { break; }
          entry.close();
        }
        for (int i = 0; i < indexCC + 1; i++) {
            File entry = cycleDir.openNextFile();
            if(!entry) { break; }
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
    lcdPrint(0, 0, "There are no cycles on");
    lcdPrint(0, 1, "the SD Card.");
    delay(2000);
    prevMenu = 1;
  }
  cycleDir.close();
}

void cureCycleOptions(String name) {
  currentFile = name;
  lcdClear();
  lcdPrint(((25 - name.length()) / 2), 0, name);
  lcdPrint(2, 2, "Run  Cycle");
  lcdPrint(2, 3, "View  Cycle  Instructions");
  lcdPrint(2, 4, "Delete  Cycle");
  
  lcdPrint(0, indexCCO + 1, "->"); //Initial Location
  prevMenu = 3;
  
  while(!digitalRead(B_ENTER));
  while(!digitalRead(B_DOWN));
  while(!digitalRead(B_BACK));
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
    {
      if(indexCCO != 1) {
        lcdPrint(0, indexCCO + 1, "   ");
        lcdPrint(0, indexCCO, "->");
        --indexCCO;
      }
    }
    lastButton_up = currentButton_up; //reset button value

    currentButton_down = digitalRead(B_DOWN); //read button state
    if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
    {
      if(indexCCO != 3) {
        lcdPrint(0, indexCCO + 1, "   ");
        lcdPrint(0, indexCCO + 2, "->");
        ++indexCCO;
      }
    }
    lastButton_down = currentButton_down; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
    {
      switch(indexCCO) {
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
  lcdClear();
  lcdPrint(5, 0, "Data  Logs");
  //List of logs
  lcdPrint(2, 2, "Back");
  
  lcdPrint(0, indexData + 1, "->"); //Initial Location
  prevMenu = 4;
  
  while(!digitalRead(B_ENTER));
  while(1) {
    currentButton_up = digitalRead(B_UP); //read button state
    if (lastButton_up == HIGH && currentButton_up == LOW) //if it was pressed…
    {
      if(indexData != 1) {
        lcdPrint(0, indexData + 1, "   ");
        lcdPrint(0, indexData, "->");
        --indexData;
      }
    }
    lastButton_up = currentButton_up; //reset button value

    currentButton_down = digitalRead(B_DOWN); //read button state
    if (lastButton_down == HIGH && currentButton_down == LOW) //if it was pressed…
    {
      if(indexData != 1) {
        lcdPrint(0, indexData + 1, "   ");
        lcdPrint(0, indexData + 2, "->");
        ++indexData;
      }
    }
    lastButton_down = currentButton_down; //reset button value

    currentButton_enter = digitalRead(B_ENTER); //read button state
    if (lastButton_enter == HIGH && currentButton_enter == LOW) //if it was pressed…
    {
      switch(indexData) {
        default:
          prevMenu = 1;
          break;
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
  char title[20];
  byte buffer[BUFFERSIZE];
  int data;
  unsigned int ticker = 0;
  bool toggle = LOW;
  File dataFile;
  File tempFile;
  lcdClear();
  lcdPrint(4, 2, "Ready to receive file...");
  while(!digitalRead(B_ENTER));
  if(SD.exists("/temp/temp")) { SD.remove("/temp/temp"); }
  tempFile = SD.open("/temp/temp", FILE_WRITE);
   while(1) {
    if(Serial.available() > 0) {
      lcdPrint(4, 3, "Receiving Data...");

      unsigned int ticker = 0;
      bool toggle = LOW;
      while ((data = Serial.readBytes(buffer, BUFFERSIZE)) > 0) {
        tempFile.write(buffer, data);
        ticker++;
        if((ticker % 25) == 0)
          toggle = !toggle;
        digitalWrite(LED, toggle);
      }
      digitalWrite(LED, LOW);

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
      if(SD.exists(fullPath)) {
        lcdPrint(4, 5, "File Exists.");
        lcdPrint(4, 6, "Overwrite?");
        while(1);
      }

      lcdPrint(4, 4, "Saving: ");
      lcdPrint(0, 5, dirC + name);
      //while(1);
      dataFile = SD.open(dirC + name, FILE_WRITE);
      tempFile.seek(0);
      
      ticker = 0;
      while ((data = tempFile.read(buffer, BUFFERSIZE)) > 0) {
        dataFile.write(buffer, data);
        ticker++;
        if((ticker % 15) == 0)
          toggle = !toggle;
        digitalWrite(LED, toggle);
      }
      digitalWrite(LED, LOW);

      dataFile.close();
      tempFile.close();

      SD.remove("/temp/temp");
      totalCycles++;
      delay(1000);
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
  //int col = 0;
  int lineNum = 1;
  bool leave = LOW;
  char myChar;
  lcdClear();
  String dir = "/cycles/";
  String file = dir + currentFile;
  File cycle = SD.open(file, FILE_READ);
  if (!cycle) { Serial.write("Can't open file"); }
  
  while (cycle.available()) {
    while (row != 8 && cycle.available()) {  
      String line = "";
      do {
        myChar = cycle.read();
        line += myChar;
      } while(myChar != 10 && cycle.available());
      lcdPrint(0, row, String(lineNum) + ". " + line);
      row++;
      lineNum++;
    }
    lcdPrint(19, 7, "Down");
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
    
    lcdClear();
    row = 0;
    if (leave) { break; }
  }
}

void deleteCycle() {
  lcdClear();
  lcdPrint(0, 3, "Are you sure you want");
  lcdPrint(0, 4, "to delete " + currentFile + "?");
  while(!digitalRead(B_ENTER));
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

void lcdPrint(byte x, byte y, String message) {
   digitalWrite(GLCD, LOW);
   Serial.write(2);
   Serial.write(x);
   Serial.write(y);
   Serial.write(1);
   Serial.println(message);
   //delay(10);
   //digitalWrite(GLCD, HIGH);
}

void lcdClear() {
  digitalWrite(GLCD, LOW);
  Serial.write(3);
  //delay(10);
  //digitalWrite(GLCD, HIGH);
}
