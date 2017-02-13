#include "fastcgi.h"

FCGI_Header makeHeader (
	int type,
	int requestId,
	int contentLength,
	int paddingLength)
{
	FCGI_Header header;
	header.version = FCGI_VERSION_1;
	header.type = (unsigned char) type;
	header.requestIdB1 = (unsigned char) ((requestId >> 8)&0xff);
	header.requestIdB0 = (unsigned char) ((requestId     )&0xff);
	header.contentLengthB1 = (unsigned char) ((requestId>>8)&0xff);
	header.contentLengthB0 = (unsigned char)((requestId   )&0xff);
	header.paddingLength = (unsigned char)paddingLength;
	header.reserved = 0;
	return header;
}

FCGI_BeginRequestBody makeBeginRequstBody(int role)
{
	FCGI_BeginRequestBody body;
	body.roleB1 = (unsigned char)((role >>8)&0xff);
	body.roleB0 = (unsigned char)((role    )&0xff);
	body.flags = (unsigned char) 0 ;
	memset(body.reserved, 0, sizeof(body.reserved) );
	return body;
}
