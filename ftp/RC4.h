// RC4.h: interface for the CRC4 class.
//
//////////////////////////////////////////////////////////////////////
#ifndef _RC4_H_
#define _RC4_H_

typedef struct _tagRc4Key
{
    int x, y, m[256];
}RC4_KEY, *PRC4_KEY;

/*
 *    加解密字符串，特点\0结尾
 */
void RC4EncryptStr(char * v_szOutput, const char * v_szInput, int v_iBufLen, const char * v_szKey, int v_iKeyLen);
void RC4DecryptStr(char * v_szOutput, const char * v_szInput, int v_iBufLen, const char * v_szKey, int v_iKeyLen);
/*
 *    加解密内容
 */
void RC4EncryptContent(char *v_strBuf, int v_iBufLen, char* v_strPwd, int v_iPwdLen);
void RC4DecryptContent(char *v_strBuf, int v_iBufLen, char* v_strPwd, int v_iPwdLen);

void RC4(unsigned char *v_strBuf, int v_iBufLen, unsigned char* v_strPwd, const int v_iPwdLen);
void RC4Init(PRC4_KEY v_pRc4Key, unsigned char *v_strPwd,  const int v_iPwdLen);
void RC4_Section(unsigned char *v_strBuf, int v_iBufLen, unsigned char* v_strPwd, const int v_iPwdLen);


#endif 
