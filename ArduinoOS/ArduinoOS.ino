// C++ code
//
#define BUFSIZE 12
int commandLength = 0;
int structSize = 10;
char commandName[BUFSIZE];


void store();
void retrieve();
void erase();
void files();
void freespace();
void run();
void list();
void suspend();
void resume();
void kill();


typedef struct{
  char name[BUFSIZE];
  void (*func)();
}commandType;


static commandType command[] ={
  {"store", &store},
  {"retrieve", &retrieve},
  {"erase", &erase},
  {"files", &files},
  {"freespace", &freespace},
  {"run", &run},
  {"list", &list},
  {"suspend", &suspend},
  {"resume", &resume},
  {"kill", &kill}
};



void setup()
{

  Serial.begin(9600);
  Serial.println("Welcome to Arduino OS");
  Serial.println("Please type your command");
}

void loop()
{
  processCommand();
  // do other stuff
}

void processCommand(){
    commandType commandType;
    char c;
    bool commandFound = false;
    while (Serial.available()) 
    {
      c = Serial.read();
      if (c == ' ' || c == '\r' || c =='\n') {
        // validate command name
        commandType.name[commandLength] = '\0'; // terminate string
        if (strlen(commandType.name) == 0) {
          Serial.println("No command entered, please try again");
          return;
        }
        for (int i = 0; i < strlen(commandType.name); i++) {
          if (!isalpha(commandType.name[i]) && !isspace(commandType.name[i])) {
            Serial.println("Invalid character in command, please try again");
            return;
          }
        }
        // search for command
        for (int indexStruct = 0; indexStruct < structSize; indexStruct++){
          int commandResult = strcmp(command[indexStruct].name,commandType.name);  
          if (commandResult == 0){
            command[indexStruct].func();    
            commandFound = true;   
          }
        }
        if(!commandFound){
          Serial.println("No command found, please try again");
        }
        // reset command length and buffer
        commandLength = 0;
        memset(commandType.name, 0, BUFSIZE); 
      }
      else{
        // add character to buffer
        if (commandLength < BUFSIZE - 1){
          commandType.name[commandLength] = tolower(c);
          commandLength++; 
        }  
        else{
          Serial.println("Command too long, please try again");
          // reset command length and buffer
          commandLength = 0;
          memset(commandType.name, 0, BUFSIZE); 
        }
      }
    }
}

void store(){
  Serial.println("store function called");
}
void retrieve(){
  Serial.println("retrieve function called");
}
void erase(){
  Serial.println("erase function called");
}
void files(){
  Serial.println("files function called");
}
void freespace(){
  Serial.println("freespace function called");
}
void run(){
  Serial.println("run function called");
}
void list(){
  Serial.println("list function called");
}
void suspend(){
  Serial.println("suspend function called");
}
void resume(){
  Serial.println("resume function called");
}
void kill(){
  Serial.println("kill function called");
}