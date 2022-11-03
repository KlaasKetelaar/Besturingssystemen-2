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
        Serial.println(commandType.name);
        for (int indexStruct = 0; indexStruct < structSize; indexStruct++){
          int commandResult = strcmp(command[indexStruct].name,commandType.name);  
          Serial.println(commandResult); 
          if (commandResult == 0){
            command[indexStruct].func();    
            commandFound = true;   
          }
        }
          if(commandFound == false){
            Serial.println("No command found please try again");
            commandLength = 0;
            memset(commandType.name, NULL, BUFSIZE); //reset the array          
          }
        // if (commandFound == false){
        //   memset(commandType.name, NULL, commandLength);
        //   Serial.println("No command found please try again");
        //   Serial.println("please choose one of these commands:");
        //   for(int i = 0; i < structSize; i++){
        //     Serial.println(i);
        //     Serial.println(command[i].name);
        //   }
        // }
        
        commandLength = 0;
        memset(commandType.name, NULL, BUFSIZE); //reset the array
        // token complete
       }
      else{
      //   if(commandLength <= 12){
          Serial.println(commandType.name);
          commandType.name[commandLength] = tolower(c);
          commandLength++; 
        }  
      //   else{
      //     Serial.println("command too long please try again");
      //     processCommand();
      //   }

      // }
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