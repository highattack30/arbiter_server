////////////////////////////////////////////////////////////
/////////////////Writed by Balan Narcis/////////////////////
////////Created to be used with Tera Server [Sky Lake]//////
////////////////////////////////////////////////////////////

//use only for big-edian

#ifndef STREAM_H
#define STREAM_H

#include "typeDefs.h"

#include "serverEnums.h"

#include <iostream>
#include <vector>
#include <memory>
#include <string>

#include "typeDefs.h"

#ifndef STREAM_MAX_SIZE
#define STREAM_MAX_SIZE 0xffffu //max size of tera packet
#endif

class player;
class Stream
{

public:
	Stream();
	Stream(uint16 size);
	Stream(byte* data, uint16 size);
	Stream(std::istream * str);

	Stream(Stream&);
	~Stream();


	//WRITE_*******************************************************
	void				Resize(uint16 size);

	void				Write(byte* data, uint16 size);

	void				WriteString(std::string s, bool L_ED = false);

	void				WriteUInt8(byte);

	void				WriteInt16(int16);
	void				WriteUInt16(uint16);

	void				WriteInt32(int32);
	void				WriteUInt32(int32);

	void				WriteInt64(int64);
	void				WriteUInt64(uint64);

	void				WriteFloat(float);
	void				WriteDouble(double);

	void				WritePos(uint16, int16 offset = 0);



	//READ_********************************************************
	void				Read(byte* out_buffer, uint16 size);

	char				ReadInt8();
	byte				ReadUInt8();

	int16				ReadInt16();
	uint16				ReadUInt16();

	int32				ReadInt32();
	uint32				ReadUInt32();

	int64				ReadInt64();
	uint64				ReadUInt64();

	float				ReadFloat();
	double				ReadDouble();

	std::string			ReadUTF16StringLittleEdianToASCII();
	std::string			ReadUTF16StringBigEdianToASCII();
	std::string			ReadASCIIString();
	bool				ReadASCIIStringTo(byte * out, uint16 max_len);

	

	//SPECIAL_TERA_SERVER_**************************************************
	void				WriteWorldId(std::shared_ptr<player>);
	void				WriteSpawnId(std::shared_ptr<player>, bool login = false);

	
	//MISC_**************************************************
	void				Clear();
	uint16				SetEnd();
	uint16				SetFront();
	uint16				NextPos();


	//MEMBER_VAR_**************************************************
	byte*				_raw;
	uint16				_size;
	uint16				_pos;
};

#endif



