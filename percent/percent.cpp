/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../cweb.hpp"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Private Variables
////////////////////////////////////////////////////////////////////////////////

								
////////////////////////////////////////////////////////////////////////////////
// Private Functions Head
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Private Functions Inline
////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Construct Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
bool
cweb::decode(char* out,
			 const char* in,
			 const int maxDecode)
{
    static const char tbl[256] = {
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7,  8, 9,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,10,11,12,13,14,15,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1, -1,-1,-1,-1,-1,-1,-1,-1
    };
    char c, v1, v2, *beg=out;
    int numDecode = 0;
    if(in != NULL) {
//        while(((c=*in++) != '\0') && numDecode < maxDecode) {
		while(numDecode < maxDecode) {
			c=*in++;
            if(c == '%') {
                if((v1=tbl[(unsigned char)*in++])<0 || 
                   (v2=tbl[(unsigned char)*in++])<0) {
                    *beg = '\0';
                    return false;
                }
                c = (v1<<4)|v2;
                numDecode += 2;
            } else if(c == '+') {
            	*out++ = ' ';
            	++numDecode;
            	continue;
            }
            *out++ = c;
            ++numDecode;
        }
    } else {
    	return false;
    }
    *out = '\0';
    return true;
}

char*
cweb::encode(const char* in,
			 const int maxEncode)
{
	if(in == NULL)    return NULL;
	if(maxEncode < 1) return NULL;
	
	FILE *tmp = tmpfile();
	if(tmp == NULL) {
		MError("open temporary file error.\nerro is %d\nstr erro is \"%s\"",
    		errno, strerror(errno));
		return NULL;
	}
	
	for(int i=0; i < maxEncode; ++i)
	{
		int c = in[i];
		if(c == '\0') break;
		
		if(c == ' ') { fprintf(tmp, "+"); }
		else if (c<33 || c>126 ||
			strchr("!\"%'();:@&=+$,/?#[]{}^\\|<>~`",c)) {
            fprintf(tmp, "%%%02X",c & 0xff);
        }
        else {
            fprintf(tmp, "%c", c);
        }
	}
	
	rewind(tmp);
	char *out = util::file::toStr(tmp); // esta função já fecha o arquivo
	
	return out;
}




