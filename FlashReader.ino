
#define PIN_CS                 7
#define PIN_SK                 6
#define PIN_DI                 5
#define PIN_DO                 4

#define FLAG_SHOW_PROMPT       1
#define SHOW_PROMPT            ((prgFlags & FLAG_SHOW_PROMPT) == FLAG_SHOW_PROMPT)

byte prgFlags = FLAG_SHOW_PROMPT;
unsigned int crc_sum;


void setup() {
    Serial.begin(19200);

    pinMode(PIN_CS, OUTPUT);
    pinMode(PIN_SK, OUTPUT);
    pinMode(PIN_DI, OUTPUT);
    pinMode(PIN_DO, INPUT_PULLUP);
    Reset();

    Usage();
}


void loop() {
    byte inByte;
    int data;
    char buf[255];


    if(SHOW_PROMPT)
        Serial.print(">");
    
    if(Serial.available() > 0) {
        inByte = Serial.read();
        Serial.write(inByte);
        Serial.println();
    
        switch(inByte) {
            // --------------------------------------------------------------------------------
            // Primary functions
            // Dump full memory in HEX (direct mode)
            case 'd':
                StartTextResult(true);
                crc_sum = 0;
                ReadAddress(0x00, 256, false, DumpWord);
                Reset();
                SendOK("DUMP done");
                break;

            // Dump full memory in HEX with CRC (direct mode)
            case 'D':
                StartTextResult(true);
                crc_sum = 0;
                ReadAddress(0x00, 256, true, DumpWord);
                Reset();
                SendOK("DUMP (CRC) done");
                break;

            // Dump full memory in ASCII (direct mode)
            case 'a':
                StartTextResult(true);
                ReadAddress(0x00, 256, false, DumpText);
                Reset();
                SendOK("DUMP done");
                break;

            // Dump full memory in ASCII (one-by-one mode)
            case 'A':
                StartTextResult(true);
                for(int i=0; i<256; i++) {
                    ReadAddress(i, 1, false, DumpText);
                    Reset();
                }                
                SendOK("DUMPx done");
                break;
            
            // Upload
            case 'u':
                StartTextResult(true);
                ProcessUpload(false);
                Serial.println();
                SendOK("UPLOAD done");
                break;

            // Upload with CRC
            case 'U':
                StartTextResult(true);
                ProcessUpload(true);
                Serial.println();
                SendOK("UPLOAD done");
                break;

            // Erase ALL memory
            case 'E':
                StartTextResult(true);
                if(AskYesNo("Erase ALL memory [y/n]: ")) {
                    EnableWrite();
                    EraseAll();
                    DisableWrite();
                    SendOK("EARSE ALL done");
                }
                else
                    SendOK("EARSE ALL - NOT - done");
                break;

            
            // --------------------------------------------------------------------------------
            // Development functions
            
            // Chip select
            case 'S':
                StartTextResult(false);
                digitalWrite(PIN_CS, HIGH);
                SendOK("CHIP_SEL HIGH");
                break;
            case 's':
                StartTextResult(false);
                digitalWrite(PIN_CS, LOW);
                SendOK("CHIP_SEL low");
                break;

            // Clock
            case 'C':
                StartTextResult(false);
                digitalWrite(PIN_SK, HIGH);
                SendOK("SER_CLK HIGH");
                break;
            case 'c':
                StartTextResult(false);
                digitalWrite(PIN_SK, LOW);
                SendOK("SER_CLK low");
                break;

            // Data IN (to chip)
            case 'I':
                StartTextResult(false);
                digitalWrite(PIN_DI, HIGH);
                SendOK("DATA_IN HIGH");
                break;
            case 'i':
                StartTextResult(false);
                digitalWrite(PIN_DI, LOW);
                SendOK("DATA_IN low");
                break;

            // Data OUT (from chip)
            case 'r':
                StartTextResult(false);
                data = digitalRead(PIN_DO);
                snprintf(buf, sizeof(buf), "DATA_OUT %s",
                    (data ? "HIGH" : "low"));
                SendOK(buf);
                break;

            // Test read 2 words at address 0x00           
            case 't':
                StartTextResult(true);
                ReadAddress(0x00, 2, false, ProcessWord);
                Reset();
                SendOK("Read TEST done");
                break;
            // Test write a word to addr 0x00
            case 'T':
                StartTextResult(true);
                WriteAddress(0x00, 0x1234);
                Reset();
                SendOK("Write TEST done");
                break;

            // Disable write mode
            case 'w':
                StartTextResult(false);
                DisableWrite();
                SendOK("Disable write");
                break;

            // Enable write mode
            case 'W':
                StartTextResult(false);
                EnableWrite();
                SendOK("Enable write");
                break;

            // Reset all signal lines
            case 'R':
                StartTextResult(false);
                Reset();
                SendOK("RESET done");
                break;

            // Help
            case 'h':
            case '?':
                Usage();
                break;

            default:
                StartTextResult(false);
                SendOK("UNK");
        }
        prgFlags |= FLAG_SHOW_PROMPT;
    }
    else {
        prgFlags &= ~(FLAG_SHOW_PROMPT);
        delay(200);
    }                
}


void Usage() {
    Serial.println("\nS93C Flash Tool V1.0");

    Serial.println("\n--- Main tools ---");
    Serial.println("d   - Dump as HEX");
    Serial.println("D   - Dump as HEX with CRC");
    Serial.println("a   - Dump as ASCII");
    Serial.println("A   - Dump as ASCII (one-by-one)");
    Serial.println("u   - Upload HEX");
    Serial.println("U   - Upload HEX with CRC");   
    Serial.println("E   - Erase All");

    Serial.println("\n--- Development tools ---");
    Serial.println("S/s - Chip select On/Off");
    Serial.println("C/c - Clock On/Off");
    Serial.println("I/i - Data-IN On/Off");    
    Serial.println("r   - Report Data-OUT state");
    Serial.println("R   - Reset all signals");
    Serial.println("t   - Read 2 words at addr 0x00");
    Serial.println("T   - Test write to addr 0x00");
    Serial.println("W/w - Write mode On/Off");
    
    Serial.println("h/? - This help");
}


void StartTextResult(boolean MultiLine) {
    Serial.print( (MultiLine ? "=\r\n" : "-\r\n") );
}


void SendOK(const char *Message) {
    char buf[255];
    snprintf(buf, sizeof(buf), "OK %010lu: %s",
        millis(), Message);
    Serial.println(buf);
}


void SendERR(const char *Message) {
    char buf[255];
    snprintf(buf, sizeof(buf), "ERR %010lu: %s",
        millis(), Message);
    Serial.println(buf);
}


void SendMSG(const char *Message) {
    if(Message != NULL)
        Serial.print(Message);
}


boolean IsCanceled() {
    byte inByte;
    
    if(Serial.available() > 0) {
        inByte = Serial.read();
        if(inByte == '.')
            return true;
    }
    
    return false;
}


int ProcessWord(uint16_t addr, uint16_t data, boolean withCRC) {
    char buf[255];
    uint8_t h = (data >> 8) & 0x00ff;
    uint8_t l = data & 0x00ff;
    snprintf(buf, sizeof(buf), "%04x:   %02x %02x   %c%c",
        addr, l, h,
        (l<32 || l>126 ? '.' : l),
        (h<32 || h>126 ? '.' : h));
    Serial.println(buf);    
}


int DumpWord(uint16_t addr, uint16_t data, boolean withCRC) {
    char buf[255];
    uint8_t h = (data >> 8) & 0x00ff;
    uint8_t l = data & 0x00ff;
    uint8_t crc = 0;

    if(((addr) % 4) == 0) {
        snprintf(buf, sizeof(buf), "%04x : ", addr);
        Serial.print(buf);
    }

    snprintf(buf, sizeof(buf), "%02x%02x", l, h);
    crc_sum += l;
    crc_sum += h;
    Serial.print(buf);
    if(((addr + 1) % 4) == 0) {
        if(withCRC) {
            crc = crc_sum % 256;
            snprintf(buf, sizeof(buf), " : %02x", crc);
            Serial.print(buf);
        }
        Serial.println();
        crc_sum = 0;
    }
    return crc;
}


int DumpText(uint16_t addr, uint16_t data, boolean withCRC) {
    char buf[255];
    uint8_t h = (data >> 8) & 0x00ff;
    uint8_t l = data & 0x00ff;    
    snprintf(buf, sizeof(buf), "%c%c",
        (l<32 || l>126 ? '.' : l),
        (h<32 || h>126 ? '.' : h));
    Serial.print(buf);
    if(((addr+1) % 4) == 0)
        Serial.println();
    return 0;
}


// askYesNo shows prompt and waits until
// user enters yes or no.
// Yes could be (y,Y,j,J) and No (n,N)
// Return true for YES and false for NO
boolean AskYesNo(const char *prompt) {
    uint8_t inByte;
    
    Serial.print(prompt);
    while(true) {
        if(Serial.available() > 0) {
            inByte = Serial.read();
            
            if(inByte == 'Y' || inByte == 'J' || inByte == 'y' || inByte == 'j') {
                Serial.println("Y");
                return true;
            }
            
            if(inByte == 'N' || inByte == 'n') {
                Serial.println("N");
                return false;
            }
        }
        
        delay(100);
    }
}
