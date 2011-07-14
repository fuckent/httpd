#ifndef FCGI_H
#define FCGI_H      1

#include "inc.h"

typedef struct
{
    unsigned char       version;
    unsigned char       type;
    unsigned short int  requestId;
    unsigned short int  contentlength;
    unsigned char       paddingLength;  
    unsigned char       reserved;
    unsigned char       ContentData;
} FCGI_Record_t;

typedef struct {
    unsigned char   nameLength;  /* nameLengthB0  >> 7 == 0 */
    unsigned char   valueLength; /* valueLengthB0 >> 7 == 0 */
    unsigned char*  nameData;
    unsigned char*  valueData;
} FCGI_NameValuePair11_t;

typedef struct {
    unsigned char   nameLength;  /* nameLengthB0  >> 7 == 0 */
    unsigned int    valueLength;
    unsigned char*  nameData;
    unsigned char* valueData;
} FCGI_NameValuePair14_t;

typedef struct {
    unsigned int    nameLength;
    unsigned char   valueLength;
    unsigned char*  nameData;
    unsigned char*  valueData;
} FCGI_NameValuePair41_t;

typedef struct {
    unsigned int    nameLength;
    unsigned int    valueLength;
    unsigned char*  nameData;
    unsigned char*  valueData;
} FCGI_NameValuePair44_t;

typedef struct {
    unsigned int    appStatus;
    unsigned char protocolStatus;
    unsigned char reserved[3];
} FCGI_EndRequestBody_t;

typedef struct {
    unsigned short int  role;
    unsigned char       flags;
    unsigned char       reserved[5];
} FCGI_BeginRequestBody_t;

/*
 * Number of bytes in a FCGI_Header.  Future versions of the protocol
 * will not reduce this number.
 */
#define FCGI_HEADER_LEN  8

/*
 * Value for version component of FCGI_Header
 */
#define FCGI_VERSION_1           1

/*
 * Values for type component of FCGI_Header
 */
#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

/*
 * Value for requestId component of FCGI_Header
 */
#define FCGI_NULL_REQUEST_ID     0
/*
 * Mask for flags component of FCGI_BeginRequestBody
 */
#define FCGI_KEEP_CONN  1

/*
 * Values for role component of FCGI_BeginRequestBody
 */
#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3


/*
 * Values for protocolStatus component of FCGI_EndRequestBody
 */
#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

/*
 * Variable names for FCGI_GET_VALUES / FCGI_GET_VALUES_RESULT records
 */
#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"
#define FCGI_MAX_REQS   "FCGI_MAX_REQS"
#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"


#endif
