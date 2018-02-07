#define LIST_REQUEST 0xA1
#define LIST_REPLY 0xA2
#define GET_REQUEST 0xB1
#define GET_REPLY_EXIST 0xB2
#define GET_REPLY_NON_EXIST 0xB3
#define PUT_REQUEST 0xC1
#define PUT_REPLY 0xC2
#define FILE_DATA 0xFF

#define HEADER_SIZE 10

// Header Stucture
struct message_s {
	unsigned char protocol[5]; /* protocol string (5 bytes) */
	unsigned char type; /* type (1 byte) */
	unsigned int length; /* length (header + payload) (4 bytes) */
} __attribute__ ((packed));

struct message_s *createHeader(unsigned char type, unsigned int length);
char *encodeHeader(struct message_s *header);
struct message_s *decodeHeader(char* message);
char *generatePacket(struct message_s *header, char* payload, int packetSize);
void *explodePacket(char* packet, struct message_s **header_src, char **payload);
int getFileSize(FILE *file);
int *fragmentateFile(FILE *file, unsigned int size);
char *readFileToByte(FILE *file);