#include "RingBuffer.h"

uint32_t RingBuffer::ringBufferWrite(char in) {
		if (head == ((tail - 1 + RINGBUFFLENGTH) % RINGBUFFLENGTH)) {
			return 1;
		}
		ringBufferVariable[head] = in;
		head = (head + 1) % RINGBUFFLENGTH;
		return 0;
}

uint32_t RingBuffer::ringBufferStringWrite(char in[]) {
	int charinc = 0;
	while (in[charinc])	{
		if (head == ((tail - 1 + RINGBUFFLENGTH) % RINGBUFFLENGTH)) {
			return 1;
		}
		ringBufferVariable[head] = in[charinc++];
		head = (head + 1) % RINGBUFFLENGTH;
	}
	return 0;
}

uint32_t RingBuffer::ringBufferRead(char *out) {
		if (head == tail) {
			return 1;
		}
		*out = ringBufferVariable[tail];
		ringBufferVariable[tail] = 0;
		tail = ((tail + 1) % RINGBUFFLENGTH);
		return 0;
}
