#pragma once
#include "../Base/platform.h"
#include <time.h>
#include <sys/time.h>

using namespace std;

inline int constrain(int x, int l, int h)
{
	int y;

	if (x < l)y = l;
	else y = x;

	if (y > h)return h;
	else return y;
}

inline double confineVal(double val, double max, double min)
{
	if (val>max)val = max;
	else if (val<min)val = min;

	return val;
}


inline uint64_t get_time_usec()
{
	struct timeval _time_stamp;
	gettimeofday(&_time_stamp, NULL);
	uint64_t time = (uint64_t)_time_stamp.tv_sec * (uint64_t)1000000 + _time_stamp.tv_usec;

	return time;
}

/*
inline int32_t confineVal(int32_t val, int32_t max, int32_t min)
{
	if(val>max)val=max;
	else if(val<min)val=min;

	return val;
}

inline int32_t makeLong(char* buf)
{
	D_WORD num;
	num.bytes[0] = buf[0];
	num.bytes[1] = buf[1];
	num.bytes[2] = buf[2];
	num.bytes[3] = buf[3];
	return num.val;
}

inline double makeDouble(char* buf)
{
	Q_WORD num;
	num.bytes[0] = buf[0];
	num.bytes[1] = buf[1];
	num.bytes[2] = buf[2];
	num.bytes[3] = buf[3];
	num.bytes[4] = buf[4];
	num.bytes[5] = buf[5];
	num.bytes[6] = buf[6];
	num.bytes[7] = buf[7];
	return num.val;
}

inline void copyByte(char* pBuf, D_WORD val)
{
	pBuf[0] = val.bytes[0];
	pBuf[1] = val.bytes[1];
	pBuf[2] = val.bytes[2];
	pBuf[3] = val.bytes[3];
}

inline void copyByte(char* pBuf, Q_WORD val)
{
	pBuf[0] = val.bytes[0];
	pBuf[1] = val.bytes[1];
	pBuf[2] = val.bytes[2];
	pBuf[3] = val.bytes[3];
	pBuf[4] = val.bytes[4];
	pBuf[5] = val.bytes[5];
	pBuf[6] = val.bytes[6];
	pBuf[7] = val.bytes[7];
}

inline void copyByte(char* pBuf, int32_t num)
{
	D_WORD val;
	val.val = num;
	pBuf[0] = val.bytes[0];
	pBuf[1] = val.bytes[1];
	pBuf[2] = val.bytes[2];
	pBuf[3] = val.bytes[3];
}

inline void copyByte(char* pBuf, double num)
{
	Q_WORD val;
	val.val = num;
	pBuf[0] = val.bytes[0];
	pBuf[1] = val.bytes[1];
	pBuf[2] = val.bytes[2];
	pBuf[3] = val.bytes[3];
	pBuf[4] = val.bytes[4];
	pBuf[5] = val.bytes[5];
	pBuf[6] = val.bytes[6];
	pBuf[7] = val.bytes[7];
}
*/
inline char* ftoa( char* pString, size_t Size, float Value, int FracDigits )
{
    if( pString == NULL )return NULL;

    char* pSource = pString;
    if( *(int32_t*)&Value < 0 )
    {
        if( (size_t)( pString - pSource ) >= Size )return NULL;
        *pString++ = '-';
        *(int32_t*)&Value ^= 0x80000000;
    }

    char* pBegin  = pString;
    int   Integer = (int)Value;

    for( int i = 10; i <= Integer; i *= 10, pString++ );

    if( (size_t)( pString - pSource ) >= Size )return NULL;

    char* pCode = pString++;
    *pCode-- = '0' + (char)( Integer % 10 );
    for( int i = Integer / 10; i != 0; i /= 10 )
        *pCode-- = '0' + (char)( i % 10 );
    if( FracDigits > 0 )
    {
        if( (size_t)( pString - pSource ) >= Size )return NULL;
        *pString++ = '.';
        while( FracDigits-- > 1 )
        {
            Value -= Integer;
            Integer = (int)( Value *= 10.0f );
            if( (size_t)( pString - pSource ) >= Size )return NULL;
            *pString++ = '0' + (char)Integer;
        }
        Integer = (int)( ( Value - Integer ) * 10.0f + 0.5f );
        if( Integer < 10 )
        {
            if( (size_t)( pString - pSource ) >= Size )return NULL;
            *pString++ = '0' + (char)Integer;
        }
        else
        {
            pCode = pString - 1;
            if( (size_t)( pString - pSource ) >= Size )return NULL;
            *pString++ = '0';
            do
            {
                if( *pCode == '.' )
                    pCode--;
                if( *pCode != '9' )
                {
                    *pCode += 1;
                    break;
                }
                else
                    *pCode = '0';
                if( pCode == pBegin )
                {
                    if( (size_t)( pString - pSource ) >= Size )return NULL;
                    char* pEnd = pString++;
                    while( pEnd-- != pCode )
                        *( pEnd + 1 ) = *pEnd;
                    *pCode = '1';
                }
            } while( *pCode-- == '0' );
        }
    }
    if( (size_t)( pString - pSource ) >= Size )return NULL;
    *pString = '\0';
    return pSource;
}


inline string f2str(double val)
{
	char buf[128];
	return string(ftoa(buf,sizeof(buf),val,10));
}

inline string i2str(int val)
{
	char buf[128];
	sprintf(buf, "%d", val);
	return string(buf);
}

inline void trimJson(string* pStr)
{
	unsigned int idx = pStr->rfind('}',pStr->size())+1;
	if(pStr->size()>idx)
	{
		pStr->erase(idx);
	}

	idx = pStr->find('{',0);
	if(idx!=string::npos && idx>0)
	{
		pStr->erase(0,idx-1);
	}
}

