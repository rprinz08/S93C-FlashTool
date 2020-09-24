
#define BYTES_PER_LINE  8
const unsigned int NIBBLES_PER_LINE = BYTES_PER_LINE * 2;
const unsigned int WORDS_PER_LINE = BYTES_PER_LINE / 2;


uint8_t ReadLine(uint8_t data[BYTES_PER_LINE+1], boolean withCRC) {    
    uint8_t inByte;
    uint8_t dataIn;
    uint8_t nibblesRead = 0;
    unsigned int bytesRead = 0;

    memset(data, 0, BYTES_PER_LINE);
    while(bytesRead < 2 + BYTES_PER_LINE + (withCRC ? 1 : 0)) {
        if(Serial.available() > 0) {
            inByte = Serial.read();
            
            if(inByte == 27 || inByte == 'q' || inByte == 'Q')
                return 1;
                
            if((inByte >= '0' && inByte <= '9') ||
                    (inByte >= 'a' && inByte <= 'f') ||
                    (inByte >= 'A' && inByte <= 'F')) {
                        
                Serial.print((char)inByte);
                        
                dataIn = (dataIn << 4) | HexToByte(inByte);
                nibblesRead++;
                
                if((nibblesRead % 2) == 0) {
                    nibblesRead = 0;
                    data[bytesRead] = dataIn;
                    bytesRead++;
                }
            }
        }
    }

    return 0;
}


byte HexToByte(char c) {
    if((c >= '0') && (c <= '9'))
        return c - '0';

    if((c >= 'a') && (c <= 'f'))
        return (c - 'a') + 10;

    if((c >= 'A') && (c <= 'F'))
        return (c - 'A') + 10;
}


void ProcessUpload(boolean withCRC) {
    uint8_t quit = 0;
    uint8_t data[2 + BYTES_PER_LINE + (withCRC ? 1 : 0)];
    unsigned int addr = 0;
    unsigned int start_addr = 0;
    uint16_t data_word = 0;
    uint8_t crc;
    char buf[255];

    EnableWrite();
    
    snprintf(buf, sizeof(buf), "read (A=addr, D=DATA%s) as HEX chars",
        (withCRC ? ", C=CRC" : ""));
    Serial.println(buf);
    Serial.println("q/Q/ESC - ends input");
    snprintf(buf, sizeof(buf), "AAAADDDDDDDDDDDDDDDD%s",
        (withCRC ? "CC" : ""));
    Serial.println(buf);

    do {        
        quit = ReadLine(data, withCRC);
        Serial.println();
        start_addr = (data[0] << 8) | data[1];
        addr = start_addr;
        
        if(!quit) {
            // Show ouput of entered data
            for(int i=2; i<2+BYTES_PER_LINE; i+=2) {
                data_word = (data[i+1] << 8) | data[i];
                crc = DumpWord(addr, data_word, withCRC);
                addr++;
            }

            // check CRC
            if(withCRC && crc != data[2+BYTES_PER_LINE]) {
                Serial.println("CRC missmatch - repeat last line");
            }
            else {
                // If CRC OK - write to flash
                addr = start_addr;
                for(int i=2; i<2+BYTES_PER_LINE; i+=2) {
                    data_word = (data[i+1] << 8) | data[i];
                    WriteAddress(addr, data_word);
                    addr++;
                }
            }
        }
    } while(!quit);

    DisableWrite();
}
