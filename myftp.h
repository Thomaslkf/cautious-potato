struct message_s {
	unsigned char protocol[5]; /* protocol string (5 bytes) */
	unsigned char type; /* type (1 byte) */
	unsigned int length; /* length (header + payload) (4 bytes) */
} __attribute__ ((packed));
