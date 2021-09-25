/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../../cweb.hpp"

/*
#define TRACE_FUNC \
fprintf(stderr, "\n*****************************************\n");\
fprintf(stderr, "TRACE FUNC:: \"%s\" (%d, \"%s\")\n",\
__PRETTY_FUNCTION__, __LINE__, __FILE__);\
fprintf(stderr, "*****************************************\n");
*/
////////////////////////////////////////////////////////////////////////////////
// função para criação de dependência circular
////////////////////////////////////////////////////////////////////////////////
void call_child(const char *name);

////////////////////////////////////////////////////////////////////////////////
// Private Structs
////////////////////////////////////////////////////////////////////////////////
struct Content // use this to keep the array/variale that will be write in session file
{
	char *name = NULL;
	bool hasPrint = false;
	bool hasValue = false; // to control delete[] value
	char *data = NULL;
	char *tag = NULL;
	
	////////////////////////////////////////////////////////////////////////////////
	// functions
	////////////////////////////////////////////////////////////////////////////////
	Content(const char *name)
	{/*TRACE_FUNC*/
		init(name);
	}
	
	Content(const char *name,
			const char *data)
	{/*TRACE_FUNC*/
		init(name);
		
		if(data == NULL) {
			throw err("WEB::OUT - Data of output is NULL\nContent name is \"%s\"", name);
		}
		
		int len = strlen(data);
		this->data = new char[len +1];
		if(this->data == NULL) {
			throw err("CWEB::OUT - In allocation memory for output value.\n"
			"Name of content is \"%s\"\ncontent size is %d bytes\nContent data is \"%s\"\n"
			"erro is %d\nstr erro is \"%s\"", name, len, data, errno, strerror(errno));
		}
		strcpy(this->data, data);
	}
	
	virtual ~Content()
	{
		delete[] name;
		delete[] data;
	}
	void print()
	{/*TRACE_FUNC*/
		// para evitar chamadas recursivas e loops infinitos.
		// garante que cada conteúdo somente será impresso uma única vez
		if(hasPrint == true) return;
		hasPrint = true;
		
		init_data(); // unifica o tratamento
		
		while(strlen(data) > 0)
		{
			/////////////////////////////////////////////////////////////////
			// e realiza a busca pela tag
			/////////////////////////////////////////////////////////////////
			tag = strstr(data, "<?cweb"); // realiza a busca pela tag
			if(tag == NULL) { // termina o loop pois não há tag
				fprintf(stdout, "%s", data); // imprime a string
				break;
			}
			
			/////////////////////////////////////////////////////////////////
			// verifica se não é comentário e se não for, chama o parser da tag
			/////////////////////////////////////////////////////////////////
			if(tag[6] == '@') // verifica se é o character de escape
			{	// o character de escape é sempre omitido
				tag[6] = '\0'; // finaliza a string que será impressa
				fprintf(stdout, "%s", data);
				data = &tag[7];// atualiza o ponteiro, para continuar a verificação da string
			} else {
				/**
				 * Está função realiza as operações:
				 * 1. imprime 'data' até o começo da tag "<?cweb" - esta não impressa
				 * 2. verifica se a tag chamada está correta e dentro do padrão
				 * 3. chama a tag filho e imprime ela (realiza todo o parser dela.
				 * 4. atualiza o ponteiro 'data', para o final da tag "?>"
				 */
				
				print_tag();
			}
		}
	}
	
	virtual void init_data(){}
	
	void print_tag()
	{/*TRACE_FUNC*/
		
		/////////////////////////////////////////////////////////////////
		// check the header of tag "<?cweb #in "" or <?cweb #add "
		/////////////////////////////////////////////////////////////////
		if(strncmp(tag, "<?cweb #in \"", 12) != 0 &&
			strncmp(tag, "<?cweb #add \"", 13) != 0)
		{
			char *endLine = strchr(tag, '\n'); // descobre o fim da linha onde ocorreu o erro
			if(endLine != NULL) {
				endLine[0] = '\0'; // posiciona um novo fim na string para impressão do erro.
			}
		
			throw err("WEB::OUT - Tag <?cweb is incorrect.\n"
			"Tag found (until the end of line): \"%s\"\n"
			"Expected one of these sequences: \"<?cweb #in \"\" (with \' \') or"
			" \"<?cweb\n#add \"\" (with \' \') \nfor more details see documentation.\n"
			"Client Output Name: \"%s\"\nType of output is %s", tag, name, type());
		}
	
		bool isTagADD = false;
		if(strncmp(tag, "<?cweb #add \"", 12) == 0) {
			isTagADD = true;
		}
		/////////////////////////////////////////////////////////////////
		// imprime a string até o ponto do começo da tag.
		/////////////////////////////////////////////////////////////////
		tag[0] = '\0'; // finaliza a string passada
		// sempre existe essa posição, pois o nome da tag já foi conferido acima
		tag = &tag[1]; // atualiza a posição da tag, para a dentro da tag.
		fprintf(stdout, "%s", data); // imprime até tag[0]
		
		/////////////////////////////////////////////////////////////////
		// get the name in the include
		/////////////////////////////////////////////////////////////////
		// sempre terá o character '"' - devido a verificação anterior
		char *tagName = strchr(tag, '\"');
		// atualiza o nome para a próxima possição do arranjo - 
		// sempre haverá tal posição - garantida pelo character '\0'
		tagName = &tagName[1]; 
		char *nameEnd = strchr(tagName, '\"'); // descobre o fim de name
		if(nameEnd == NULL) { // trata o caso de ter ocorrido um erro
			// descobre o fim da linha onde ocorreu o erro
			char *endLine = strchr(tagName, '\n');
			if(endLine != NULL) {
				endLine[0] = '\0'; // posiciona um novo fim na string para impressão do erro.
			}
			throw err("WEB::OUT - Tag <?cweb is incorrect.\n"
			"NAME has always be terminated with character \'\"\'.\nNAME given is = \"%s\".\n"
			"tag struct is \"<?cweb #in \"NAME\" ?>\" or \"<?cweb #add \"NAME\" ?>\".\n"
			"for more details see documentation.\nClient Output Name: \"%s\"\n"
			"Type of output is %s", tagName, name, type());
		}
		nameEnd[0] = '\0'; // seta o fim de name, onde era o character '"'
	
	
		if(isTagADD == true) // run tag <?cweb #add
		{
			// trata se for a tag <cweb #add \"
			/**
			 * para fins de deixar o código mais simples e harmonico.
			 * Ele le o nome do arquivo e adiciona o mesmo como sendo uma tag #in
			 * Em seguida manda executar a tag.
			*/
			char *tagAddName = get_new_name_tag_add();
			cweb::out::file(tagAddName, tagName);
			call_child(tagAddName); // run child 
			delete[] tagAddName; // free memory
		} else { // run tag <?cweb #in
			call_child(tagName); // run child
		}
		
		/////////////////////////////////////////////////////////////////
		// treat the end of tag after NAME
		/////////////////////////////////////////////////////////////////
		char *tagEnd = nameEnd; // descobre o começo depois do fim do nome
		tagEnd = &tagEnd[1]; // seta o primeiro character depois de '"' - após o fim do nome
	
		if( strncmp(tagEnd, " ?>",    3) != 0)
		{
			// descobre o fim da linha onde ocorreu o erro
			char *endLine = strchr(tagEnd, '\n'); 
			if(endLine != NULL) {
				endLine[0] = '\0'; // posiciona um novo fim na string para impressão do erro.
			}
		
			throw err("WEB::OUT - Tag <?cweb is incorrect.\n"
			"End of Tag found (until the end of line): \"%s\"\n"
			"Expected one of these sequences: \" ?>\" (with \' \')\n"
			"for more details see documentation.\nClient Output Name: \"%s\"\n"
			"tag value is \"%s\"\nType of output is %s",
			tagEnd, name, tagName, type());
		}
	
		/////////////////////////////////////////////////////////////////
		// treat the end of tag after NAME
		/////////////////////////////////////////////////////////////////
		data = strchr(tagEnd, '>');
		data = &data[1]; // passa para o próximo character
	}
	
	virtual char* get_new_name_tag_add()
	{/*TRACE_FUNC*/
		throw err("WEB::OUT - This class cannot call this function.\nError in inheritance.\n"
		"This function must only be called by class \"ContentFileName\"");
		return NULL;
	}
	
	virtual char* type() {/*TRACE_FUNC*/
		return const_cast<char*>("string (char*)");
	}
	
	void init(const char *name)
	{/*TRACE_FUNC*/
		check(name);
		
		int len = strlen(name);
		this->name = new char[len +1];
		if(this->name == NULL) {
			throw err("CWEB::OUT - In allocation memory for output name.\n"
			"name size is %d bytes\nName is \"%s\"\nerro is %d\nstr erro is \"%s\"",
			len, name, errno, strerror(errno));
		}
		strcpy(this->name, name);
	}
	
	void check(const char *name)
	{/*TRACE_FUNC*/
		// check name 
		if(name == NULL) {
			throw err("WEB::OUT - Name of output is NULL");
		}
		
		if(strlen(name) < 1) {
			throw err("WEB::OUT - Name of output can not be a empty string");
		}
		
		if(Has_Only_Alpha_Num(name) == false) {
			throw err("WEB::SESSION - Not a valid output name.\n"
			"all character of name must be a letter (A to Z or a to z) or a digit (0 to 9) "
			"or special character ('_' and '-').\nname: \"%s\"", name);
		}
	}
	
	bool Has_Only_Alpha_Num(const char *str)
	{/*TRACE_FUNC*/
		for(int i=0; i < static_cast<int>(strlen(str)); ++i) {
			if(isalnum(str[i]) == false &&
				str[i] != '_' &&
				str[i] != '-')
			{
				return false;
			}
		}
	
		return true;
	}
};

struct ContentFile: public Content
{
	FILE *file;
	////////////////////////////////////////////////////////////////////////////////
	// functions
	////////////////////////////////////////////////////////////////////////////////
	ContentFile(
		const char *name,
		FILE *file) : Content(name)
	{/*TRACE_FUNC*/
		if(file == NULL) {
			throw err("WEB::OUT - File output is NULL\nContent name is \"%s\"", name);
		}
		
		this->file = file;
	}
	
	char* type() {/*TRACE_FUNC*/
		return const_cast<char*>("file");
	}
	
	void init_data()
	{/*TRACE_FUNC*/
		data = u::file::toStr(file);
	}
};

struct ContentFileName: public Content
{
	char *fname; // file name
	////////////////////////////////////////////////////////////////////////////////
	// functions
	////////////////////////////////////////////////////////////////////////////////
	ContentFileName(
		const char *name,
		const char *fname) : Content(name)
	{/*TRACE_FUNC*/
		if(fname == NULL) {
			throw err("WEB::OUT - File name of output is NULL\nContent name is \"%s\"", name);
		}
		
		int len = strlen(fname);
		if(len < 1) {
			throw err("WEB::OUT - File name of output is a empty string");
		}
		this->fname = new char[len +1];
		if(this->fname == NULL) {
			throw err("CWEB::OUT - In allocation memory for output file name.\n"
			"file name size is %d bytes\nfile name is \"%s\"\nName is \"%s\"\nerro is %d\n"
			"str erro is \"%s\"", len, fname, name, errno, strerror(errno));
		}
		strcpy(this->fname, fname);
	}
	
	~ContentFileName() {
		delete[] fname;
	}
	
	char* get_new_name_tag_add()
	{/*TRACE_FUNC*/
		static int num = 0;
	
		const char prefix[14] = "___tag_add___";
	
		char *tagName = new char[strlen(prefix) + sizeof(int) +1];
		if(tagName == NULL) {
			throw err("WEB::OUT - Allocating space for new name of tag add\n"
			"Name of the file who tried to add is \"%s\"\nId of tag is %d\n"
			"Tag is \"%s\"\nerro is %d\nstr erro is \"%s\"",
			fname, num, tag, errno, strerror(errno));
		}
	
		sprintf(tagName, "%s%d", prefix, num);
		++num; // update num for the next name
	
		return tagName;
	}
	
	char* type() {/*TRACE_FUNC*/
		return const_cast<char*>("file name");
	}
	
	void init_data() {/*TRACE_FUNC*/
		data = u::file::toStr(fname);
	}
};

class Out
{ private:
	////////////////////////////////////////////////////////////////////////////////
	// inner structs
	////////////////////////////////////////////////////////////////////////////////
	struct cmp_str // use this to compare strings (C-style) in map function to find key map
	{
		bool operator()(char const *a, char const *b) const
		{
			return std::strcmp(a, b) < 0;
   		}
	};
	
	////////////////////////////////////////////////////////////////////////////////
	// inner variables
	////////////////////////////////////////////////////////////////////////////////
	std::map<const char*, Content*, cmp_str> _map;
	std::deque<Content*> _queue;
	//queue<Print*> _print;
	
	////////////////////////////////////////////////////////////////////////////////
	// 
	////////////////////////////////////////////////////////////////////////////////
 public:
 	/**
 	 * Esta variável static running é utilizada para unificar e homogenizar a execução
 	 * tanto das operações normais quanto das operações de erro (_out e _error).
 	 * Ela guarda a instância de quem está operando e chama de acordo com quem 
 	 * chamou o print pela última vez.
 	 */
 	static Out *running;
 	
 	void set(const Content& cont)
 	{/*TRACE_FUNC*/
 		auto it = _map.find(cont.name);
		if (it != _map.end()) {
			throw err("CWEB::OUT - There is already a content with the same name in output.\n"
			"Choose another name\nName is \"%s\"", cont.name);
		}
		
		_map[cont.name] = const_cast<Content*>(&cont);
		_queue.push_back(const_cast<Content*>(&cont));
 	}
 	
 	void print()
 	{/*TRACE_FUNC*/
 		running = this;
 		for(auto i : _queue) {
 			i->print();
 		}
	}
	
	void print(const char *name)
	{/*TRACE_FUNC*/
		if(name == NULL)
		{
			log_print_no_key_map(name);
		} 
		
		auto it = _map.find(name);
		if (it == _map.end()) // key not found
		{
			log_print_no_key_map(name);
		}
		
		return it->second->print();
	}
	
	void clean()
	{
		_map.clear();
		_queue.clear();
	}

 private:	
	void log_print_no_key_map(const char *name)
	{/*TRACE_FUNC*/
		err("CWEB::OUT - Fetch for a no name in output.\n"
		"fectch name = \"%s\"\nnumber of keys is %d\n"
		"List of all names in output that be parsed:", name, _map.size());
		
		for(auto elem : _map)
		{
   			fprintf(stderr, "[\"%s\"] = \"%s\"\n", elem.first, elem.second->type());
		}
		
		throw err("");
	}
};

////////////////////////////////////////////////////////////////////////////////
// Variáveis globais
////////////////////////////////////////////////////////////////////////////////
Out _out;
Out _error;
Out* Out::running = NULL;

void call_child(const char *name)
{/*TRACE_FUNC*/
	/**
	 * não precisa verificar se 'name' e 'Out::running' são NULL.
	 * as funções anteriores já fizeram isso.
	 * como essa função não é exportada, ela sempre será segura.
	 */
	Out::running->print(name);
}

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////

void
cweb::out::str(
	const std::string& name,
	const std::string& data)
{
	return _out.set(*(new Content(name.c_str(), data.c_str())));
}

void
cweb::out::file(
	const std::string& name,
	FILE *file)
{
	return _out.set(*(new ContentFile(name.c_str(), file)));
}

void
cweb::out::file(
	const std::string& name,
	const std::string& fname)
{
	return _out.set(*(new ContentFileName(name.c_str(), fname.c_str())));
}

void
cweb::out::print()
{
	return _out.print();
}

void
cweb::out::clean()
{
	return _out.clean();
}

////////////////////////////////////////////////////////////////////////////////
// err Interface
////////////////////////////////////////////////////////////////////////////////

void
cweb::out::error::str(
	const char *name,
	const char *data)
{
	return _error.set(*(new Content(name, data)));
}

void
cweb::out::error::file(
	const char *name,
	FILE *file)
{
	return _error.set(*(new ContentFile(name, file)));
}

void
cweb::out::error::file(
	const char *name,
	const char *fname)
{
	return _error.set(*(new ContentFileName(name, fname)));
}

void
cweb::out::error::print()
{
	return _error.print();
}

void
cweb::out::error::clean()
{
	return _error.clean();
}


































