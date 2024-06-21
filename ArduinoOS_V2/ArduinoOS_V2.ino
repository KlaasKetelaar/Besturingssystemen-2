#include <EEPROM.h>

// Definities en structuren
#define BUFFER_SIZE 12
#define FAT_SIZE 5
#define EEPROM_SIZE 512
#define MEMORY_SIZE 128
#define STACK_SIZE 16
#define MAX_VARS 10
#define MAX_PROCESSES 5

#define NOP 0x00
#define LOAD_CONST 0x01
#define ADD 0x02
#define SUB 0x03
#define MUL 0x04
#define DIV 0x05
#define PRINT 0x06
#define STOP 0x07
#define IF 0x10
#define ELSE 0x11
#define ENDIF 0x12
#define LOOP 0x13
#define ENDLOOP 0x14
#define WHILE 0x15
#define ENDWHILE 0x16
#define MILLIS 0x20
#define DELAYUNTIL 0x21
#define OPEN 0x30
#define WRITE 0x31
#define READCHAR 0x32
#define READINT 0x33
#define READFLOAT 0x34
#define READSTR 0x35
#define CLOSE 0x36
#define FORK 0x40
#define WAITUNTILDONE 0x41

struct FATEntry {
    char name[BUFFER_SIZE];
    int start;
    int length;
};

struct Command {
    char name[BUFFER_SIZE];
    void (*func)();
};

struct Variable {
    char name;
    int processId;
    int address;
    int size;
    char type;
};

struct Process {
    char programName[BUFFER_SIZE];
    int processId;
    char status; // 'r' for running, 'p' for paused, 't' for terminated
    int pc; // Program counter
    int sp; // Stack pointer
    int fp; // File pointer
    int loopStart; // Start address of the loop
};

// Globale variabelen
char inputBuffer[BUFFER_SIZE];
int bufferIndex = 0;
FATEntry fat[FAT_SIZE];
int noOfFiles = 0;
byte memory[MEMORY_SIZE];
Variable memoryTable[MAX_VARS];
int noOfVars = 0;
byte stack[STACK_SIZE];
byte sp = 0;
Process processTable[MAX_PROCESSES];
int nextProcessId = 1; // Unique process ID generator
int noOfProcesses = 0;

// Functieprototypes
void prompt();
bool inputRoutine();
void processCommand(char* command);
void store();
void retrieve();
void erase();
void files();
void freeSpace();
void pushByte(byte b);
byte popByte();
void pushInt(int i);
int popInt();
void pushFloat(float f);
float popFloat();
void pushString(const char* str);
char* popString();
void storeVariable(char name, int processId, char type);
Variable retrieveVariable(char name, int processId);
void deleteVariable(char name, int processId);
void deleteAllVariables(int processId);
void writeFATEntry(int index, FATEntry entry);
FATEntry readFATEntry(int index);
int findFileIndex(const char* filename);
void testMemory();
void startProcess(const char* programName);
void pauseProcess(int processId);
void resumeProcess(int processId);
void stopProcess(int processId);
void listProcesses();
int findProcessIndex(int processId);
void runProcesses();
void executeInstruction(int processIndex);
void loadExampleProgram();
void loadIfElseProgram();
void loadLoopProgram();
void loadWhileProgram();
void printEEPROMContents(int start, int length); // Nieuwe functie voor debugging
void printStack(); // Functie voor stack debugging

void executeStop(Process* proc);
void executeIf(Process* proc);
void executeElse(Process* proc);
void executeEndIf(Process* proc);
void executeLoop(Process* proc);
void executeEndLoop(Process* proc);
void executeWhile(Process* proc);
void executeEndWhile(Process* proc);
void executeMillis(Process* proc);
void executeDelayUntil(Process* proc);
void executeOpen(Process* proc);
void executeWrite(Process* proc);
void executeRead(Process* proc, byte type);
void executeReadChar(Process* proc);
void executeReadInt(Process* proc);
void executeReadFloat(Process* proc);
void executeReadStr(Process* proc);
void executeClose(Process* proc);
void executeFork(Process* proc);
void executeWaitUntilDone(Process* proc);

// Command lijst
Command commands[] = {
    {"store", store},
    {"retrieve", retrieve},
    {"erase", erase},
    {"files", files},
    {"freespace", freeSpace},
    {"start", []() { // Anonymous function to start a process
        Serial.println("Enter program name:");
        while (!inputRoutine());
        startProcess(inputBuffer);
    }},
    {"pause", []() { // Anonymous function to pause a process
        Serial.println("Enter process ID:");
        while (!inputRoutine());
        pauseProcess(atoi(inputBuffer));
    }},
    {"resume", []() { // Anonymous function to resume a process
        Serial.println("Enter process ID:");
        while (!inputRoutine());
        resumeProcess(atoi(inputBuffer));
    }},
    {"stop", []() { // Anonymous function to stop a process
        Serial.println("Enter process ID:");
        while (!inputRoutine());
        stopProcess(atoi(inputBuffer));
    }},
    {"list", listProcesses},
    // Voeg meer commando's toe indien nodig
};

// Setup en loop
void setup() {
    Serial.begin(9600);
    prompt();
    
    loadExampleProgram(); // Laad het voorbeeldprogramma in het EEPROM en de FAT
    // loadIfElseProgram();  // Laad het IF-ELSE programma
    loadLoopProgram();    // Laad het LOOP programma
    // loadWhileProgram();   // Laad het WHILE programma
    
    // startProcess("example"); // Start het voorbeeldproces
    // startProcess("ifelse");  // Start het IF-ELSE proces
    // startProcess("loop");    // Start het LOOP proces
    // startProcess("while");   // Start het WHILE proces
}

void loop() {
    if (inputRoutine()) {
        processCommand(inputBuffer);
        prompt();
    }
    runProcesses(); // Run processes during each loop iteration
}


// Functie implementaties
void prompt() {
    Serial.println("ArduinOS 1.0 ready");
}

bool inputRoutine() {
    while (Serial.available()) {
        char c = Serial.read();
        if (c == ' ' || c == '\n') {
            inputBuffer[bufferIndex] = '\0';
            bufferIndex = 0;
            return true;
        } else {
            inputBuffer[bufferIndex++] = c;
            if (bufferIndex >= BUFFER_SIZE - 1) {
                inputBuffer[BUFFER_SIZE - 1] = '\0';
                bufferIndex = 0;
                return true;
            }
        }
    }
    return false;
}

void processCommand(char* command) {
    for (int i = 0; i < sizeof(commands) / sizeof(Command); i++) {
        if (strcmp(command, commands[i].name) == 0) {
            commands[i].func();
            return;
        }
    }
    Serial.println("Unknown command. Available commands:");
    for (int i = 0; i < sizeof(commands) / sizeof(Command); i++) {
        Serial.println(commands[i].name);
    }
}

void store() {
    Serial.println("Enter filename:");
    while (!inputRoutine());
    char filename[BUFFER_SIZE];
    strcpy(filename, inputBuffer);

    Serial.println("Enter size:");
    while (!inputRoutine());
    int size = atoi(inputBuffer);

    int fileIndex = findFileIndex(filename);
    if (fileIndex != -1) {
        Serial.println("Error: File already exists");
        return;
    }

    if (noOfFiles >= FAT_SIZE) {
        Serial.println("Error: No space in FAT");
        return;
    }

    int availableSpace = EEPROM_SIZE - (sizeof(FATEntry) * FAT_SIZE);
    if (size > availableSpace) {
        Serial.println("Error: Not enough space");
        return;
    }

    FATEntry newFile;
    strcpy(newFile.name, filename);
    newFile.start = noOfFiles == 0 ? sizeof(FATEntry) * FAT_SIZE : fat[noOfFiles - 1].start + fat[noOfFiles - 1].length;
    newFile.length = size;

    writeFATEntry(noOfFiles, newFile);
    fat[noOfFiles++] = newFile;

    Serial.println("Enter data:");
    for (int i = 0; i < size; i++) {
        while (!Serial.available());
        char c = Serial.read();
        EEPROM.write(newFile.start + i, c);
    }

    Serial.println("File stored successfully");
}

void retrieve() {
    Serial.println("Enter filename:");
    while (!inputRoutine());
    char filename[BUFFER_SIZE];
    strcpy(filename, inputBuffer);

    int fileIndex = findFileIndex(filename);
    if (fileIndex == -1) {
        Serial.println("Error: File not found");
        return;
    }

    FATEntry file = readFATEntry(fileIndex);
    Serial.print("File content: ");
    for (int i = 0; i < file.length; i++) {
        Serial.print((char)EEPROM.read(file.start + i));
    }
    Serial.println();
}

void erase() {
    Serial.println("Enter filename:");
    while (!inputRoutine());
    char filename[BUFFER_SIZE];
    strcpy(filename, inputBuffer);

    int fileIndex = findFileIndex(filename);
    if (fileIndex == -1) {
        Serial.println("Error: File not found");
        return;
    }

    for (int i = fileIndex; i < noOfFiles - 1; i++) {
        fat[i] = fat[i + 1];
        writeFATEntry(i, fat[i]);
    }
    noOfFiles--;
    Serial.println("File erased successfully");
}

void files() {
    for (int i = 0; i < noOfFiles; i++) {
        Serial.print("Bestand: ");
        Serial.print(fat[i].name);
        Serial.print(" Grootte: ");
        Serial.println(fat[i].length);
    }
}

void freeSpace() {
    int usedSpace = sizeof(FATEntry) * FAT_SIZE;
    for (int i = 0; i < noOfFiles; i++) {
        usedSpace += fat[i].length;
    }
    int availableSpace = EEPROM_SIZE - usedSpace;
    Serial.print("Available space: ");
    Serial.println(availableSpace);
}

void writeFATEntry(int index, FATEntry entry) {
    EEPROM.put(index * sizeof(FATEntry), entry);
}

FATEntry readFATEntry(int index) {
    FATEntry entry;
    EEPROM.get(index * sizeof(FATEntry), entry);
    return entry;
}

int findFileIndex(const char* filename) {
    for (int i = 0; i < noOfFiles; i++) {
        if (strcmp(fat[i].name, filename) == 0) {
            return i;
        }
    }
    return -1;
}

void pushByte(byte b) {
    if (sp < STACK_SIZE) {
        stack[sp++] = b;
        Serial.print("Pushed byte: ");
        Serial.println(b, HEX);
    } else {
        Serial.println("Stack Overflow");
    }
    printStack(); // Voeg deze regel toe voor debugging
}

byte popByte() {
    if (sp > 0) {
        byte b = stack[--sp];
        Serial.print("Popped byte: ");
        Serial.println(b, HEX);
        printStack(); // Voeg deze regel toe voor debugging
        return b;
    } else {
        Serial.println("Stack Underflow");
        return 0;
    }
}

void pushInt(int i) {
    pushByte(highByte(i));  // High byte first
    pushByte(lowByte(i));   // Low byte second
    Serial.print("Pushed int: ");
    Serial.println(i);
}

int popInt() {
    if (sp >= 2) {
        byte low = popByte();  // Low byte first
        byte high = popByte(); // High byte second
        int value = (high << 8) | low;
        Serial.print("Popped int: ");
        Serial.println(value);
        return value;
    } else {
        Serial.println("Stack Underflow");
        return 0;
    }
}

void pushFloat(float f) {
    byte* p = (byte*)&f;
    for (int i = 0; i < 4; i++) {
        pushByte(p[i]);
    }
    Serial.print("Pushed float: ");
    Serial.println(f);
}

float popFloat() {
    if (sp >= 4) {
        float f;
        byte* p = (byte*)&f;
        for (int i = 3; i >= 0; i--) {
            p[i] = popByte();
        }
        Serial.print("Popped float: ");
        Serial.println(f);
        return f;
    } else {
        Serial.println("Stack Underflow");
        return 0.0;
    }
}

void pushString(const char* str) {
    int length = strlen(str) + 1; // inclusief null-terminator
    for (int i = 0; i < length; i++) {
        pushByte(str[i]);
    }
    pushByte(length); // lengte van de string inclusief null-terminator
}

char* popString() {
    if (sp >= 1) {
        int length = popByte();
        static char str[STACK_SIZE];
        for (int i = length - 1; i >= 0; i--) {
            str[i] = popByte();
        }
        return str;
    } else {
        Serial.println("Stack Underflow");
        return NULL;
    }
}

void storeVariable(char name, int processId, char type) {
    int size = (type == 'C') ? 1 : (type == 'I') ? 2 : (type == 'F') ? 4 : 0;
    if (type == 'S') {
        size = popByte(); // lengte van de string
    }

    // Check of de variabele al bestaat
    for (int i = 0; i < noOfVars; i++) {
        if (memoryTable[i].name == name && memoryTable[i].processId == processId) {
            // Variable overschrijven
            int index = memoryTable[i].address;
            for (int j = 0; j < size; j++) {
                memory[index++] = popByte();
            }
            memoryTable[i].size = size;
            memoryTable[i].type = type;
            return;
        }
    }

    // Nieuwe variabele
    if (noOfVars < MAX_VARS) {
        memoryTable[noOfVars].name = name;
        memoryTable[noOfVars].processId = processId;
        memoryTable[noOfVars].type = type;
        memoryTable[noOfVars].size = size;
        int index = 0;
        for (int i = 0; i < MEMORY_SIZE; i++) {
            if (memory[i] == 0) {
                index = i;
                break;
            }
        }
        memoryTable[noOfVars].address = index;
        for (int j = 0; j < size; j++) {
            memory[index++] = popByte();
        }
        noOfVars++;
    } else {
        Serial.println("No space in memory table");
    }
}

Variable retrieveVariable(char name, int processId) {
    for (int i = 0; i < noOfVars; i++) {
        if (memoryTable[i].name == name && memoryTable[i].processId == processId) {
            return memoryTable[i];
        }
    }
    Serial.println("Variable not found");
    return {};
}

void deleteVariable(char name, int processId) {
    for (int i = 0; i < noOfVars; i++) {
        if (memoryTable[i].name == name && memoryTable[i].processId == processId) {
            for (int j = i; j < noOfVars - 1; j++) {
                memoryTable[j] = memoryTable[j + 1];
            }
            noOfVars--;
            return;
        }
    }
    Serial.println("Variable not found");
}

void deleteAllVariables(int processId) {
    for (int i = 0; i < noOfVars; i++) {
        if (memoryTable[i].processId == processId) {
            deleteVariable(memoryTable[i].name, processId);
            i--; // Omdat deleteVariable de lijst inkort
        }
    }
}

void testMemory() {
    // Pushen van data op de stack
    pushInt(1234);
    pushFloat(56.78);
    pushString("Hallo");

    // Opslaan als variabelen
    storeVariable('a', 1, 'I');
    storeVariable('b', 1, 'F');
    storeVariable('c', 1, 'S');

    // Ophalen van variabelen
    Variable varA = retrieveVariable('a', 1);
    Variable varB = retrieveVariable('b', 1);
    Variable varC = retrieveVariable('c', 1);

    // Printen van variabelen
    Serial.print("Variable a: ");
    Serial.println(popInt());
    Serial.print("Variable b: ");
    Serial.println(popFloat());
    Serial.print("Variable c: ");
    Serial.println(popString());

    // Alle variabelen wissen
    deleteAllVariables(1);
}

void startProcess(const char* programName) {
    if (noOfProcesses >= MAX_PROCESSES) {
        Serial.println("Error: No space in process table");
        return;
    }

    int fileIndex = findFileIndex(programName);
    if (fileIndex == -1) {
        Serial.println("Error: Program not found");
        return;
    }

    FATEntry file = readFATEntry(fileIndex);
    Process newProcess;
    strcpy(newProcess.programName, programName);
    newProcess.processId = nextProcessId++;
    newProcess.status = 'r';
    newProcess.pc = file.start;
    newProcess.sp = 0;
    newProcess.fp = 0;
    newProcess.loopStart = file.start;

    processTable[noOfProcesses++] = newProcess;
    Serial.print("Process started with ID: ");
    Serial.println(newProcess.processId);

    // Print details of the new process for debugging
    Serial.print("Program Name: ");
    Serial.println(newProcess.programName);
    Serial.print("Program Counter (PC): ");
    Serial.println(newProcess.pc);
    Serial.print("Start Address: ");
    Serial.println(file.start);
    Serial.print("Program Length: ");
    Serial.println(file.length);
}

void pauseProcess(int processId) {
    int index = findProcessIndex(processId);
    if (index == -1) {
        Serial.println("Error: Process not found");
        return;
    }
    if (processTable[index].status == 'p') {
        Serial.println("Error: Process already paused");
        return;
    }
    processTable[index].status = 'p';
    Serial.print("Process paused with ID: ");
    Serial.println(processId);
}

void resumeProcess(int processId) {
    int index = findProcessIndex(processId);
    if (index == -1) {
        Serial.println("Error: Process not found");
        return;
    }
    if (processTable[index].status == 'r') {
        Serial.println("Error: Process already running");
        return;
    }
    processTable[index].status = 'r';
    Serial.print("Process resumed with ID: ");
    Serial.println(processId);
}

void stopProcess(int processId) {
    int index = findProcessIndex(processId);
    if (index == -1) {
        Serial.println("Error: Process not found");
        return;
    }
    processTable[index].status = 't';
    deleteAllVariables(processId); // Wis alle variabelen die bij dit proces horen
    Serial.print("Process stopped with ID: ");
    Serial.println(processId);
}

void listProcesses() {
    for (int i = 0; i < noOfProcesses; i++) {
        if (processTable[i].status != 't') {
            Serial.print("Process ID: ");
            Serial.print(processTable[i].processId);
            Serial.print(" Name: ");
            Serial.print(processTable[i].programName);
            Serial.print(" Status: ");
            Serial.println(processTable[i].status == 'r' ? "Running" : "Paused");
        }
    }
}

int findProcessIndex(int processId) {
    for (int i = 0; i < noOfProcesses; i++) {
        if (processTable[i].processId == processId) {
            return i;
        }
    }
    return -1;
}

void runProcesses() {
    for (int i = 0; i < noOfProcesses; i++) {
        if (processTable[i].status == 'r') {
            executeInstruction(i);
            // Controleer of het programma het einde heeft bereikt
            FATEntry file = readFATEntry(findFileIndex(processTable[i].programName));
            if (processTable[i].pc >= file.start + file.length) {
                processTable[i].status = 't'; // Beëindig het proces
                Serial.print("Process with ID ");
                Serial.print(processTable[i].processId);
                Serial.println(" has terminated.");
            }
        }
    }
}

void executeInstruction(int processIndex) {
    Process* proc = &processTable[processIndex];
    byte instruction = EEPROM.read(proc->pc++);
    Serial.print("Executing instruction: ");
    Serial.println(instruction, HEX); // Debugging output voor instructies

    switch (instruction) {
        case NOP:
            // Do nothing
            break;
        
        case LOAD_CONST:
            {
                int value = EEPROM.read(proc->pc++);
                Serial.print("LOAD_CONST value: ");
                Serial.println(value); // Debugging output voor LOAD_CONST
                pushInt(value);
            }
            break;

        case ADD:
            if (sp >= 2) {
                int b = popInt();
                int a = popInt();
                Serial.print("ADD ");
                Serial.print(a);
                Serial.print(" + ");
                Serial.println(b);
                pushInt(a + b);
            } else {
                Serial.println("Stack Underflow in ADD");
            }
            break;

        case SUB:
            if (sp >= 2) {
                int b = popInt();
                int a = popInt();
                Serial.print("SUB ");
                Serial.print(a);
                Serial.print(" - ");
                Serial.println(b);
                pushInt(a - b);
            } else {
                Serial.println("Stack Underflow in SUB");
            }
            break;

        case MUL:
            if (sp >= 2) {
                int b = popInt();
                int a = popInt();
                Serial.print("MUL ");
                Serial.print(a);
                Serial.print(" * ");
                Serial.println(b);
                pushInt(a * b);
            } else {
                Serial.println("Stack Underflow in MUL");
            }
            break;

        case DIV:
            if (sp >= 2) {
                int b = popInt();
                int a = popInt();
                Serial.print("DIV ");
                Serial.print(a);
                Serial.print(" / ");
                Serial.println(b);
                if (b != 0) {
                    pushInt(a / b);
                } else {
                    Serial.println("Error: Division by zero");
                }
            } else {
                Serial.println("Stack Underflow in DIV");
            }
            break;

        case PRINT:
            if (sp >= 1) {
                int value = popInt();
                Serial.print("PRINT: ");
                Serial.println(value);
            } else {
                Serial.println("Stack Underflow in PRINT");
            }
            break;

        case STOP:
            executeStop(proc);
            break;
        case IF:
            executeIf(proc);
            break;
        case ELSE:
            executeElse(proc);
            break;
        case ENDIF:
            executeEndIf(proc);
            break;
        case LOOP:
            executeLoop(proc);
            break;
        case ENDLOOP:
            executeEndLoop(proc);
            break;
        case WHILE:
            executeWhile(proc);
            break;
        case ENDWHILE:
            executeEndWhile(proc);
            break;
        case MILLIS:
            executeMillis(proc);
            break;
        case DELAYUNTIL:
            executeDelayUntil(proc);
            break;
        case OPEN:
            executeOpen(proc);
            break;
        case WRITE:
            executeWrite(proc);
            break;
        case READCHAR:
            executeReadChar(proc);
            break;
        case READINT:
            executeReadInt(proc);
            break;
        case READFLOAT:
            executeReadFloat(proc);
            break;
        case READSTR:
            executeReadStr(proc);
            break;
        case CLOSE:
            executeClose(proc);
            break;
        case FORK:
            executeFork(proc);
            break;
        case WAITUNTILDONE:
            executeWaitUntilDone(proc);
            break;

        default:
            Serial.println("Unknown instruction");
            break;
    }
}

void loadExampleProgram() {
    // Laad een voorbeeldprogramma in het EEPROM
    byte program[] = {
        // Initialize and print values
        LOAD_CONST, 10,    // Load 10 onto the stack
        PRINT,             // Print 10

        LOAD_CONST, 20,    // Load 20 onto the stack
        PRINT,             // Print 20

        // Perform a simple loop
        LOAD_CONST, 5,     // Load 5 onto the stack (loop counter)
        LOOP,              // Start loop
        PRINT,             // Print loop counter
        LOAD_CONST, 1,     // Load 1 onto the stack
        SUB,               // Subtract 1 from loop counter
        ENDLOOP,           // End loop (will jump back to LOOP if counter > 0)
        NOP                // No operation (end of program)
    };

    FATEntry exampleFile;
    strcpy(exampleFile.name, "example");
    exampleFile.start = 0;
    exampleFile.length = sizeof(program);

    // Voeg de FAT-entry toe
    writeFATEntry(noOfFiles, exampleFile);
    fat[noOfFiles++] = exampleFile;

    // Schrijf het programma naar EEPROM
    for (int i = 0; i < sizeof(program); i++) {
        EEPROM.write(exampleFile.start + i, program[i]);
    }

    // Print de EEPROM-inhoud om te controleren
    printEEPROMContents(exampleFile.start, exampleFile.length);
}

void printEEPROMContents(int start, int length) {
    Serial.print("EEPROM Contents from ");
    Serial.print(start);
    Serial.print(" to ");
    Serial.println(length);
    for (int i = start; i < start + length; i++) {
        byte val = EEPROM.read(i);
        Serial.print(val, HEX);
        Serial.print(" ");
        if ((i - start + 1) % 16 == 0) {
            Serial.println();
        }
    }
    Serial.println();
}

void printStack() {
    Serial.print("Stack [");
    for (int i = 0; i < sp; i++) {
        Serial.print(stack[i], HEX);
        if (i < sp - 1) {
            Serial.print(", ");
        }
    }
    Serial.println("]");
}

// Functie implementaties voor de nieuwe instructies
void executeStop(Process* proc) {
    proc->status = 't';
    deleteAllVariables(proc->processId);
    Serial.print("Process with ID ");
    Serial.print(proc->processId);
    Serial.println(" has terminated.");
}

void executeIf(Process* proc) {
    byte lengthOfTrueCode = EEPROM.read(proc->pc++);
    float condition = popFloat();
    if (condition == 0) {
        proc->pc += lengthOfTrueCode;
    }
}

void executeElse(Process* proc) {
    byte lengthOfFalseCode = EEPROM.read(proc->pc++);
    float condition = popFloat();
    if (condition != 0) {
        proc->pc += lengthOfFalseCode;
    }
}

void executeEndIf(Process* proc) {
    popFloat(); // Remove the condition from the stack
}

void executeLoop(Process* proc) {
    proc->loopStart = proc->pc;
}

void executeEndLoop(Process* proc) {
    proc->pc = proc->loopStart;
}

void executeWhile(Process* proc) {
    byte lengthOfConditionCode = EEPROM.read(proc->pc++);
    byte lengthOfRepeatedCode = EEPROM.read(proc->pc++);
    float condition = popFloat();
    if (condition == 0) {
        proc->pc += lengthOfRepeatedCode + 1;
    } else {
        pushInt(lengthOfConditionCode + lengthOfRepeatedCode + 4);
    }
}

void executeEndWhile(Process* proc) {
    int jumpBack = popInt();
    proc->pc -= jumpBack;
}

void executeMillis(Process* proc) {
    unsigned long currentMillis = millis();
    pushInt(currentMillis);
}

void executeDelayUntil(Process* proc) {
    unsigned long targetMillis = popInt();
    if (millis() < targetMillis) {
        pushInt(targetMillis);
        proc->pc--;
    }
}

void executeOpen(Process* proc) {
    char* filename = popString();
    int length = popInt();
    int fileIndex = findFileIndex(filename);
    if (fileIndex == -1) {
        FATEntry newFile;
        strcpy(newFile.name, filename);
        newFile.start = EEPROM_SIZE / 2 + noOfFiles * sizeof(FATEntry);
        newFile.length = length;
        writeFATEntry(noOfFiles, newFile);
        fat[noOfFiles++] = newFile;
        proc->fp = newFile.start;
    } else {
        FATEntry existingFile = readFATEntry(fileIndex);
        proc->fp = existingFile.start;
    }
}

void executeWrite(Process* proc) {
    byte type = popByte();
    int size = (type == 'C') ? 1 : (type == 'I') ? 2 : (type == 'F') ? 4 : EEPROM.read(proc->fp);
    for (int i = 0; i < size; i++) {
        EEPROM.write(proc->fp++, popByte());
    }
}

void executeRead(Process* proc, byte type) {
    int size = (type == 'C') ? 1 : (type == 'I') ? 2 : (type == 'F') ? 4 : EEPROM.read(proc->fp);
    for (int i = 0; i < size; i++) {
        pushByte(EEPROM.read(proc->fp++));
    }
    pushByte(type);
}

void executeReadChar(Process* proc) {
    executeRead(proc, 'C');
}

void executeReadInt(Process* proc) {
    executeRead(proc, 'I');
}

void executeReadFloat(Process* proc) {
    executeRead(proc, 'F');
}

void executeReadStr(Process* proc) {
    byte length = EEPROM.read(proc->fp++);
    for (int i = 0; i < length; i++) {
        pushByte(EEPROM.read(proc->fp++));
    }
    pushByte(length);
    pushByte('S');
}

void executeClose(Process* proc) {
    // No operation needed for EEPROM
}

void executeFork(Process* proc) {
    char* programName = popString();
    startProcess(programName);
    pushInt(nextProcessId - 1);
}

void executeWaitUntilDone(Process* proc) {
    int processId = popInt();
    int index = findProcessIndex(processId);
    if (index == -1 || processTable[index].status != 't') {
        pushInt(processId);
        proc->pc--;
    }
}

void loadIfElseProgram() {
    byte program[] = {
        LOAD_CONST, 10,    // Laad 10 op de stack
        IF, 4,             // Als 10 != 0, voer volgende 4 instructies uit
        LOAD_CONST, 20,    // Laad 20 op de stack
        PRINT,             // Print 20
        ELSE, 4,           // Anders, voer volgende 4 instructies uit
        LOAD_CONST, 30,    // Laad 30 op de stack
        PRINT,             // Print 30
        ENDIF,             // Einde van IF-ELSE-structuur
        NOP                // Geen operatie (einde programma)
    };

    FATEntry ifElseFile;
    strcpy(ifElseFile.name, "ifelse");
    ifElseFile.start = noOfFiles * sizeof(FATEntry) + EEPROM_SIZE / 2;
    ifElseFile.length = sizeof(program);

    writeFATEntry(noOfFiles, ifElseFile);
    fat[noOfFiles++] = ifElseFile;

    for (int i = 0; i < sizeof(program); i++) {
        EEPROM.write(ifElseFile.start + i, program[i]);
    }

    printEEPROMContents(ifElseFile.start, ifElseFile.length);
}

void loadLoopProgram() {
    byte program[] = {
        LOOP,              // Start loop
        LOAD_CONST, 40,    // Laad 40 op de stack
        PRINT,             // Print 40
        ENDLOOP,           // Einde loop (spring terug naar begin loop)
        NOP                // Geen operatie (einde programma)
    };

    FATEntry loopFile;
    strcpy(loopFile.name, "loop");
    loopFile.start = noOfFiles * sizeof(FATEntry) + EEPROM_SIZE / 2;
    loopFile.length = sizeof(program);

    writeFATEntry(noOfFiles, loopFile);
    fat[noOfFiles++] = loopFile;

    for (int i = 0; i < sizeof(program); i++) {
        EEPROM.write(loopFile.start + i, program[i]);
    }

    printEEPROMContents(loopFile.start, loopFile.length);
}

void loadWhileProgram() {
    byte program[] = {
        LOAD_CONST, 50,    // Laad 50 op de stack
        WHILE, 2, 4,       // Zolang de waarde != 0, voer volgende 4 instructies uit
        PRINT,             // Print de waarde
        LOAD_CONST, 0,     // Laad 0 op de stack om de conditie te beëindigen
        ENDWHILE,          // Einde while-loop
        NOP                // Geen operatie (einde programma)
    };

    FATEntry whileFile;
    strcpy(whileFile.name, "while");
    whileFile.start = noOfFiles * sizeof(FATEntry) + EEPROM_SIZE / 2;
    whileFile.length = sizeof(program);

    writeFATEntry(noOfFiles, whileFile);
    fat[noOfFiles++] = whileFile;

    for (int i = 0; i < sizeof(program); i++) {
        EEPROM.write(whileFile.start + i, program[i]);
    }

    printEEPROMContents(whileFile.start, whileFile.length);
}
