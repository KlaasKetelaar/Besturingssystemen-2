#include <EEPROM.h>
#include <SoftwareSerial.h>

#define BUFSIZE 12
#define MEMORY_SIZE 256
#define MAX_VARIABLES 25
#define FILESYSTEM_SIZE EEPROM.length()
#define MAX_FILES 10
#define MAX_FILENAME_LENGTH 12
#define STACK_SIZE 32

int commandLength = 0;
int structSize = 10;
char commandName[BUFSIZE];


void store(char* filename);
void retrieve(char* filename);
void erase(char* filename);
void files();
void freespace();
void run(char* filename);
void list();
void suspend(int processId);
void resume(int processId);
void kill(int processId);

typedef struct {
  char name[BUFSIZE];
  void (*func)();
} commandType;

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

struct Variable {
  char name;
  void* address;
  byte size;
};

struct Process {
  int id;
  Variable variables[MAX_VARIABLES];
  int numVariables;
  char filename[MAX_FILENAME_LENGTH];
  bool active;
  byte stack[STACK_SIZE];
};

struct FileSystem {
  char filenames[MAX_FILES][MAX_FILENAME_LENGTH];
  int numFiles;
};

// Global variables
Variable memory[MEMORY_SIZE];
FileSystem filesystem;
Process processes[10];
int numProcesses = 0;

// Function prototypes
void initializeOperatingSystem();
void readCommand();
void executeProgram(Process* process);
void createFile(char* filename);
void readFile(char* filename);
void writeFile(char* filename);
void initializeProcess(Process* process);
void allocateMemory(Process* process);
void freeMemory(Process* process);

// Initialization function
void initializeOperatingSystem() {
  Serial.begin(9600);
  filesystem.numFiles = 0;
  numProcesses = 0;
}

// Read command from the serial Serial
void readCommand() {
  commandType commandType;
  char c;
  bool commandFound = false;
  while (Serial.available()) 
  {
    c = Serial.read();
    if (c == ' ' || c == '\r' || c =='\n') {
      // validate command name
      commandType.name[commandLength] = '\0'; // terminate string
      if (strlen_P(commandType.name) == 0) {
        Serial.println("No command entered, please try again");
        return;
      }
      for (int i = 0; i < strlen_P(commandType.name); i++) {
        if (!isalpha(pgm_read_byte_near(commandType.name + i)) && !isspace(pgm_read_byte_near(commandType.name + i))) {
          Serial.println("Invalid character in command, please try again");
          return;
        }
      }
      // search for command
      for (int indexStruct = 0; indexStruct < structSize; indexStruct++){
        int commandResult = strcmp_P(command[indexStruct].name, commandType.name);
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



// Show the available storage space
void freespace() {
  int usedStorage = filesystem.numFiles * MAX_FILENAME_LENGTH;
  int availableStorage = FILESYSTEM_SIZE - usedStorage;
  Serial.print("Available storage: ");
  Serial.print(availableStorage);
  Serial.println(" bytes");
}

// Create a file in the file system
void store(char* filename) {
  if (filesystem.numFiles < MAX_FILES) {
    strcpy(filesystem.filenames[filesystem.numFiles], filename);
    filesystem.numFiles++;
    Serial.println("File created successfully.");
  } else {
    Serial.println("File system is full. Cannot create more files.");
  }
}


// Read a file from the file system
void readFile(char* filename) {
  // Read the file
  // ...
}

// Write to a file in the file system
void writeFile(char* filename) {
  // Write to the file
  // ...
}

// Delete a file from the file system
void erase(char* filename) {
  int fileIndex = -1;
  for (int i = 0; i < filesystem.numFiles; i++) {
    if (strcmp(filesystem.filenames[i], filename) == 0) {
      fileIndex = i;
      break;
    }
  }

  if (fileIndex != -1) {
    // Shift filenames to overwrite the deleted file
    for (int i = fileIndex; i < filesystem.numFiles - 1; i++) {
      strcpy(filesystem.filenames[i], filesystem.filenames[i + 1]);
    }
    filesystem.numFiles--;
    Serial.println("File deleted successfully.");
  } else {
    Serial.println("File not found. Cannot delete.");
  }
}

// Start a new process
void run(char* filename) {
  if (numProcesses < 10) {
    Process* process = &processes[numProcesses];
    initializeProcess(process);
    allocateMemory(process);
    strcpy(process->filename, filename);
    process->active = true;
    executeProgram(process);
    numProcesses++;
  } else {
    Serial.println("Maximum number of processes reached. Cannot start a new process.");
  }
}

// Execute program stored in bytecode
void executeProgram(Process* process) {
  // Here you can implement the logic to execute the program
  // stored in the bytecode.
  // You can access the filename of the program from `process->filename`.
  // Perform the necessary operations to run the program.
  // ...
  // Example:
  Serial.print("Executing program: ");
  Serial.println(process->filename);
  // ...
}

// Pause a running process
void suspend(int processId) {
  if (processId >= 0 && processId < numProcesses) {
    processes[processId].active = false;
    Serial.println("Process paused.");
  } else {
    Serial.println("Invalid process ID.");
  }
}

// Resume a paused process
void resume(int processId) {
  if (processId >= 0 && processId < numProcesses) {
    processes[processId].active = true;
    Serial.println("Process resumed.");
  } else {
    Serial.println("Invalid process ID.");
  }
}

// End a process
void kill(int processId) {
  if (processId >= 0 && processId < numProcesses) {
    freeMemory(&processes[processId]);
    numProcesses--;
    for (int i = processId; i < numProcesses; i++) {
      processes[i] = processes[i + 1];
    }
    Serial.println("Process ended.");
  } else {
    Serial.println("Invalid process ID.");
  }
}

// Initialize a process
void initializeProcess(Process* process) {
  process->id = numProcesses;
  process->numVariables = 0;
  process->active = false;
}

// Allocate memory for a process
void allocateMemory(Process* process) {
  // Allocate memory for the process
  // ...
}

// Free memory allocated for a process
void freeMemory(Process* process) {
  // Free memory allocated for the process
  // ...
}

// List all files in the file system
void list() {
  Serial.println("Files in file system:");
  for (int i = 0; i < filesystem.numFiles; i++) {
    Serial.println(filesystem.filenames[i]);
  }
}



// Perform setup operations
// void setup() {
//   // Perform setup operations
//   // ...
// }

void retrieve(char* filename)
{
  Serial.println("retrieve function called");
}


void files()
{
  Serial.println("files function called");
}



void setup() {
  initializeOperatingSystem();
  freespace();
}

void loop() {
  readCommand();
  // Other operations
  // ...
}
