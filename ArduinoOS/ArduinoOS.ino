// C++ code
//
#define BUFSIZE 12
typedef struct{
  char name[BUFSIZE];
  void (*func)();
}commandType;


void store(){
  Serial.println("store functie aangeroepen");
}
void retrieve(){
  Serial.println("retrieve functie aangeroepen");
}

static commandType command[] ={
  {"store", &store},
  {"retrieve", &retrieve}
};



void setup()
{
  Serial.begin(9600);
}

void loop()
{
    commandType commandType;
    int CHARCOUNT = 0;
    const char k = 'k';
    char c;
    while (Serial.available()) 
    {
      c = Serial.read();
      if (c == ' ' || c == '\r' || c =='\n') 
      {
      //  static int n = sizeof(command)/sizeof(commandType);
        Serial.println("in if statement:");
//        strncat(commandType.name, "\0",n+1);
       Serial.println(CHARCOUNT);
        Serial.println(commandType.name);
       for(int i=0; i<CHARCOUNT; i++){
        Serial.println("in for statement");
        Serial.println(commandType.name[i]);         
        command[commandType.name[i]].func(); 
       }
        CHARCOUNT = 0;
        memset(commandType.name, 0, BUFSIZE);
      } // token complete
      else 
      {
          CHARCOUNT++;
          strncat(commandType.name,&c,1);
          Serial.println(commandType.name);
          
      }// append c to buffer
    }
  // do other stuff
}
