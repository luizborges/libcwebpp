/**
 *
 * @descripion: funciona como um header que contém todas as bibliotecas, é necessário apenas adicionar
 * essa biblioteca para ter acesso a todas a biblioteca.
 */
#ifndef CWEBPP_H
#define CWEBPP_H


////////////////////////////////////////////////////////////////////////////////
// Includes - default libraries - C
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>

#include <unistd.h> // unix standard library
#include <sys/syscall.h>

////////////////////////////////////////////////////////////////////////////////
// Includes - default libraries - C++
////////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <ctime>
#include <ctype.h>
#include <map> 
#include <deque>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <streambuf>
#include <exception>
#include <list>

#include <pqxx/pqxx> // postgres

//using namespace std;
////////////////////////////////////////////////////////////////////////////////
// Includes - system dependent libraries
////////////////////////////////////////////////////////////////////////////////
#if defined(unix) || defined(__unix) || defined(__unix__) || (defined (__APPLE__) && defined (__MACH__)) // Unix (Linux, *BSD, Mac OS X)
#include <sys/types.h>
#include <dirent.h>
#endif

////////////////////////////////////////////////////////////////////////////////
// Includes - my libraries
////////////////////////////////////////////////////////////////////////////////
#include <util.hpp>
//#include <headers/stackTracer.h>
////////////////////////////////////////////////////////////////////////////////
// Includes - namespace
////////////////////////////////////////////////////////////////////////////////
/**#define TRACE_FUNC \
fprintf(stderr, "\n*****************************************\n");\
fprintf(stderr, "TRACE FUNC:: \"%s\" (%d, \"%s\")\n",\
__PRETTY_FUNCTION__, __LINE__, __FILE__);\
fprintf(stderr, "*****************************************\n");
*/
////////////////////////////////////////////////////////////////////////////////
// namespace
////////////////////////////////////////////////////////////////////////////////
namespace cweb 
{

////////////////////////////////////////////////////////////////////////////////
// Includes - Url Percent Encode/Decode
////////////////////////////////////////////////////////////////////////////////
// implementing in file -
extern bool
decode(char* out,
	   const char* in,
	   const int maxDecode);

/**
 * substituiu o character ' ' pelo character '+'.
 * @return retorna a string codificada em caso de acerto.
 * em caso de erro, retorna nullptr.
 */
extern char*
encode(const char* in,
	   const int maxEncode);


////////////////////////////////////////////////////////////////////////////////
// Includes - Cookie
////////////////////////////////////////////////////////////////////////////////
namespace cookie
{
	extern bool init();

	/**
	 * @arg key: if the value is nullptr, this function return nullptr.
	 * @return: nullptr if the key does not exists in map or the value is nullptr.
	 */
	extern const char* get(const char *key);
	
	
	/**
	 * Retorna uma string contendo todo o conteúdo do cookie.
	 * os valores key e value, não podem ser nullptr e também não podem ser uma
	 * string vazia.
	 * Insert the string: " GMT" ao final do tempo designado.
	 * O tempo é recuperado com a função localtime(time(nullptr) + expires_sec)
	 * Se o argumento "expires_sec" é 0, ele não é inserido.
	 * Se os argumentos "domain" e "path" são nullptrs, eles não são inseridos.
	 * se os argumentos "isSecure" e "isHttpOnly" são false, eles não são inseridos.
	 * Ao final da string do cookie é incluído o charactere '\n'
	 * OBS: se o valor do expires_sec for -1, o tempo será setado para
	 * "expires=Thu, 01 Jan 1970 00:00:00 GMT" este valor é usado para expirar o cookie
 	*/
	extern char*
	set(const char *key, const char *value,
		const long expires_sec = 0,
		const bool isSecure = false,
		const bool isHttpOnly = false,
		const char *domain = getenv("SERVER_NAME"),
		const char *path = "/");
	
	
	/**
	 * A saída é gravada pela função fprintf(stdout, ... );
	 * A gravação é feita por partes, ou seja, não é criada toda a string e então
	 * inserida no stdout.
	 * os valores key e value, não podem ser nullptr e também não podem ser uma
	 * string vazia.
	 * Insert the string: " GMT" ao final do tempo designado.
	 * O tempo é recuperado com a função localtime(time(nullptr) + expires_sec)
	 * Se o argumento "expires_sec" é 0, ele não é inserido.
	 * Se os argumentos "domain" e "path" são nullptrs, eles não são inseridos.
	 * se os argumentos "isSecure" e "isHttpOnly" são false, eles não são inseridos.
	 * Ao final da string do cookie é incluído o charactere '\n'
	 * OBS: se o valor do expires_sec for -1, o tempo será setado para
	 * "expires=Thu, 01 Jan 1970 00:00:00 GMT" este valor é usado para expirar o cookie
	 */
	extern void
	print(const char *key, const char *value,
		const long expires_sec = 0,
		const bool isSecure = false,
		const bool isHttpOnly = false,
		const char *domain = getenv("SERVER_NAME"),
		const char *path = "/");
	
	/**
	 * delete a cookie
	 * funciona como a função:
	 * cweb::cookie::set(key, "", -1);
	 */
	inline char* del(const char *key);
		
	/**
	 * print delete a cookie
	 * funciona como a função:
	 * cweb::cookie::print(key, "", -1);
	 */
	inline void pdel(const char *key);
}

////////////////////////////////////////////////////////////////////////////////
// Interface to treat File in getenv(\"CONTENT_TYPE\") =  "multipart/form-data"
////////////////////////////////////////////////////////////////////////////////
class file
{ public:
	virtual char* name() = 0;
	virtual long  size() = 0;
	virtual char* data() = 0;
	virtual char  type() = 0;
	virtual char* typeStr() = 0;
};
////////////////////////////////////////////////////////////////////////////////
// Includes - In - input - get and post
////////////////////////////////////////////////////////////////////////////////
namespace in
{
	/**
	 * inicializa os métodos abaixos.
	 * a utilização de qualquer função abaixo, sem esta inicialização gera
	 * comportamento indefinido.
	 * @arg max_size = tamanho máximo da entrada, em bytes.
	 * Se o valor for <= 0, essa variável será desconsiderada, que é o valor default
	 */
	extern bool init(const long max_size = 0);
	
	/**
	 * serve tanto para 'get' quanto 'post' ("application/x-www-form-urlencoded").
	 * a função init(); seta automaticamente qual é o method utilizado.
	 */
	extern char* read(const char *key);
	
	/**
	 * apenas para o getenv("REQUEST_METHOD") = "GET".
	 * valores diferentes o comportamento é indefinido.
	 */
	extern char* get(const char *key);
	
	/**
	 * apenas para o getenv("REQUEST_METHOD") = "POST".
	 * getenv("CONTENT_TYPE") = "application/x-www-form-urlencoded"
	 * valores diferentes o comportamento é indefinido.
	 */
	extern char* post(const char *key);
	
	/**
	 * apenas para o getenv("REQUEST_METHOD") = "POST".
	 * getenv("CONTENT_TYPE") = "multipart/form-data"
	 * valores diferentes o comportamento é indefinido.
	 */
	extern file& fpost(const char *key); 
	
	/**
	 * Inicializa a estrutura para armazenagem dos valores se:
	 * getenv("REQUEST_METHOD") = "GET".
	 * OBS: não é necessário chamar, pois já é chamada implicitamente pelo init();
	 */
	extern bool init_get(const long max_size = 0);
	
	/**
	 * Inicializa a estrutura para armazenagem dos valores se:
	 * getenv("REQUEST_METHOD") = "POST".
	 * getenv("CONTENT_TYPE") = "application/x-www-form-urlencoded"
	 * OBS: não é necessário chamar, pois já é chamada implicitamente pelo init();
	 */
	extern bool init_post(const long max_size = 0);
	
	/**
	 * Inicializa a estrutura para armazenagem dos valores se:
	 * getenv("REQUEST_METHOD") = "POST".
	 * getenv("CONTENT_TYPE") = "multipart/form-data"
	 * OBS: não é necessário chamar, pois já é chamada implicitamente pelo init();
	 */
	extern bool init_fpost(const long max_size = 0);
	
	/**
	 * Verifica se o valor de user_method é igual 
	 * ao retornado pela função "getenv("REQUEST_METHOD")";
	 * Também verifica se o valor de type é igual
	 * ao retornado pela função "getenv("CONTENT_TYPE")"
	 * @OBS: se o valor de type = nullptr -> não faz a verificação.
	 * @OBS: o valor de user_method sempre é comparado com getenv("REQUEST_METHOD")
	 * mesmo se ambos forem nullptr
	 * @throw: essa sempre lança uma exceção em caso de erro.
	 * a exceção é uma "string&", cujo conteúdo contem:
	 * 1-> identificação: "cweb::error::inMethod["
	 * 2-> onde ocorreu o erro na verificação.
	 * TODO - implementar uma conferência por meio de uma classe de erro específica.
	 */
	extern void method(const char *user_method = nullptr,
						const char *type = nullptr);
}

////////////////////////////////////////////////////////////////////////////////
// Includes - Session
////////////////////////////////////////////////////////////////////////////////
namespace session
{
	/**
	 * @dirFileSession: diretório em que será criado o arquivo de Sessão.
	 * @lifeSession: tempo que uma sessão dura - tempo em segundos.
	 * @del: tempo (em segundos) para se deletar, dentro do diretório, todos os aruqivos de
	 * sessão. A primeiro coisa que ele faz, é entrar dentro do diretório, e deletar
	 * todos os arquivos de sessão que estão dentro do diretório que foram criados
	 * a mais de 'del' segundos, onde del é esta variável.
	 * @OBS: se a variável @del, for menor que 0, está função não deleta os arquivos
	 * de sessão antigos no diretório indicado.
	 * @OBS: se a variável @del for menor que a variável @lifeSession, está função não
	 * deleta os arquivos da sessão antigos no diretório indicado.
	 * é como se o valor fosse @del < 1.
	 * @OBS: para a opção de deletar os arquivos antigos da sessão, deve-se ter:
	 * if(@del > @lifeSession)
	 */
	extern void
	config(const char *dirFileSession = "",
			const time_t lifeSession = 5*60, // valor default de 5 min
			const time_t del = 7*60); // valor default de 7 min
	
	
	/**
	 * Carrega a sessão que se encontra no arquivo de sessão definido pelo cookie "sid".
	 * @return: true: caso consiga carregar os valores.
	 * false: otherwise
	 */
	extern bool load();
	
	/**
	 * Retorna o valor do cookie sid.
	 * Sempre gera um novo sid.
	 * Uma vez que está função roda, a função 'save()' já conhece o sid.
	 */
	extern char* sid();
	
	/**
	 * Retorna o valor do cookie sid.
	 * uma vez que a função 'sid()' foi chamada, esta função já reconhece o sid().
	 * caso a função 'sid()' não foi chamada anteriormente, 
	 * e se deseja salvar a sessão, deve-se chamar a função como
	 * o parâmetro como true: cweb::session::save(true)
	 */
	extern char* save(const bool toSave = false);
	
	/**
	 * Retorna true em caso de inserção e falso em caso contrário.
	 */
	extern bool setv(
		const char *key = nullptr,
		const void *value = nullptr,
		const int size = sizeof(int),
		const int numElem = 1);

	extern inline bool
	set(const char *key, const char *value);
	
	extern inline bool
	set(const char *key, const int value);

	/**
	 * Retorna nullptr caso a chave key não exista no map.
	 */
	extern void* getv( // get vector
		const char *key,
		int *numElem = nullptr,
		int *size = nullptr);
	
	/**
	 * Retorna nullptr caso a chave key não exista no map.
	 */		
	extern inline char* get(const char *key);
	extern inline int geti(const char *key);
	
	extern void* delv( // del vector
		const char *key,
		int *numElem = nullptr,
		int *size = nullptr);
	
	extern inline char* del(const char *key);
	extern inline int deli(const char *key);
	
	/**
	 * remove todas as chaves e seus valores da sessão.
	 * @return retorna o número de chaves excluídas.
	 */
	extern int clean(); 
}

////////////////////////////////////////////////////////////////////////////////
// Includes - Out - output
////////////////////////////////////////////////////////////////////////////////
namespace out
{
	////////////////////////////////////////////////////////////////////////////////
	// how to use
	////////////////////////////////////////////////////////////////////////////////
	/*
 	 * @descripion: como funciona o parser para encontrar a tag <?cweb CONTENT ?>
	 * '<?cweb #in "name" ?>' -> inclui o objeto chamado name, que foi colocado nas funções:
	 * void str(const char *name, const char *output);
	 * void file(const char *name, FILE *output);
	 * void file(const char *name, const char *file_name);
	 * onde "name" da tag é o parâmetro 'name'.
	 * '<?cweb #add "file_name" ?>' -> inclui o conteúdo do arquivo, cujo nome é "file_name"
	 * A diferença é que esta tag não necessita de ser inserida por meio das funções de inserção de saída.
	 * A tag será então substituída pelo conteúdo do objeto name, pode ser um arquivo ou uma string.
	 * Após a inserão, continuará a impressão do objeto de onde parou.
	 * '<?cweb@' -> imprime normalemnte o valor '<?cweb' -> ou seja, o caracter '@' será omitido,  
	 * porém isto somente ocorre com o primeiro caracter posterior.
	 * - deve-se utilizar tal artifício ('<?cweb@') nos comentários, pois o * parser não distingue se a linha está em um comentário ou não.
	 * @ TODO - criar uma nova versão que verifica se a linha está dentro de um comentário ou não *
	 * all character of name must be a letter (A to Z or a to z) or a digit (0 to 9) or special character ('_' and '-')
	 * O nome da tag inserida nas funções -> 'name', não pode ser "___tag_add___XXX"
	 * OBS1: "void file(const char *name, FILE *output);"
	 * A função NÃO chama a função rewind(FILE *output) explicitamente, nem está
	 * função nem a função "void print()"
	 * ou seja será impresso o arquivo, a partir da posição informada pelo poteiro
	 * FILE *output, no momeno em que a função "void print()" for chamada.
	 * OBS2: o conteúdo da variável "const char *name" é copiado para dentro da classe
	 * assim o usuário é livre para reutilizar a memória, sem se preocupar com resultado
	 * indefinido pela biblioteca.
	 * OBS3: o conteúdo das variáveis "const char *output" da função "void str(const char *name, const char *output)" e a variável "const char *file_name" da função "void file(const char *name, const char *file_name);" são copiados para dentro da biblioteca, para o usuário
	 * ser livre para reutilizar e manusear a memória como bem lhe parecer.
	 * para liberar essas memórias utilize a função "void clean()"
 */

	extern void  str(const std::string& name, const std::string& output);
	extern void file(const std::string& name, FILE *output);
	extern void file(const std::string& name, const std::string& file_name);
	
	/**
	 * Envia o arquivo gerado na saída para a saída padrão do fast-cgi.
	 * NÃO É RECURSIVO - Se um arquivo já foi impresso, ele NÃO será novamente impresso.
	 * Isto é feito para evitar entradas em loops infinitos
	 */
	extern void print();
	
	/**
	 * se utilizado imediatamente antes da função print, o comportamento é indefinido.
	 * como a função copia 
	 */
	extern void clean();
	
	namespace error
	{
		extern void str(const char *name, const char *output);
	
		extern void file(const char *name, FILE *output);
	
		extern void file(const char *name, const char *file_name);
		
		extern void print();
		
		extern void clean();
	}
}

////////////////////////////////////////////////////////////////////////////////
// Includes - Route
////////////////////////////////////////////////////////////////////////////////
/**
 * os parâmetros são os mesmos.
 * @PARAM PATH = string que contém o caminho requisitado, ou seja, deve ser igual ao
 * retornado pela variável PATH_INFO que é preenchida pelo HTTP.
 * @PARAM FUNC = função que deve ser chamada, caso o endereço seja requisitado.
 * para utilizar mais de um código, apenas usar o parênteses ex: page_index(); page_index_2()
 */
#define cweb_route_init(PATH, FUNC) \
	char *cweb_route_path = getenv("PATH_INFO"); \
	if(cweb_route_path == nullptr) {\
		FUNC;\
	} else if(strcmp(cweb_route_path, PATH) == 0) {\
		FUNC;\
	}

#define cweb_route(PATH, FUNC) \
	else if(strcmp(cweb_route_path, PATH) == 0) {\
		FUNC;\
	}

#define cweb_route_error(FUNC) \
	else {\
		FUNC;\
	}
} // end namespace cweb


////////////////////////////////////////////////////////////////////////////////
// Implementation of templates and inline functions
////////////////////////////////////////////////////////////////////////////////
inline char*
cweb::cookie::del(const char *key)
{
	return cweb::cookie::set(key, "", -1);
}

inline void
cweb::cookie::pdel(const char *key)
{
	return cweb::cookie::print(key, "", -1);
}

inline bool
cweb::session::set(const char *key, const char *value)
{
	return cweb::session::setv(key, value,
		sizeof(char), strlen(value)+1);
}

inline char*
cweb::session::get(const char *key)
{
	return static_cast<char*>(
		cweb::session::getv(key));
}

inline char*
cweb::session::del(const char *key)
{
	return static_cast<char*>(
		cweb::session::delv(key));
}

inline bool
cweb::session::set(const char *key, const int value)
{ try {
	int* _i = new int;
	*_i = value;
	return cweb::session::setv(key, _i,
		sizeof(int), 1);
 } catch(u::error& e) { throw err("");
 } catch(std::bad_alloc& e) { throw err("std::bad_alloc - \"%s\"", e.what()); }
}

inline int
cweb::session::geti(const char *key)
{
	int* _i = static_cast<int*>(
		cweb::session::getv(key));
	if(_i == nullptr) throw err("no integer key\nkey = \"%s\"", key);
	return *_i;
}

inline int
cweb::session::deli(const char *key)
{
	int* _i = static_cast<int*>(
		cweb::session::delv(key));
	if(_i == nullptr) throw err("no integer key\nkey = \"%s\"", key);
	return *_i;
}

////////////////////////////////////////////////////////////////////////////////
// namespace cweb++
////////////////////////////////////////////////////////////////////////////////
namespace w
{
	////////////////////////////////////////////////////////////////////////////////
	// Includes - Session
	////////////////////////////////////////////////////////////////////////////////
    /**
     * // deleta todas as linhas da tabela head e body em que o tempo da sessão (head.date) é maior que 5 minutos
     * delete from head where (EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - date)/60))::NUMERIC > 5.0;
	 * // ver qual o número de linhas das tableas body and head
	 * select (select count(*) from head) as head_count, count(*) as body_count from body;
     */
	/**
	 * Observações gerais:
	 * 1. a funções: operator[], del() e save() devem ser utilizadas somente com uma sessão
	 * já existente e carregada. - comportamento indefinido se isto não ocorrer.
	 * 2. para utilizar esta classe de sessão:
	 * 2.1: banco de dados postgres
	 * 2.2: no banco de dados deve-se ter:
	 * >> tabela chamada head: com colunas(tipo_da_coluna(nome)): varchar(id) e timestamp(date) 
	 * >> tabela chamada body: com colunas(tipo_da_coluna(nome)): varchar(head_id); varchar(name), varchar(value)
	 * a coluna body.head_id é uma foreign key para a coluna: head.id
	 * 3. Para iniciar uma sessão sempre utilizar a função: create();
	 * 4. Para carregar uma sessão utilizar a função: load() -> está função diz se existe
	 * uma sessão ou não viva no banco de dados.
	 * 5. Para utilizar os valores da sessão, basta utilizar o operador[]
	 * 6. Para deletar a sessão, tanto no banco de dados como na memória, basta utilizar a função: del()
	 */
	class session_postgres_t
	{
	 public:
     /**
      * Esta estrutura serve para armazenar a os valores da variável de sessão.
      * Uma vez que a variável é inserida no banco, ela nunca é deletada, somente quando
      * a sessão (linhas) no banco de dados é excluída ela é deletada.
      * @arg val: guarda o valor da variável da sessão
      * @arg val_db: guarda o valor original que está no banco de dados.
      * isto é útil para verificar se realmente é necessário rodar em save() o comando update do sql
      * caso não seja necessário, nada é feito, ou seja, a variável de sessão foi apenas para leitura, não houve
      * alteração do valor que consta no banco de dados.
      * @arg insert: seta se a função save() irá para executar um update or um insert para o valor de val.
      */
	 struct val_t {
	  	std::string val = ""; // valor que será salvo no banco - o usuário atualiza e recebe este valor
	  	std::string val_db = ""; // valor original que existe no banco de dados; caso não exista é vazio
	  	bool insert = true; // se a operação feita pela função save() deve ser insert or update
	 };
	 
	 private:
	 	std::string connection_arg;
		int max_time_session; // time of session in minutes
		std::unordered_map<std::string, val_t> var;
		std::string sid = ""; // session id
		
		// variáveis para verificar o tamanho dos campos na sessão estão conforme os limites com o banco de dados da sessão
		size_t max_size_session_key;
		size_t max_size_session_val;
		size_t max_size_session_id;
		
	 public:
	 	/**
	 	 * @arg connection_arg = string necessária para abrir uma conexão com o banco
	 	 * de dados que contém as tabelas da sessão.
	 	 * @arg max_time_session = tempo máximo que a sessão fica viva em minutos.
	 	 * @obs as variáveis abaixo servem para evitar erro na gravação com o banco de dados, ou seja, guardam os 
	 	 * valores máximos que os respectivos campos do banco de dados podem ter
	 	 * @arg max_size_session_key = tamanho máximo da string que a chave da sessão pode ter
	 	 * @arg max_size_max_size_session_val = tamanho máximo que a string que representa o valor da variável de sessão pode ter
	 	 * @arg max_size_session_id = tamanho máximo da string que representa o 'sid' pode ter
	 	 */
	 	void config(const std::string& connection_arg, const size_t max_time_session,
	 		const size_t max_size_session_key = 4096, const size_t max_size_session_val = 4194304, // 4194304 = 4096*1024
	 		const size_t max_size_session_id = 2048);
	 	
	 	/**
	 	 * Cria uma sessão no banco de dados.
	 	 * @return Cria um novo número sid, e retorna o valor dele para ser enviado ao usuário.
	 	 */
	 	std::string create();
	 	
	 	/**
	 	 * Carrega uma sessão que já existe no banco de dados.
	 	 * Para ser válida a sessão deve existir e
	 	 * a diferença de tempo entre a última chamada da função save() 
	 	 * e o tempo atual deve ser menor ou igual que max_time_session
	 	 * Guarda o valor passado na variável sid: this->sid = sid;
	 	 * @return  true se a sessão existe e é válida.
	 	 * 			fasle otherwise.
	 	 */
	 	bool load(const std::string& sid);
	 	
	 	/**
	 	 * Salva os valores da sessão no banco de dados da sessão.
	 	 * Atualiza o tempo de vida da sessão no banco de dados.
	 	 * Utiliza o this->sid.
	 	 * Deve ser sempre a última coisa a ser chamada de qualquer operação relacionado a sessão.
	 	 */
	 	void save();
	 	
	 	/**
	 	 * Deleta a sessão do usuário.
	 	 * Deleta tanto o map da sessão.
	 	 * Deleta as linhas (que correspondem ao this->sid) da sessão no banco de dados da sessão.
	 	 * Utiliza o this->sid.
	 	 * Ao final o this->sid = "";
	 	 */
	 	void del();
	 	
	 	/**
	 	 * Utiliza o this->sid.
	 	 * Recupera o valor da sessão por meio de sua chave.
	 	 * Caso a chave não exista no map, esta função realiza o seguinte procedimento:
	 	 * 1. busca no banco de dados da sessão se a chave existe no BD.
	 	 * 	caso exista, insere a chave com o valor no map e retorna o valor para o usuário.
	 	 *  	marca que tal valor deve ser para atualizar, no sql executado em save()
	 	 *  case não exista, ele cria apenas no map a tupla (key, value) com o valor vazio e retorna
	 	 * 		para o usuário uma string vazia
	 	 */
    	std::string& operator[](const std::string& key);
    	//const std::string& operator[](const std::string& key) const;
	 	
	 	////////////////////////////////////////////////////////////////////////////////
		// public functions - for range interators - for work with for range loop
		////////////////////////////////////////////////////////////////////////////////
		/**
		 * This code is for use:
		 * obj B
		 * for(auto const& it : B)
		 * where it is the one element of std::unordered_map<std::string, field> _field
		 * for(auto const& it : B) = for(auto const& it : B._field)
		 */
		 inline std::unordered_map<std::string, val_t>::iterator begin(){
        	return var.begin();
		 }
	     inline std::unordered_map<std::string, val_t>::iterator end(){
    	    return var.end();
    	 }
    	 inline std::unordered_map<std::string, val_t>::const_iterator begin() const {
    	    return var.begin();
    	 }
    	 inline std::unordered_map<std::string, val_t>::const_iterator end() const {
    	    return var.end();
    	 }
    	 /**
    	  * Essas duas funções abaixo são importantes, para o interador, entretando
    	  * eu já tenho feito elas acima, pois adicionei checks para elas.
    	  * deixei elas pois são do código original que eu copei na internet
    	  * e caso queira reusar o original tenho elas
    	  *
    	 inline const int& operator[](const std::string& key) const {
    	    return _field.at(key);
    	 }
    	 
    	 inline int& operator[](const std::string& key) {
    	    return _field[key];
    	 }*/
	 private:
	 	bool run_sql_select(const std::string& key);
	 	std::string create_sid();
	};
	
	extern session_postgres_t session; // variable init in file session/postgres/session.cpp
	
	////////////////////////////////////////////////////////////////////////////////
	// Decode & Encode // perecent/umap/percentx.cpp
	////////////////////////////////////////////////////////////////////////////////
	/**
	 * Estas funções são baseadas na codificação do POST e GET do HTTP
	 */
	/**
	 * Decodifica uma string
	 * @arg str: string que será decodificada.
	 * @arg i: posição inicial na string. - está posição será alterada. A posição final
	 * é a posição do primeiro character end que ela encontrar, a partir do valor inicial
	 * passado na variável i.
	 * @arg end: charachter que marca o fim da codificação.
	 */
	std::string decode(const std::string& str, int& i, const char end);
	/**
	 * codifica uma string.
	 */
	std::string encode(const std::string& str);
	
	/**
	 * Codifica os valores de um objeto (std::vector, std::list, etc) em uma string
	 * já codificada.
	 * Os valores do objeto NÃO podem estar codificados.
	 * O character separador, limite, entre os elementos na string é o character '&'
	 * @arg obj: objeto que terá seus valores codificados.
	 * @arg num_elem_encode: número máximo de elementos a serem codificados.
	 * se o valor for -1, todos os elementos do objeto serão codificados.
	 * se o valor for maior que obj.size(), todos os elementos também serão codificados.
	 * @return uma string que contém todos os valores do obj codificados, separados pelo character '&'
	 */
	template<typename T>
	std::string encode_obj(const T& obj, const int num_elem_encode = -1);
	
	/**
	 * Decodifica uma string e passa seus valores para um objeto (estrutura unidimensional ordenada)
	 * A estrutura tem que ter a função push_back para funcionar.
	 * Exemplos de estruturas que funcionam com esta função assim: std::vector, std::list, etc.
	 * @arg str: string que será decodificada e seus valores colocado no objeto a ser criado.
	 * @arg lim: character que é o limite entre os valores da codificados da string.
	 * @return: uma estrutura (v.g.: std::vector, std::list) que contém os valores da string
	 * como seus elementos.
	 */
	template<typename T>
	T fill_obj(const std::string& str, const char lim = '&');
	
	/**
	 * Decodifica uma string e passa seus valores para um map.
	 * @arg: str: string que será decoficiada.
	 * @arg: lim: charachter que marca o limite entre as chaves e os valores.
	 * Exemplo: nas funções do HTTP:: GET: '&' - POST: '&'
     * @arg end_key: character que marca o fim da chave e o início do valor do par: chave-valor.
     * Exemplo: nas funções do HTTP:: GET: '=' - POST: '='
     * @obs: as funções que serão usadas nesse template tem que ser possível fazer a seguinte atribuição:
     * T map; map[key] = val; -> onde key e val são std::string.
	 */
	template<typename T>
	T fill_map(const std::string& str, const char lim = '&', const char end_key = '=');
	////////////////////////////////////////////////////////////////////////////////
	// Cookie
	////////////////////////////////////////////////////////////////////////////////
	class cookie_simple_t
	{
	 private:
		std::unordered_map<std::string, std::string> cookie;
		
	 public:
	 	/**
	 	 * @arg max_size: número máximo de bytes que poderá ter a variável HTTP_COOKIE do CGI.
	 	 * maior que o valor aqui referido, é lançado uma exceção.
	 	 * se o valor for -1, é ignorado a checagem de tamanho.
	 	 */
	 	void init(const int max_size = -1);
	 	
	 	/**
	 	 * Envia o cookie para o cliente.
	 	 * @arg name: nome do cookie.
	 	 * @arg value: value of cookie.
	 	 * @arg args: arguments of cookie.
	 	 */
	 	void print(const std::string& name, const std::string& value,
	 		const std::string& args = "Path=/; HttpOnly; Domain="+u::to_str(getenv("SERVER_NAME")));
	 	
	 	/**
	 	 * Envia o cookie para o cliente.
	 	 * sintax sugar:
	 	 * send() = print(name, cookie[name], args)
	 	 * @arg name: nome do cookie.
	 	 * @arg args: arguments of cookie.
	 	 */
	 	inline void send(const std::string& name, 
	 		const std::string& args = "Path=/; HttpOnly; Domain="+u::to_str(getenv("SERVER_NAME"))) {
	 		print(name, cookie[name], args);
	 	}
	 	
	 	/**
	 	 * Envia o cookie, com o comando para deletar o cookie no cliente.
	 	 * @arg name: name of cookie
	 	 */
		inline void del(const std::string& name) {
			print(name, "", "Expires=Thu, 01 Jan 1970 00:00:00 GMT; Path=/; HttpOnly; " //; Secure // no https
				"Domain="+u::to_str(getenv("SERVER_NAME")));
		}
	 	
	 	/**
	 	 * @return o valor do cookie cujo nome é a chave.
	 	 * se o coookie não existe, ele é criado, contendo o valor vazio, e seu valor é retornado.
	 	 */
    	std::string& operator[](const std::string& key);
    	//const std::string& operator[](const std::string& key) const;
	 	
	 	////////////////////////////////////////////////////////////////////////////////
		// public functions - for range interators - for work with for range loop
		////////////////////////////////////////////////////////////////////////////////
		/**
		 * This code is for use:
		 * obj B
		 * for(auto const& it : B)
		 * where it is the one element of std::unordered_map<std::string, field> _field
		 * for(auto const& it : B) = for(auto const& it : B._field)
		 */
		 inline std::unordered_map<std::string, std::string>::iterator begin(){
        	return cookie.begin();
		 }
	     inline std::unordered_map<std::string, std::string>::iterator end(){
    	    return cookie.end();
    	 }
    	 inline std::unordered_map<std::string, std::string>::const_iterator begin() const {
    	    return cookie.begin();
    	 }
    	 inline std::unordered_map<std::string, std::string>::const_iterator end() const {
    	    return cookie.end();
    	 }
    	 /**
    	  * Essas duas funções abaixo são importantes, para o interador, entretando
    	  * eu já tenho feito elas acima, pois adicionei checks para elas.
    	  * deixei elas pois são do código original que eu copei na internet
    	  * e caso queira reusar o original tenho elas
    	  *
    	 inline const int& operator[](const std::string& key) const {
    	    return _field.at(key);
    	 }
    	 
    	 inline int& operator[](const std::string& key) {
    	    return _field[key];
    	 }*/
	};
	
	extern cookie_simple_t cookie; // variable init in file cookie/strUMap_simple/cookie.cpp
	
	////////////////////////////////////////////////////////////////////////////////
	// in - input methods: get post or file_post
	////////////////////////////////////////////////////////////////////////////////
	class in_unify_t
	{ private:
		std::unordered_map<std::string, std::string> var;
		
	 public:
	 	/**
	 	 * Inicializa a variável var da classe.
	 	 * Realiza o decode e preenche o map da classe - variável var.
	 	 */
	 	virtual void init(std::string& http_encode_input);
	 
	 	/**
	 	 * necessary to multipart/form-data
	 	 */
	 	virtual inline std::string type(const std::string& key) { 
	 		throw err("HTTP INPUT NOT POST multipart/form-data");
            key.empty();
	 	}
	 	
	 	virtual inline size_t size() const {
	 		return var.size();
	 	}
	 	
	 	virtual inline bool empty() const {
	 		return var.empty();
	 	}
	 	
	 	virtual inline std::unordered_map<std::string, std::string>::iterator find(const std::string& key) {
			return var.find(key);
		}
		virtual inline std::unordered_map<std::string, std::string>::const_iterator find (const std::string& key) const {
			return var.find(key);
		}
	 	
	 	/**
	 	 * @return o valor da entrada cujo nome é a chave.
	 	 * se o valor não existe, throw exception
	 	 */
    	virtual std::string& operator[](const std::string& key);
    	virtual std::string& operator[](const std::string& key) const;
	 	
	 	////////////////////////////////////////////////////////////////////////////////
		// public functions - for range interators - for work with for range loop
		////////////////////////////////////////////////////////////////////////////////
		/**
		 * This code is for use:
		 * obj B
		 * for(auto const& it : B)
		 * where it is the one element of std::unordered_map<std::string, field> _field
		 * for(auto const& it : B) = for(auto const& it : B._field)
		 */
		 virtual inline std::unordered_map<std::string, std::string>::iterator begin(){
        	return var.begin();
		 }
	     virtual inline std::unordered_map<std::string, std::string>::iterator end(){
    	    return var.end();
    	 }
    	 virtual inline std::unordered_map<std::string, std::string>::const_iterator begin() const {
    	    return var.begin();
    	 }
    	 virtual inline std::unordered_map<std::string, std::string>::const_iterator end() const {
    	    return var.end();
    	 }
    	 /**
    	  * Essas duas funções abaixo são importantes, para o interador, entretando
    	  * eu já tenho feito elas acima, pois adicionei checks para elas.
    	  * deixei elas pois são do código original que eu copei na internet
    	  * e caso queira reusar o original tenho elas
    	  *
    	 inline const int& operator[](const std::string& key) const {
    	    return _field.at(key);
    	 }
    	 
    	 inline int& operator[](const std::string& key) {
    	    return _field[key];
    	 }*/
	};
	
	/**
	 * inicializa o input CGI de acordo com o método correto. - if get or post
 	 * @arg max_size: número máximo de bytes que poderá ter a variável CGI de entrada.
 	 * maior que o valor aqui referido, é lançado uma exceção.
 	 * se o valor for -1, é ignorado a checagem de tamanho.
 	 * se for getenv("REQUEST_METHOD") = GET then: max_size >= QUERY_STRING throw execption
 	 * se for getenv("REQUEST_METHOD") = POST then: max_size >= CONTENT_TYPE throw execption
 	 */
	void in_init(const long max_size = -1); // in file in/unify_umap/in.cpp
	
	extern in_unify_t in; // variable init in file in/unify_umap/in.cpp
} // end namespace w


////////////////////////////////////////////////////////////////////////////////
// Implementation of templates and inline functions
////////////////////////////////////////////////////////////////////////////////
template<typename T>
std::string w::encode_obj(const T& obj, const int num_elem_encode)
{ try {
	// verifica o número de elementos da lista que será decodificado
	if(num_elem_encode < -1) { throw err("number of elements of object to encode: %d\n", num_elem_encode); }
	int max = num_elem_encode == -1 ? obj.size() : num_elem_encode;
	
	std::string str = "";
	
	typename T::const_iterator it = obj.begin();
	for(int i = 0; i < max && it != obj.end(); ++i , ++it)
	{
		str += encode(*it) + "&";
	}
	
	return str;
 } catch(std::exception const& e) { throw err(e.what()); }
}

template<typename T>
T w::fill_obj(const std::string& str, const char lim)
{ try {
	T obj;
	for(int i=0; static_cast<size_t>(i) < str.size(); ++i)
	{
		std::string val = decode(str, i, lim);
		obj.push_back(val);
 	}
 	
	return obj;
 } catch(std::exception const& e) { throw err(e.what()); }
}

template<typename T>
T w::fill_map(const std::string& str, const char lim, const char end_key)
{ try {
	T map;
	for(int i=0; static_cast<size_t>(i) < str.size(); ++i)
	{
		///////////////////////////////////////////////////////////////////
		// busca, decodifica chave do ENCODE - um par de key e conteúdo
		///////////////////////////////////////////////////////////////////
		std::string key = decode(str, i, end_key);
		++i; // ir para o próximo character que iniciára a nova codificação
		std::string val = decode(str, i, lim);
		map[key] = val;
 	}
	return map;
 } catch(std::exception const& e) { throw err(e.what()); }
}









#endif // CWEBPP_H

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////


































