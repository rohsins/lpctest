#include <stdint.h>

#define BUFSIZE 34
#define RINGBUFFSIZE 34
#define RINGBUFFLENGTH (RINGBUFFSIZE + 1)

class ringBuffer {

	char ringBufferVariable[RINGBUFFLENGTH];
	char temp[34];
	char *readout;
	
	unsigned int head, tail;
	
	ringBuffer(void) {
		readout = temp;
		head = tail = 0;
	}
	
	uint32_t ringBufferWrite(char in) {
		if (head == ((tail - 1 + RINGBUFFLENGTH) % RINGBUFFLENGTH)) {
			return 1;
		}
		
		ringBufferVariable[head] = in;
		head = (head + 1) % RINGBUFFLENGTH;
		return 0;
	}
	
	uint32_t ringBufferRead(char *out) {
		if (head == tail) {
			return 1;
		}
		*out = ringBufferVariable[tail];
		ringBufferVariable[tail] = 0;
		tail = ((tail + 1) % RINGBUFFLENGTH);
		return 0;
	}
};
