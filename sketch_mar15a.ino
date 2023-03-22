#include <stdint.h>

#include <nRF24L01.h>
#include <RF24.h>
#include <printf.h>
#include <RF24_config.h>
#include <SPI.h>

//SERIAL
const int timeout = 10000;
const int argc = 6;
int16_t transmission[6] = {-100, 100, 100, -100, 100, 100};

namespace codes {
  const static uint8_t COMMSTART = 0;
  const static uint8_t COMMEND = 1;
  const static uint8_t ACK = 2;
  const static uint8_t ERR = 3;
}

bool wait_for_serial(int bytes = 1) {
  int cycles = 0;

  while(!(Serial.available() >= bytes)/* && cycles < timeout*/) {
    ++cycles;
  }

  return Serial.available() >= bytes;
}

// True = sucesso
// False = falhou
bool recieverProtocol() {
  if(Serial.read() != codes::COMMSTART) {
    Serial.write(codes::ERR);
    return false;
  }

  Serial.write(codes::COMMSTART);

  for(int args = 0; args != argc; ++args) {
    if(!wait_for_serial(2)) {
      Serial.write(codes::ERR);
      return false;
    }

    Serial.readBytes(reinterpret_cast<byte*>(&transmission[args]), 2);

    Serial.write(codes::ACK);
  }

  if(!wait_for_serial() || Serial.read() != codes::COMMEND) {
    Serial.write(codes::ERR);
    return false;
  }

  Serial.write(codes::COMMEND);
  return true;
}

//NRF
RF24 radio(7, 8); // CE, CSN
const byte address[6] = "00001";
//uint64_t address = 0xF0F0F0F0E1LL;

void setup() {
  Serial.begin(9600);
  printf_begin();

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  radio.printDetails();

  Serial.flush();
}

int16_t t1[6] = {-100, 100, 100, -100, 100, 100};
int16_t t2[6] = {100, -100, -100, 100, -100, -100};


void loop() {
  if (Serial.available()) {
    if (recieverProtocol()) {
      //transmit_data();
    } else Serial.flush();
  }
  
  // 12 Bytes: 2 bytes por motor [-255, 255], 2 numeros por arduino (2 motores) e 3 robos
  Serial.println( radio.write(&t1, sizeof(t1)) );
  delay(2000);
  Serial.println( radio.write(&t2, sizeof(t2)) );
  delay(2000);
}
