#ifndef CODEC_H
#define CODEC_H

#include "SequenceASN1.h"

class Codec : public  SequenceASN1{
public:
	Codec();
	virtual ~Codec();
	virtual int msgEncode(char** outData, int& len);

	virtual void* msgDecode(char* inData, int inLen);
};

#endif
