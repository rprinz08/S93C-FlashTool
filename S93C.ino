// Read and Write S93C serial flash

#define OP_SPECIAL  0x0
#define OP_EWEN     0x0
#define OP_EWDS     0x0
#define OP_ERAL     0x0
#define OP_WRITE    0x1
#define OP_READ     0x2
#define OP_ERASE    0x3


void _sendStartBit() {
    Select(true);
    digitalWrite(PIN_SK, LOW);
    
    digitalWrite(PIN_DI, HIGH);
    digitalWrite(PIN_SK, HIGH);
    digitalWrite(PIN_SK, LOW);
}


void _sendCommand(byte command) {
    digitalWrite(PIN_DI, ((command & 0x2) == 0 ? LOW : HIGH));
    digitalWrite(PIN_SK, HIGH);
    digitalWrite(PIN_SK, LOW);

    digitalWrite(PIN_DI, ((command & 0x1) == 0 ? LOW : HIGH));
    digitalWrite(PIN_SK, HIGH);
    digitalWrite(PIN_SK, LOW);
}


void _sendAddress(int addr) {
    for(int i=7; i>=0; i--) {
        digitalWrite(PIN_DI, ((addr & (0x1 << i)) == 0 ? LOW : HIGH));
        digitalWrite(PIN_SK, HIGH);
        digitalWrite(PIN_SK, LOW);
    }
}


void _sendWord(uint16_t data) {
    for(int i=15; i>=0; i--) {
        digitalWrite(PIN_DI, ((data & (0x1 << i)) == 0 ? LOW : HIGH));
        digitalWrite(PIN_SK, HIGH);
        digitalWrite(PIN_SK, LOW);
    }
}


uint16_t _readWord(boolean dummy_bit) {
    uint16_t data = 0;
    for(int i=0; i<16; i++) {
        digitalWrite(PIN_SK, HIGH);
        digitalWrite(PIN_SK, LOW);
        
        uint8_t data_bit = digitalRead(PIN_DO);
        //Serial.print((data_bit ? "+" : "."));

        data = data | data_bit;
        if(i<15)
            data = data << 1;
    }
    //Serial.println();
    return data;
}


int Select(boolean selected) {
    digitalWrite(PIN_CS, selected);
}


void Reset() {
    pinMode(PIN_DI, OUTPUT);
    digitalWrite(PIN_SK, LOW);
    digitalWrite(PIN_DI, LOW);
    Select(false);
}


void ReadAddress(int addr, uint16_t words, boolean withCRC,
    int (*callback)(uint16_t, uint16_t, boolean)) {
        
    if(words < 1)
        return;
        
    pinMode(PIN_DI, OUTPUT);
    
    _sendStartBit();
    _sendCommand(OP_READ);
    _sendAddress(addr);
    
    digitalWrite(PIN_DI, LOW);
    pinMode(PIN_DI, INPUT_PULLUP);

    int addr_i = addr;
    for(uint16_t i=0; i<words; i++) {
        if(callback != NULL)
            callback(addr_i, _readWord(i == 0), withCRC);
        addr_i++;
    }
}


void EnableWrite() {
    pinMode(PIN_DI, OUTPUT);
    
    _sendStartBit();
    _sendCommand(OP_EWEN);
    _sendAddress(0xC0);
    Select(false);
}


void DisableWrite() {
    pinMode(PIN_DI, OUTPUT);
    
    _sendStartBit();
    _sendCommand(OP_EWDS);
    _sendAddress(0);
    Select(false);
}


void WriteAddress(int addr, uint16_t data) {
    pinMode(PIN_DI, OUTPUT);
    
    _sendStartBit();
    _sendCommand(OP_WRITE);
    _sendAddress(addr);
    _sendWord(data);
    Select(false);
    delay(10);
}


void EraseAll() {
    pinMode(PIN_DI, OUTPUT);
    
    _sendStartBit();
    _sendCommand(OP_ERAL);
    _sendAddress(0x80);
    Select(false);
    delay(100);
}


int ReadNext() {
    return _readWord(false);
}
