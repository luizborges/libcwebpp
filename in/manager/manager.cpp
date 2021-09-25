/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../../cweb.hpp"

////////////////////////////////////////////////////////////////////////////////
// Private Structs
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
// Private Variables
////////////////////////////////////////////////////////////////////////////////
static char* (*_read) (const char*);

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////
bool
cweb::in::init(const long max_size)
{ try {
	char *rm = getenv("REQUEST_METHOD");
	if(rm == nullptr) {
		err("CWEB::IN - getenv(\"REQUEST_METHOD\") is nullptr");
		return false;
	}
	
	// descobre qual o REQUEST_METHOD passado
	if(strcmp(rm, "GET") == 0)
	{
		_read = cweb::in::get;
		return cweb::in::init_get(max_size);
	}
	else if(strcmp(rm, "POST") == 0)
	{
		char *ct = getenv("CONTENT_TYPE");
		if(ct == nullptr) {
			throw err("CWEB::IN - NO CONTENT_TYPE\ngetenv(\"CONTENT_TYPE\") = nullptr");
		}
		if(strcmp(ct, "application/x-www-form-urlencoded") == 0) {
			_read = cweb::in::post;
			return cweb::in::init_post(max_size);
		} 
		else if(strncmp(ct, "multipart/form-data", 19) == 0) {
			_read = nullptr;
			return cweb::in::init_fpost(max_size);
		} else {
			throw err("CWEB::IN - CONTENT_TYPE NOT IMPLEMENTED.\n"
			"getenv(\"CONTENT_TYPE\") = \"%s\".\n"
			"This library only implemented getenv(\"CONTENT_TYPE\") = "
			"\"application/x-www-form-urlencoded\" or \"multipart/form-data\"\n", ct);
		}
	}
	else {
		throw err("CWEB::IN - REQUEST_METHOD do not recognize.\n"
			"REQUEST_METHOD = \"%s\"", rm);
	}
	
	return false;
 } catch(u::error& e) { throw err(); }
}

char*
cweb::in::read(const char *key)
{
	return _read(key);
}

void
cweb::in::method(const char *um, const char *type)
{
	char *rm = getenv("REQUEST_METHOD");
	if(rm == nullptr)
	{
		if(um == nullptr) return; // sucesso pois tanto o valor passado pelo usuário como o da biblioteca são iguais
		//throw cweb::error::inMethod::instance();
		throw *new std::string("cweb::error::inMethod["
		"getenv(\"REQUEST_METHOD\") = nullptr & user_method != nullptr]");
	}
	
	if(um == nullptr) {
		throw *new std::string("cweb::error::inMethod["
		"getenv(\"REQUEST_METHOD\") != nullptr & user_method = nullptr]");
	}
	
	if(strcmp(rm, um) != 0) {
		throw *new std::string("cweb::error::inMethod["
		"getenv(\"REQUEST_METHOD\") != user_method]");
	}
	
	if(type == nullptr) return; // não é necessário continuar a conferir
	
	char *ct = getenv("CONTENT_TYPE");
	if(strcmp(ct, type) != 0) {
		throw *new std::string("cweb::error::inMethod["
		"getenv(\"CONTENT_TYPE\") != type]");
	}
	
}






