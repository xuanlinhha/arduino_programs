const String LOCATION = "1";
const char START = 0x02;
const char END = 0x03;

bool completed;
String command;

void setup() {
  Serial.begin(115200);
  setupPinMode();
  completed = true;
  command = "";
}
 
void loop() {
  // wait for command
  if (Serial.available()) {
    char c = Serial.read();
    if (c == START) {
      completed = false;
    }
    else if (c == END) {
      completed = true;
      
      // process full message
      processCommand(command);
      
      // reset command
      command = "";
    }
    else {
      if(completed == false) {
        command += c;
      }
    }
  }
}

void setupPinMode() {
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
}

void processCommand(String message){
  // [location ticket id status checksum]
  // verify checksum
  if (!verifyChecksum(message)) {
    return;
  }
  
  // check location
  String location = message.substring(0, 1);
  if (!location.equals(LOCATION)) {
    // forward message to next arduino
    return;
  }
  
  // process command based on its content
  // TODO: [1 0 0_0 1 cs]
  String ticket = message.substring(2, 3);
  String id = message.substring(4, 7);
  String status = message.substring(8, 9);
  int pin = getPinById(id);
  if (status.equals("1")) {
    digitalWrite(pin, HIGH);
  }
  else if (status.equals("0")) {
    digitalWrite(pin, LOW);
  }
  
  // send response
  String response = prepareResponse(ticket, "1");
  Serial.print(response);
  Serial.flush();
}

char generateChecksum(String message) {
  int sum = 0;
  for (int i = 0; i < message.length(); i++) {
    sum = sum + message.charAt(i);
  }
  int remainder = sum % 256;
  char result = 256 - remainder;
  if (result == 0x02 || result == 0x03) {
    result = ' ';
  }
  return result;
}

boolean verifyChecksum(String fullMessage) {
  char checksum = generateChecksum(fullMessage.substring(0, fullMessage.length() - 2));
  if (checksum != fullMessage.charAt(fullMessage.length() - 1)) {
    return false;
  }
  return true;
}

// get pin based on id
int getPinById(String id) {
  if(id.equals("0_0")) {
    return 8;
  }
  else if(id.equals("0_1")) {
    return 9;
  }
  else if(id.equals("0_2")) {
    return 10;
  }
  else if(id.equals("0_3")) {
    return 11;
  }
  else if(id.equals("0_4")) {
    return 12;
  }
  else {
    return 13;
  }
}

String prepareResponse(String ticket, String status) {
  // [START 0 R ticket status checksum END]
  String message = "";
  message += '0';
  message += '\t';
  message += 'R';
  message += '\t';
  message += ticket;
  message += '\t';
  message += status;
  char checksum = generateChecksum(message);
  String result = "";
  result += START;
  result += message;
  result += '\t';
  result += checksum;
  result += END;
  return result;
}
