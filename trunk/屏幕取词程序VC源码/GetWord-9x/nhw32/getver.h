#ifndef GETVERSION_H
#define GETVERSION_H

// predefined languange identifiers,
#define CHINESE_TAIWAN  0x0404  //Chinese (Taiwan)
#define CHINESE_PRC             0x0804  //Chinese (PRC)
#define ENGLISH_UNITED_STATES   0x0409  //English (United States)
#define PROCESS_DEFAULT 0x0400  //Process Default Language

//Internet Explorer Version Numbers.
#define IE_VERSION_FIRST        4       // IE major version
#define IE_VERSION_SECOND       70      // IE minor version


#ifdef __cplusplus
extern "C" 
{
#endif
/*
        IsDesiredIEVersion check the whether the version of 
        IE exefile specified by szIEExeName is 4.70.XXXX and
        the whether this version of IE support the language
        identified by dwLangId.

 Parameters

        szIEExeName
                .EXE file name of Internet Explorer 3.X

        dwLangId
                language identifier, if this dword is 0, languange
                support will not check.
 Return value
                If this is the desired IE version , return TRUE,
                otherwise, return FALSE, call GetLastError 
                to get extended error information. 
*/
BOOL IsDesiredFileVersion(LPTSTR szFileName, DWORD dwLangId);

#ifdef __cplusplus
}
#endif
#endif // GETVERSION_H
