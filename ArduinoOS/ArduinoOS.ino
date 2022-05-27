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
}

void loop()
{
  processCommand();
  // do other stuff
}

void processCommand(){
    commandType commandType;
    char c;
    while (Serial.available()) 
    {
      c = Serial.read();
      if (c == ' ' || c == '\r' || c =='\n') {
        Serial.println(commandType.name);
        // Serial.println("1111");
        for (int indexStruct = 0; indexStruct < structSize; indexStruct++){
          int commandResult = strcmp(command[indexStruct].name,commandType.name);   
          // Serial.println("2222");
          // Serial.println(commandResult);    
          if (commandResult == 0){
            command[indexStruct].func();       
          }
        }
        commandLength = 0;
        memset(commandType.name, NULL, BUFSIZE); //reset the array
        // Serial.println(commandType.name);
        // Serial.println("3333");
        // token complete
       }
      else{  
        // Serial.println(commandLength);
        if (commandLength > BUFSIZE){
          Serial.println("command too long try again");
          commandLength = 0;
          memset(commandType.name, NULL, BUFSIZE); //reset the array           
          processCommand();
        }
        else{
        commandType.name[commandLength] = tolower(c);
        commandLength++; 
        }
      }// append c to buffer
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