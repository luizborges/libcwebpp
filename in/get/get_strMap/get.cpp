/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../../../cweb.hpp"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////
struct cmp_str // use this to compare strings (C-style) in map function to find key map
{
	bool operator()(char const *a, char const *b) const
	{
		return std::strcmp(a, b) < 0;
   	}
};

class Get
{
 private:
	char *httpg = NULL; //  = getenv("QUERY_STRING"); | httpg = http get
	int  hglen = 0; // = strlen(getenv("QUERY_STRING")) | hglen = len of http get
	char *str = NULL;
	int  size = 0;
	std::map<const char*, const char*, cmp_str> _map;
 
 public:

	Get()
	{
 		size = 2048; // 2 kB is the default size of str to keep cookies
 		str = static_cast<char*>(malloc(2048*sizeof(char)));
 		if(str == NULL) {
			err("CWEB::IN - Allocated Space for Get - String Default.\n"
			"Tried to allocated %d bytes\nerro is %d\nstr erro is \"%s\"",
			size*sizeof(char), errno, strerror(errno));
		}
	}

	bool init(const long max_size = 0)
	{
		// verifica se é possível inicializar o get
		char *rm = getenv("REQUEST_METHOD");
		if(rm == NULL) {
			err("CWEB::IN - NO REQUEST_METHOD.\ngetenv(\"REQUEST_METHOD\") = NULL.");
			return false;
		}
		if(strcmp(rm, "GET") != 0) {
			err("CWEB::IN - REQUEST_METHOD IS NOT GET.\n"
			"getenv(\"REQUEST_METHOD\") = \"%s\"", rm);
			return false;
		}
	
		///////////////////////////////////////////////////////////////////
		// recebe o get string
		///////////////////////////////////////////////////////////////////
		httpg = getenv("QUERY_STRING"); // recupera a string 
		if(httpg == NULL) {
			err("CWEB::IN - HTTP REQUEST METHOD GET STRING is NULL.");
		}
		hglen = strlen(httpg);
		
		if(hglen > max_size && max_size > 0)
		{
			err("CWEB::IN - size of input data recieve by HTTP GET METHOD is bigger"
			"bigger than max size allowed for this type of input.\n"
			"strlen(getenv(\"QUERY_STRING\")) = %ld bytes\n"
			"Maximum size for HTTP GET METHOD = %ld bytes\n"
			"You can change this size.\nSee documentation for more details",
			httpg, max_size);
		}
		
		if(size <= hglen)
		{
			size = hglen +1; // for character `\0`
			free(str);
		
			str = static_cast<char*>(malloc(size*sizeof(char)));
			if(str == NULL) {
				err("CWEB::IN - Allocated Space for Cookie - String Default.\n"
				"Tried to allocated %d bytes\nerro is %d\nstr erro is \"%s\"",
				size*sizeof(char), errno, strerror(errno));
			}
		}
	
		///////////////////////////////////////////////////////////////////
		// insere a string no map
		///////////////////////////////////////////////////////////////////
		fill_map();
	
		return true; // foi possível inicializar o HTTP REQUEST METHOD GET no mapa de busca
	}
	
	const char*
	get(const char *key)
	{
		if(key != NULL)
		{
			auto it = _map.find(key);
			if (it != _map.end()) {
				return it->second;
			}
		}
		//PRINT_errING_IF_NO_KEY_IN_MAP:
		err("CWEB::IN - Fetch for a no key of HTTP REQUEST METHOD GET.\n"
		"fectch key = \"%s\"\nnumber of keys is %d\n"
		"List of all keys in HTTP REQUEST METHOD GET that be parsed:", key, _map.size());
		
		for(auto elem : _map)
		{
   			fprintf(stderr, "[\"%s\"] = \"%s\"\n", elem.first, elem.second);
		}
		
		return NULL;
	}
	
 private:

	void fill_map()
	{
		int posInitNextStr = 0;
		for(int i=0; i < hglen; ++i)
		{
			///////////////////////////////////////////////////////////////////
			// busca, decodifica chave do GET - um par de key e conteúdo
			///////////////////////////////////////////////////////////////////
			const char *key = &httpg[i];
			
			// descobre o tamanho de key
			int keyLen = 0;
			for(; i < hglen && key[keyLen] != '='; ++keyLen) {++i;} 
		
			char *keyDecode = &str[posInitNextStr];
			if(cweb::decode(keyDecode, key, keyLen) != true) { // decode the key
				err("CWEB::IN - Decoding key in HTTP REQUEST METHOD GET.\n"
				"key enconding: \"%s\"\nkey decode (wrong value): \"%s\"\nKey length: %d\n"
				"HTTP GET: \"%s\"\n", key, keyDecode, keyLen, httpg);
			}
			posInitNextStr += strlen(keyDecode) +2; // atualiza a nova posição inicial
		
			///////////////////////////////////////////////////////////////////
			// busca, decodifica contéudo do GET - um par de key e contéudo
			///////////////////////////////////////////////////////////////////
			const char *content = &httpg[++i]; // seta o início da string do content
			int contentLen = 0;
			for(; i < hglen && content[contentLen] != '&';
				++contentLen) { ++i; } // descobre o tamanho do content
		
		
			// recebe a posição no array de conteúdo própria para guardar o valor
			char *contentDecode = &str[posInitNextStr];
			// decode o conteúdo do par do get
			if(cweb::decode(contentDecode, content, contentLen) != true) {
				err("CWEB::IN - Decoding content in HTTP REQUEST METHOD GET.\n"
				"content enconding: \"%s\"\nkey decode (wrong value): \"%s\"\n"
				"content length: %d\nHTTP GET: \"%s\"\n",
				content, contentDecode, contentLen, httpg);
			}
			posInitNextStr += strlen(contentDecode) +2; // atualiza a nova posição inicial
		
			///////////////////////////////////////////////////////////////////
			// insere no map o key e o conteúdo - atualiza a posição na array do get
			///////////////////////////////////////////////////////////////////
			_map[keyDecode] = contentDecode;
		}
	}
};


////////////////////////////////////////////////////////////////////////////////
// Private Variable
////////////////////////////////////////////////////////////////////////////////
static Get _get;

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
/**
 * Insere a string do http request method get in a map format
 */
bool
cweb::in::init_get(const long max_size)
{
	return _get.init(max_size);
}

/**
 * 
 */
char*
cweb::in::get(const char *key)
{
	return const_cast<char*>(_get.get(key));
}














