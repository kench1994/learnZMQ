// RC4.cpp: implementation of the CRC4 class.
//
//////////////////////////////////////////////////////////////////////

#include "RC4.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void RC4Init(PRC4_KEY v_pRc4Key, unsigned char *v_strPwd,  const int v_iPwdLen)
{
    int i, j, k, *m, a;

    v_pRc4Key->x = 0;
    v_pRc4Key->y = 0;
    m = v_pRc4Key->m;

    for( i = 0; i < 256; i++ )
    {
        m[i] = i;
    }

    j = k = 0;

    for( i = 0; i < 256; i++ )
    {
        j = ( j + m[i] + v_strPwd[k] ) % 256;

        a = m[i];
        m[i] = m[j]; 
        m[j] = a;

        k = (k + 1) % v_iPwdLen;
    }
}

void RC4_Section(unsigned char *v_strBuf, int v_iBufLen, unsigned char* v_strPwd, const int v_iPwdLen)
{
    RC4_KEY        Rc4Key;
    int            i, x, y, *m, a, b;

    RC4Init( &Rc4Key, v_strPwd, v_iPwdLen );

    x = Rc4Key.x;
    y = Rc4Key.y;
    m = Rc4Key.m;

    for( i = 0; i < v_iBufLen; i++ )
    {
        x = (unsigned char) ( x + 1 ); a = m[x];
        y = (unsigned char) ( y + a );
        m[x] = b = m[y];
        m[y] = a;
        v_strBuf[i] ^= m[(unsigned char) ( a + b )];
    }

    Rc4Key.x = x;
    Rc4Key.y = y;
}

// ---------------------------------------------------
//
// RC4加密字符串，按 MAX_COMPRESS_BUF 分组
//
void RC4(unsigned char *v_strBuf, int v_iBufLen, unsigned char* v_strPwd, const int v_iPwdLen)
{ 
    if((NULL != v_strPwd) && (v_iPwdLen > 0))
    {
        int        iRcLen = v_iBufLen;
        int        iPos = 0;
        int        iBufLen = 0;

        while(iRcLen > 0)
        {
            iBufLen = (iRcLen > 512) ? 512: iRcLen; //!<每次加解密512个字节
            RC4_Section(v_strBuf + iPos, iBufLen, v_strPwd, v_iPwdLen);
            iRcLen -= iBufLen;
            iPos += iBufLen;
        }
    }
}

/*
 *    RC4加密字符串，属于定长加解密，进行16进制转换，加密完为2倍的长度，适应于短信息的加密传输
 *    特别注意对于加密后长度的计算，strlen遇到\0就返回，存在误差导致加解密结果不匹配
 *    最大支持1024字节的加密
 */
void RC4EncryptStr(char * v_szOutput, const char * v_szInput, int v_iBufLen, const char * v_szKey, int v_iKeyLen)
{
    if(v_iBufLen > 1024)
    {
        memcpy(v_szOutput, v_szInput, v_iBufLen);
        return;
    }
    
    char szInput[1025];
    memset(szInput, 0, 1025);
    memcpy(szInput, v_szInput, v_iBufLen);

    char szPwd[500];
    memset(szPwd, 0, 500);
    memcpy(szPwd, v_szKey, v_iKeyLen);

    RC4((unsigned char*)szInput, v_iBufLen, (unsigned char*)szPwd, strlen(szPwd));

    char szTmp[2050];
    memset(szTmp, 0 , 2050);

    //!<将加密结果转成16进制
      int i = 0;
    for (i = 0; i < v_iBufLen; ++i)
    {
        sprintf((char*)&szTmp[i*2], "%02x", (unsigned char)szInput[i]);
    }

    memcpy(v_szOutput, szTmp, strlen(szTmp));
}

/*
 *    RC4解密字符串，也只能支持到2096字节的加密内容解密，再长
 *    特别注意对于加密后长度的计算，strlen遇到\0就返回，存在误差导致加解密结果不匹配
 */
void RC4DecryptStr(char * v_szOutput, const char * v_szInput, int v_iBufLen, const char * v_szKey, int v_iKeyLen)
{
    char szTemp[2050];
    char szTemp1[3];
    memset(szTemp, 0x00, 2050);
    memset(szTemp1, 0x00, 3);

    //!<先进行16进制转换
      int i = 0;
    int iLen = v_iBufLen / 2;
    for (i = 0; i < iLen; ++i)
    {
        memcpy(szTemp1, &v_szInput[i*2], 2);
        szTemp[i] = (unsigned char)strtoul(szTemp1, NULL, 16);
    }

    //!<再解密
    char szPwd[500];
    memset(szPwd, 0, 500);
    memcpy(szPwd, v_szKey, v_iKeyLen);

    RC4((unsigned char*)szTemp, iLen, (unsigned char*)szPwd, v_iKeyLen);

    memcpy(v_szOutput, szTemp, strlen(szTemp));
}

void RC4EncryptContent(char *v_strBuf, int v_iBufLen, char* v_strPwd, int v_iPwdLen)
{
    RC4((unsigned char*)v_strBuf, v_iBufLen, (unsigned char*)v_strPwd, v_iPwdLen);
}

void RC4DecryptContent(char *v_strBuf, int v_iBufLen, char* v_strPwd, int v_iPwdLen)
{
    RC4((unsigned char*)v_strBuf, v_iBufLen, (unsigned char*)v_strPwd, v_iPwdLen);
}
