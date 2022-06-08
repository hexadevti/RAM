/*
  EEPROM reader
*/

#include <string.h>

int CE = 46; // NAO USADO
int OE = 15; 
int WE = 5;
int CE2 = 7;

const unsigned int dataPin[] = { 24, 26, 28, 29, 27, 25, 23, 21 };
int addressPin[] = { 22, 20, 18, 16, 14, 12, 10, 8, 9, 11, 17, 13, 6, 2, 3 };
// int decoder[] = { 21, 23, 25, 27 };
int addressSize = 15;
int currentAddress = 0;
const unsigned int WCT = 10;
int maxAddress = 0;
const boolean logData = false;
// byte data[] = {0xAB, 0x5E, 0x6F, 0xB4, 0xEC, 0x88, 0x25, 0x3D, 
//                0xC0, 0x12, 0x55, 0x79, 0xBC, 0xAA, 0x11, 0x99};
int size = 0;
int listCount = 0;

int StringCount = 0;


String command;

void setup() {
  Serial.begin(115200); 
  
  maxAddress = (int)pow(2, addressSize);
  size = sizeof(addressPin)/sizeof(addressPin[0]);
  
  pinMode(OE, OUTPUT);
  pinMode(WE, OUTPUT);
  pinMode(CE, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(OE, HIGH);
  digitalWrite(WE, HIGH);
  digitalWrite(CE, HIGH);

  // pinMode(7, OUTPUT);
  // pinMode(4, OUTPUT);
  // digitalWrite(7, LOW);
  // digitalWrite(4, LOW);

  pinMode(CE2, OUTPUT);
  digitalWrite(CE2, HIGH);
  
  

  Serial.println("OK");
// Decoder Test
// pinMode(3, OUTPUT);
// pinMode(2, OUTPUT);
// pinMode(19, OUTPUT);
// pinMode(21, INPUT);
// pinMode(23, INPUT);
// pinMode(25, INPUT);
// pinMode(27, INPUT);

// digitalWrite(19, LOW);
// digitalWrite(3, LOW);
// digitalWrite(2, LOW);
// End decoder test



}

// the loop function runs over and over again forever
void loop() {
  // Decoder test
  // digitalWrite(3, LOW);
  // digitalWrite(2, LOW);
  // readDecoder();
  // End Decoder test
  // digitalWrite(3, LOW);
  // digitalWrite(2, HIGH);
  //  readDecoder();
  // digitalWrite(3, HIGH);
  // digitalWrite(2, LOW);
  // readDecoder();
  // digitalWrite(3, HIGH);
  // digitalWrite(2, HIGH);
  // readDecoder();
  //while(true);
  // End Decoder Test
  

  if (Serial.available()) {
    command = Serial.readStringUntil('\n');
    command.trim();
    command.toLowerCase();
    if (command.startsWith("g ")) {
      commandGo(command);
    } else if (command.startsWith("c ")) {
      commandClear(command);
    }
    else if (command.equals("l")) {
      commandRead("r");
      currentAddress = currentAddress+256;
    } else if (command.equals("p ")) {
      char buf[60];
      sprintf(buf, "%04x", currentAddress);
      Serial.println("");
      Serial.print(buf);
      Serial.println("");
      Serial.println("OK");
    } else if (command.startsWith("r ")) {
      commandRead(command);
    } 
    else if (command.startsWith("w ")) {
      commandWrite(command);
    }
    else 
    {
      commandGo("g " + command);
    }
  }
}

// void readDecoder() {
//   uint8_t data = 0;
//   for (int pin = 3; pin >= 0; pin -= 1) {
//     data = (data << 1) + digitalRead(decoder[pin]);
//   }
//   Serial.println(toBinary(data, 4));
// }

void commandGo(String strCommand)
{
  unsigned int parmNo = 0;
  while (strCommand.length() > 0)
  {
    int index = strCommand.indexOf(' ');
    String parm = strCommand.substring(0, index);
    if (parmNo == 1)
    {
      currentAddress = StrToDec(parm);
      Serial.println();
      Serial.println("OK");
      break;
    }
    strCommand = strCommand.substring(index + 1);
    parmNo = parmNo + 1;
  }
}

void commandClear(String command) {
  unsigned int parmNo = 0;
  unsigned int startAddress = 0;
  unsigned int endAddress = 0;
  int dat = 0;
   
  while (command.length() > 0)
  {
    int index = command.indexOf(' ');
    String parm = command.substring(0, index);
    if (parmNo == 1)
    {
      startAddress = StrToDec(parm);
    }
    if (parmNo == 2)
    {
      endAddress = StrToDec(parm);
    }
    if (parmNo == 3)
    {
      dat = StrToDec(parm);
      erase(startAddress, endAddress, dat);
      Serial.println();
      Serial.println("OK");
      break;
    }

    command = command.substring(index + 1);
    parmNo = parmNo + 1;
  }
}

void commandRead(String strCommand)
{
  unsigned int parmNo = 0;
  unsigned int startAddress = 0;
  unsigned int endAddress = 0;
  unsigned int itemCount = 1;
  char buf[60];

  while (strCommand.length() > 0)
  {
    int index = strCommand.indexOf(' ');
    String parm = strCommand.substring(0, index);

    if (strCommand.equals("r"))
    {
      setRead();
      startAddress = currentAddress;
      endAddress = currentAddress + 255;
    }

    if (parmNo == 1)
    {
      setRead();
      startAddress = StrToDec(parm);
    }
    else if (parmNo == 2)
    {
      endAddress = StrToDec(parm);
    }
    else
    {
      if (index == -1)
      {
        for (unsigned int i = startAddress; i <= endAddress; i++)
        {
          if (itemCount == 1)
          {
            Serial.println();
            sprintf(buf, "%04x: ", i);
            Serial.print(buf);
          }
          else if (itemCount == 9)
          {
            Serial.print("    ");
          }
          sprintf(buf, "%02x ", readEEPROM(i));
          Serial.print(buf);
          if (itemCount == 16)
          {
            itemCount = 0;
          }
          itemCount++;
        }
        setStandby();
        Serial.println();
        Serial.println("OK");
        break;
      }
    }
    strCommand = strCommand.substring(index + 1);
    parmNo = parmNo + 1;
  }
}

void commandWrite(String strCommand)
{
  unsigned int parmNo = 0;
  unsigned int address = 0;
  unsigned int itemCount = 1;
  char buf[60];

  while (strCommand.length() > 0)
  {
    int index = strCommand.indexOf(' ');
    String parm = strCommand.substring(0, index);

    if (parmNo == 1)
    {
      address = StrToDec(parm);
      Serial.println();
      sprintf(buf, "%04x: ", address);
      Serial.print(buf);
    }
    else if (parmNo > 1)
    {
      int data = StrToDec(parm);
      writeByte(address + (parmNo - 2), data);
      sprintf(buf, "%02x ", data);
      Serial.print(buf);

      if (itemCount == 8)
      {
        Serial.print("   ");
      }
      if (itemCount == 16)
      {
        Serial.println();
        sprintf(buf, "%04x: ", address + (parmNo - 2));
        Serial.print(buf);
        itemCount = 0;
      }
      itemCount++;

      if (index == -1)
      {
        Serial.println();
        break;
      }
    }
    strCommand = strCommand.substring(index + 1);
    parmNo = parmNo + 1;
  }
}

int StrToDec(String parm) {
  int str_len = parm.length() + 1; 
  char char_array[str_len];
  parm.toCharArray(char_array, str_len);
  return (int) strtol(char_array, 0, 16);
}


void setRead() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(WE, HIGH);
  // digitalWrite(CE, LOW);
  digitalWrite(CE2, HIGH);
  digitalWrite(OE, LOW);
  for (unsigned int pin = 0; pin < 8; pin+=1) {
    pinMode(dataPin[pin], INPUT);
  }
  delay(WCT);
}

void setWrite() {
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(WE, HIGH);
  //digitalWrite(CE, LOW);
  digitalWrite(CE2, HIGH);
  digitalWrite(OE, HIGH);
  for (unsigned int pin = 0; pin < 8; pin+=1) {
    pinMode(dataPin[pin], OUTPUT);
  }
  delay(WCT);
}

void setStandby() {
  
  digitalWrite(WE, HIGH);
  digitalWrite(OE, HIGH);
  for (unsigned int pin = 0; pin < addressSize; pin+=1) {
    digitalWrite(addressPin[pin], LOW);
  }
  //digitalWrite(CE, HIGH);
  digitalWrite(CE2, LOW);

  digitalWrite(LED_BUILTIN, LOW);
  delay(WCT);
}

void setAddress(int address) {
  if (logData) Serial.println("address: " + toBinary(address, 8));

  for (unsigned int pin = 0; pin < addressSize; pin+=1) {
    pinMode(addressPin[pin], OUTPUT);
    if (logData) Serial.println("pinMode(" + (String)(addressPin[pin]) + ",OUTPUT)");
  }
  
  String binary;
  for (int i = 0; i < addressSize; i++) {
    digitalWrite(addressPin[i], (address & 1) == 1 ? HIGH : LOW);
    if (logData) Serial.println("digitalWrite(" + (String)(addressPin[i]) + "," + ((address & 1) == 1 ? "HIGH" : "LOW") + ")");
    address = address >> 1;
  }
}

uint8_t readEEPROM(int address) {
  for (int pin = 0; pin < 8; pin+=1) {
    pinMode(dataPin[pin], INPUT);
  }
  setAddress(address);
  digitalWrite(CE2, HIGH);
  digitalWrite(OE, LOW);
  uint8_t data = 0;
  for (int pin = 7; pin >= 0; pin -= 1) {
    data = (data << 1) + digitalRead(dataPin[pin]);
  }
  if (logData) Serial.println("read: " + (String)(data) + ", " + toBinary(data, 8));
  digitalWrite(CE2, LOW);
  digitalWrite(OE, HIGH);
  return data;
}

void writeEEPROM(unsigned int address, uint8_t data) {
  setAddress(address);
  digitalWrite(OE, HIGH);
  digitalWrite(CE2, HIGH);
  if (logData) Serial.println("write: " + (String)(data) + ", " + toBinary(data,8));
  for (int pin = 0; pin <= 7; pin += 1) {
    digitalWrite(dataPin[pin], data & 1);
    if (logData) Serial.println("digitalWrite(" + (String)(dataPin[pin]) + "," + ((data & 1) == 1 ? "HIGH" : "LOW" ) + ")");
    data = data >> 1;
  }
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(OE, LOW);
  digitalWrite(CE2, LOW);
  delay(WCT);
}

String toBinary(int n, int len)
{
    String binary;

    for (unsigned i = (1 << len - 1); i > 0; i = i / 2) {
        binary += (n & i) ? "1" : "0";
    }
 
    return binary;
}



void printContents(unsigned int startAddress, unsigned int endAddress ) {
  Serial.println("Reading EEPROM Max addresses: " + (String)((int)pow(2, addressSize)));
  Serial.println();
  setRead();

  for (unsigned int base = startAddress; base < endAddress; base += 16) {
    uint16_t data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }
    char buf[60];
    sprintf(buf, "%04x: %02x %02x %02x %02x %02x %02x %02x %02x     %02x %02x %02x %02x %02x %02x %02x %02x",
    base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], 
    data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
    Serial.println(buf);
  }

  setStandby();
}

void erase(unsigned int startAddress, unsigned int endAddress, uint8_t data) {
  setWrite();
  for (unsigned int address = startAddress; address <= endAddress; address += 1) {
    writeEEPROM(address, data);
    if ((address+1) % 128 == 0) {
      Serial.println((String)((float)(address+1) / endAddress * 100) + "%");
    }
  }
  setStandby();
}

void writeByte(unsigned int startAddress, byte data) {
  setWrite();
  writeEEPROM(startAddress, data);
  setStandby();
}

uint8_t readByte(unsigned int address) {
  setRead();
  return readEEPROM(address);
  setStandby();
}

void write(unsigned int startAddress, byte data[]) {
  Serial.println("Writing EEPROM");
  setWrite();

  for (unsigned int a = 0; a < size; a = a + 1)
  {
    if (a % 128 == 0) {
      Serial.println((String)((float)(startAddress + a) / (startAddress + size) * 100) + "%");
    }
    writeEEPROM(startAddress + a, data[a]);
  } 
  Serial.println(" done");
  setStandby();
}

