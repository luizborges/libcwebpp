/**
 * @desc: implementa a leitura do post "enctype='multipart/form-data'"
 */
class Token
{private:
	char *in = NULL;
	long in_size = 0;
	long in_idx = 0;
	char *delimitators = NULL;
	char *boundary = NULL;
	int  boundary_size = 0;
 public:
	void init(
			char *in,
			const long in_size,
			const char *delimitators,
			const char *boundary,
			const int boundary_size)
	{
		this->in = in;
		this->in_size = in_size;
		this->delimitators = const_cast<char*>(delimitators);
		this->boundary = const_cast<char*>(boundary);
		this->boundary_size = boundary_size;
	}
		
	char* next() // give the next token
	{
		///////////////////////////////////////////////////////////////////
		// descobre o início do token
		///////////////////////////////////////////////////////////////////
		for(; in_idx < in_size; ++in_idx)
		{
			if(&in[in_idx] == NULL) {
				err("CWEB::IN - NO Token init valid. token is NULL.\n"
				"Post type is \"multipart/form-data\"\n"
				"Input is \"%s\"\nIndex NULL is %d", in, in_idx);
			}
			
			if(in[in_idx] == '\n')
			{
				in[in_idx] = '\0';
				return NULL;
			}
			
			if(in[in_idx] == '\0') {
				return NULL;
			}
			
			for(int i=0; i < static_cast<int>(strlen(delimitators)); ++i)
			{
				if(in[in_idx] == delimitators[i])
				{
					goto _continue;
				}
			}
			
			break; // sai do loop - significa que encontrou algum character válido
			// continuar a busca já que não encontrou nenhum character válido
			_continue:;
		}
		
		if(in_idx >= in_size) { // verifica se chegou ao final
			return NULL;
		}
		int token_init = in_idx;
		
		///////////////////////////////////////////////////////////////////
		// descobre o fim do token
		///////////////////////////////////////////////////////////////////
		for(; in_idx < in_size; ++in_idx)
		{
			if(&in[in_idx] == NULL) {
				err("CWEB::IN - NO Token end valid. token is NULL.\n"
				"Post type is \"multipart/form-data\"\n"
				"Input is \"%s\"\nIndex NULL is %d", in, in_idx);
			}
			
			if(in[in_idx] == '\n')
			{
				in[in_idx] = '\0';
				return &in[token_init];
			}
			
			if(in[in_idx] == '\0')
			{
				return &in[token_init];
			}
			
			for(int i=0; i < static_cast<int>(strlen(delimitators)); ++i)
			{
				if(in[in_idx] == delimitators[i])
				{
					in[in_idx] = '\0';
					++in_idx;
					return &in[token_init];
				}
			}
		}
		// se não encontrou um character válido para terminar a string
		if(in_idx >= in_size) { // verifica se chegou ao final
			in[in_size -1] = '\0';
			return &in[token_init];
		}
		
		err("CWEB::IN - Token end invalid.\n"
		"Post type is \"multipart/form-data\"\n"
		"Input is \"%s\"\nIndex NULL is %d", in, in_idx);
		return NULL;
	}
	
	// considera tanto o character '\n' como o '\0' como nova linha
	bool end_line()
	{
		if(in[in_idx] == '\0') {
			++in_idx;
			return true;
		}
		
		if(in[in_idx] == '\n') {
			++in_idx;
			return true;
		}
		
		return false;
	}
	
	// considera tanto o character '\n' como o '\0' como nova linha
	bool next_line(long* _in_idx = NULL)
	{
		for(; in_idx < in_size; ++in_idx)
		{
			if(&in[in_idx] == NULL) {
				continue;
			}
			
			if(in[in_idx] == '\0')
			{
				++in_idx;
				if(_in_idx != NULL) {*_in_idx = in_idx;}
				if(in_idx == in_size-1) {return false;}
				return true;
			}
			
			if(in[in_idx] == '\n')
			{
				++in_idx;
				if(_in_idx != NULL) {*_in_idx = in_idx;}
				if(in_idx == in_size-1) {return false;}
				return true;
			}
		}
		
		if(_in_idx != NULL) {*_in_idx = in_idx;}
		return false;
	}
	
	char* file(long& size)
	{
		long fileInit = in_idx;
		int ib = 0;
		for(; in_idx < in_size; ++in_idx)
		{
			if(&in[in_idx] == NULL) {
				ib = 0;
				continue;
			}
			
			if(in[in_idx] == boundary[ib])
			{
				++ib; // pula para a próxima posição
				if(ib == boundary_size) break;
			} else {
				ib = 0;
			}
		}
		
		if(in_idx >= in_size) {
			err("CWEB::IN - Token end invalid.\n"
		"Post type is \"multipart/form-data\"\n"
		"Input is \"%s\"\nIndex NULL is %d\npost size is %ld", in, in_idx, in_size);
		}
		
		size = in_idx -boundary_size -fileInit -1;
		if(size == 0) return NULL;
		in[fileInit +size] = '\0'; // primeiro character do boundary
		return &in[fileInit];
	}
};


class FileImpl: public cweb::file
{public:
	char* _name = NULL;
	long  _size = 0;
	char* _data = NULL;
	char  _type = 0;
	char* _typeStr = NULL;

	char* name() { return _name; }
	long  size() { return _size; }
	char* data() { return _data; }
	char  type() { return _type; }
	char* typeStr() {return _typeStr; }
};
// usado para retorno caso não exista a chave, todos os valores são NULL ou 0
static FileImpl _fiEmpty;

class Mult
{private:
	struct cmp_str // use this to compare strings (C-style) in map function to find key map
	{
		bool operator()(char const *a, char const *b) const
		{
			return std::strcmp(a, b) < 0;
   		}
	};
	std::map<const char*, cweb::file*, cmp_str> _map;
	char *boundary = NULL;
	int boundary_size = 0;
	char *_stdin = NULL;
	long stdin_size = 0;
	Token token;
 public:
	bool init(const long max_size)
 	{
		///////////////////////////////////////////////////////////////////
		// verifica o tamanho do post, verfica o tamanho dos buffers
		///////////////////////////////////////////////////////////////////
		char *strctlen = getenv("CONTENT_LENGTH");
		if(strctlen == NULL) {
			err("CWEB::IN - NO CONTENT_LENGTH\ngetenv(\"CONTENT_LENGTH\") = NULL\n"
			"Post type is \"multipart/form-data\"");
		}
		stdin_size = strtol(strctlen, NULL, 0);
		if(stdin_size == 0L) {
			err("CWEB::IN - strtol()\nstrtol() = 0L\ngetenv(\"CONTENT_LENGTH\") = \"%s\"\n"
			"POST type is \"multipart/form-data\"\nerro is %d\nstr erro is \"%s\"",
			strctlen, errno, strerror(errno));
		}
		if(stdin_size < 0L) {
			err("CWEB::IN - CONTENT_LENGTH is lesser than 1\nCONTENT_LENGTH = %ld\n"
			"getenv(\"CONTENT_LENGTH\") = \"%s\"\nPOST type is \"multipart/form-data\"",
			stdin_size, strctlen);
		}
		
		if(stdin_size > max_size && max_size > 0)
		{
			err("CWEB::IN - size of input data recieve by HTTP POST METHOD is bigger"
			"bigger than max size allowed for this type of input.\n"
			"getenv(\"CONTENT_LENGTH\") = \"%s\"\nCONTENT_LENGTH = %ld bytes\n"
			"Maximum size for HTTP POST METHOD -\"multipart/form-data\" "
			"= %ld bytes\nYou can change this size.\nSee documentation for more details",
			strctlen, stdin_size, max_size);
		}
		
		if(_stdin != NULL) {
			delete[] _stdin;
		}
		_stdin = new char[stdin_size +1];
		
		size_t read = fread(_stdin, sizeof(char), stdin_size, stdin);
		if(read != static_cast<size_t>(stdin_size)) {
			err("CWEB::IN - Reading post content and putting it into postStr.\n"
			"Read in post content: %ld\nCONTENT_LENGTH string: \"%s\"\nCONTENT_LENGTH: %ld\n"
			"POST type is \"multipart/form-data\"\nerro is %d\nstr erro is \"%s\"",
			read, strctlen, stdin_size, errno, strerror(errno));
		}
		_stdin[stdin_size] = '\0';
		
		//err("stdin is \"%s\"", _stdin);
		init_boundary();
		//err("boundary is \"%s\"", boundary);
		fill_map();
		return true; // foi possível inicializar o HTTP REQUEST METHOD POST no mapa de busca
	}
 
	const char* type()
	{
		return "multipart/form-data";
	}
	
	cweb::file& fpost(const char *key)
	{
		if(key != NULL)
		{
			auto it = _map.find(key);
			if (it != _map.end()) {
				return *it->second;
			}
		}
		//PRINT_errING_IF_NO_KEY_IN_MAP:
		err("CWEB::IN - Fetch for a no key of HTTP REQUEST METHOD POST.\n"
		"type of the post is \"%s\"\nfectch key = \"%s\"\nnumber of keys is %d\n"
		"List of all keys in HTTP REQUEST METHOD POST that be parsed:",
		type(), key, _map.size());
		
		for(auto elem : _map)
		{
   			fprintf(stderr, "[\"%s\"] = file name \"%s\"\n",
   				elem.first, elem.second->name());
		}
		
		return _fiEmpty;
	}

 private:
	void init_boundary()
	{/*TRACE_FUNC*/
		// não precisa de verificar, se é NULL pois já foi verificado antes
		char *ct = getenv("CONTENT_TYPE");
		ct = strstr(ct, "boundary=");
		if(ct == NULL) {
			err("CWEB::IN - Reading HTTP POST CONTENT_TYPE.\n"
			"POST type is \"multipart/form-data\"\n"
			"getenv(\"CONTENT_TYPE\") has no \"boundary=\" parameter.\n"
			"getenv(\"CONTENT_TYPE\") is \"%s\"", getenv("CONTENT_TYPE"));
		}
		
		char *_boundary = &ct[9]; // set o boundary no character imediatamente após o '='.
		ct = &ct[9];
		char *end_boundary = strchr(ct, ';');
		if(end_boundary != NULL) { // para o caso de haver mais de um atributo na variável
			boundary_size = static_cast<int>(end_boundary - _boundary);
		} else {
			boundary_size = strlen(_boundary);
		}
		
		boundary_size += 3;  // "--" + '\0'
		boundary = new char[boundary_size];
		if(boundary == NULL) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"\n"
			"Allocate memory for boundary.\nBoundary size is %d", boundary_size);
		}
		
		// monta o boundary - padrão do http - testado para o firefox e chrome
		// os 2 primeiros characteres tem que ser "--" > para checar com o valor do 
		// do boundary fornecido pela "getenv("CONTENT_TYPE")". o Boundary desta função
		// é sempre 2 characteres ("--") menor que os boundary do stdin, e sempre são os
		// 2 primeiros characteres do boundary do stdin que está a diferença.
		// os dois character finais são '\r\n' - se for o fim do stdin os characteres
		// finais serão "--\r" | TODO - conferir se '\n' entra
		sprintf(boundary, "--%s", _boundary);
	}
	
	void fill_map()
	{/*TRACE_FUNC*/
		parser_header_stdin(); // get the header
		token.init(&_stdin[boundary_size-1], stdin_size-boundary_size+1,
			"\r\n =;:\"",boundary, boundary_size-1);

		err("stdin_size = %ld\nstdin=\"%s\"", stdin_size, _stdin);
		token.next_line();
		do {
			FileImpl* _fi = new FileImpl();
			if(_fi == NULL) {
				err("CWEB::IN - HTTP POST type is \"multipart/form-data\"");
			}
			FileImpl& fi = *_fi;
			parser_header(fi);
			parser_attribute(fi);
			fi._data = token.file(fi._size);
			
			long idx = -1;
			bool b = token.next_line(&idx);
			err("fi.data()=\"%s\"", fi._data);
			err("idx=%ld | idxS=%ld| stdin_size=%ld | b=%s",
			 idx,idx+boundary_size-1, stdin_size, b?"true":"false");
			if(idx+boundary_size-1 >= stdin_size) break;
		} while(true);
	}
	
	void parser_header_stdin()
	{/*TRACE_FUNC*/
		if(stdin_size <= boundary_size) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"\n"
			"Size of HTTP POST is lesser than boundary size.\n"
			"Header of stdin is wrong.\nExpected boundary value.\nBoundary = \"%s\"\n"
			"Boundary size is %d\nstdin size is %ld\nstdin is \"%s\"",
			boundary, boundary_size, stdin_size, _stdin);
		}
		
		if(strncmp(_stdin, boundary, boundary_size-1) != 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"\n"
			"Header of stdin is wrong.\nExpected boundary value.\nBoundary = \"%s\"\n"
			"Boundary size is %d\nstdin is \"%s\"",&boundary[2], boundary_size, _stdin);
		}
		
		//check if the stdin is empty
		if(strncmp(&_stdin[boundary_size-1], "--\r\n", 4) == 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"\n"
			"Body of HTTP POST is empty");
			return;
		}
		
		if(strncmp(&_stdin[boundary_size-1], "\r\n", 2) != 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"\n"
			"Header of stdin is wrong.\nExpected boundary + \"\\r\\n\".\nBoundary = \"%s\"\n"
			"Boundary size is %d\nstdin is \"%s\"", boundary, boundary_size, _stdin);
		}
	}
	
	void parser_header(FileImpl& fi)
	{/*TRACE_FUNC*/
		char *str = token.next();
		if(strcmp(str, "Content-Disposition") != 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"\n"
			"Expected: \"%s\"\nFound: \"%s\"", "Content-Disposition", str);
		}
		
		str = token.next();
		if(strcmp(str, "form-data") != 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\""
			"Expected: \"%s\"\nFound: \"%s\"", "form-data", str);
		}
		
		str = token.next();
		if(strcmp(str, "name") != 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\""
			"Expected: \"%s\"\nFound: \"%s\"", "name", str);
		}
		
		char*& key = str; // faz um alias -compilador otimiza isso - apenas para legibilidade
		key = token.next(); // nome do key do map
		
		auto it = _map.find(key);
		if (it != _map.end()) { // verifica se a chave já foi inserida no map
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\"");
		}
		
		err("key=\"%s\"",key);
		_map[key] = &fi; // insere o arquivo no map
		
		str = token.next();
		if(str == NULL) { // fim da linha
			return;
		}
		if(strcmp(str, "filename") != 0) {
			err("CWEB::IN - HTTP POST type is \"multipart/form-data\""
			"Expected: \"%s\"\nFound: \"%s\"", "filename", str);
		}
		
		fi._name = token.next();
		
		token.next_line();
	}
	
	void parser_attribute(FileImpl& fi)
	{/*TRACE_FUNC*/
		char *str = token.next();
		if(str == NULL) {
			fi._type = 1;
			fi._typeStr = new char[6];
			if(fi._typeStr == NULL) {
				err("CWEB::IN - HTTP POST type is \"multipart/form-data\"");
			}
			sprintf(fi._typeStr, "char*");
			token.next_line();
			token.next_line();
			return;
		}
		if(strcmp(str, "Content-Type") != 0) {
			fi._type = 3;
			fi._typeStr = new char[13];
			if(fi._typeStr == NULL) {
				err("CWEB::IN - HTTP POST type is \"multipart/form-data\"");
			}
			sprintf(fi._typeStr, "unknown type");
		}
		
		str = token.next();
		if(strcmp(str, "application/octet-stream") == 0) {
			fi._type = 2;
			fi._typeStr = new char[25];
			if(fi._typeStr == NULL) {
				err("CWEB::IN - HTTP POST type is \"multipart/form-data\"");
			}
			sprintf(fi._typeStr, "application/octet-stream");
		}
		
		token.next_line();
		// se possui arquivo o arquivo está sempre marcado por 1 linha em branco
		token.next_line();
	}
};


////////////////////////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////////////////////////
static Mult _mult;
////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
bool cweb::in::
init_fpost(const long max_size)
{
	return _mult.init(max_size);
}

cweb::file& cweb::in::
fpost(const char *key)
{
	return _mult.fpost(key);
}

















