#include <DynamicBuffer.h>

// Create a dynamic buffer with 8 slots of 16 bytes.
// This makes a maximum size of 128 bytes.
DynamicBuffer<char> buffer(16, 8);
char shortString[] = "Short string!\n";
char mediumString[] = "Medium string!!!!!!!!!\n";
char longString[] = "Long string!!!!!!!!!!!!!!!!!!!!!!!!!\n";

char printBuffer[64] = {};
int count = 0;
int8_t previousSlot = SLOT_FREE;

// This function prints out the free ram to demonstrate that usage does not increase without allocation and deallocation.
void freeRam () {
   extern int __heap_start, *__brkval;
   int v;
   Serial.print((int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval));
   Serial.println(" bytes of ram free");
}

void setup() {
  Serial.begin(9600);
  // Allocate enoguh memory for longString and copy longString into the buffer.
  int8_t slot = buffer.allocate(longString, sizeof(longString));
  // Get the accessor class for the assigned buffer. This class supports the array operator,
  // size() and a number of other utility functions.
  DynamicBuffer<char>::Buffer accessor = buffer.getBuffer(slot);
  // Write the dynamic buffer to a fixed size array for printing.
  accessor.write(printBuffer, 64);
  Serial.print(printBuffer);
}

void loop() {
  delay(500);
  freeRam();
  Serial.print(buffer.getFree());
  Serial.println(" free bytes in dynamic buffer");
  int8_t slot;
  // Each time through allocate a different size.
  switch (count % 3) {
    case 0:
      slot = buffer.allocate(shortString, sizeof(shortString));
      break;
    case 1:
      slot = buffer.allocate(mediumString, sizeof(mediumString));
      break;
    case 2:
      slot = buffer.allocate(longString, sizeof(longString));
      break;
  }
  // Then free the previous buffer. This should cause some fragmentation.
  buffer.free(previousSlot);
  previousSlot = slot;
  DynamicBuffer<char>::Buffer accessor = buffer.getBuffer(slot);
  accessor.write(printBuffer, 64);
  Serial.print(printBuffer);
  count++;
}
