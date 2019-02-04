#ifndef MP3DECODER_H
#define MP3DECODER_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct MP3DEC_ MP3DEC_;

extern MP3DEC_ MP3DEC;

#define MP3_BUFFER_SIZE (4*1152)

struct MP3DEC_{
	void (*init)();

	int (*loadFile)(const char* fileName);

	int (*decode)(short * out,unsigned* len);

	unsigned char* (*getMP3Info)(const char* tag,int* size);

	int (*onFile)();

	void (*unloadFile)();
};
#ifdef __cplusplus
}
#endif



#endif
