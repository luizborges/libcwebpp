/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../../cweb.hpp"

////////////////////////////////////////////////////////////////////////////////
// variável global da sessão
////////////////////////////////////////////////////////////////////////////////
//static Session _session;
// w::in_unify_t w::in;
std::unordered_map<std::string, std::string> w::in;

////////////////////////////////////////////////////////////////////////////////
// Public Function - class in_unify_t
////////////////////////////////////////////////////////////////////////////////
void w::in_unify_t::init(std::string& http_encode_input)
{ try {
	var = w::fill_map<std::unordered_map<std::string, std::string>>(http_encode_input, '&');
 } catch(std::exception const& e) { throw err(e.what()); }
}

std::string& w::in_unify_t::operator[](const std::string& key)
{ try {
	if(key.empty()) {
		throw err("input key is empty.");
	}
	
	auto svar = var.find(key);
	if(svar == var.end()) {// não existe a chave no map
		throw err("input key: \"%s\" does not exists.", key.c_str());
	} else 	{ // existe a chave no map
		return svar->second;
	}
 } catch(std::exception const& e) { throw err(e.what()); }
}

std::string& w::in_unify_t::operator[](const std::string& key) const
{ try {
	if(key.empty()) {
		throw err("input key is empty.");
	}
	
	auto svar = var.find(key);
	if(svar == var.end()) {// não existe a chave no map
		throw err("input key: \"%s\" does not exists.", key.c_str());
	} else 	{ // existe a chave no map
		return const_cast<std::string&>(svar->second);
	}
 } catch(std::exception const& e) { throw err(e.what()); }
}

////////////////////////////////////////////////////////////////////////////////
// Public Function - input methods
////////////////////////////////////////////////////////////////////////////////
void w::in_init(const long max_size)
{ try {
	std::string request_method = u::to_str(getenv("REQUEST_METHOD"));
	
	// descobre qual o REQUEST_METHOD passado
	std::string input_data = "";
	if(request_method == "GET") {
		input_data = u::to_str(getenv("QUERY_STRING"));
		if(max_size != -1) { // check the size of input
			if(input_data.size() > static_cast<size_t>(max_size)) {
				throw err("GET CONTENT LENGTH IS BIGGER THAN MAX SIZE ALLOWED. CONTENT_LENGTH: %ld | MAX_SIZE: %ld",
				input_data.size(), max_size);
			}
		}
	} else if(request_method == "POST") {
		////////////////////////////////////////////////////////////////////////////////
		// check the size of post
		////////////////////////////////////////////////////////////////////////////////
		if(max_size != -1) {  // check the size of input
			long size = std::stol(u::to_str(getenv("CONTENT_LENGTH")));
			if(size > max_size) {
				throw err("POST CONTENT LENGTH IS BIGGER THAN MAX SIZE ALLOWED. CONTENT_LENGTH: %ld | MAX_SIZE: %ld",
				size, max_size);
			}
		}
		
		////////////////////////////////////////////////////////////////////////////////
		// get the post content & check the content_type
		////////////////////////////////////////////////////////////////////////////////
		//std::ifstream t(std::cin); // open stdin into a file
		//input_data((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>()); // put the file in string
		//input_data((std::istreambuf_iterator<char>(std::cin)), std::istreambuf_iterator<char>()); // put the file in string
		std::stringstream str_stream;
	    str_stream << std::cin.rdbuf(); //read the file
    	input_data = str_stream.str(); //str holds the content of the file
				
		std::string content_type = u::to_str(getenv("CONTENT_TYPE"));
		if(content_type == "application/x-www-form-urlencoded") {} 
		else if(content_type == "multipart/form-data") {
			throw err("getenv(\"CONTENT_TYPE\") = \"multipart/form-data\" no implement yet.");
		} else {
			if(content_type.empty()) { throw err("getenv(\"CONTENT_TYPE\") is empty."); }
			
			throw err("CWEB::IN - CONTENT_TYPE NOT IMPLEMENTED.\n"
			"getenv(\"CONTENT_TYPE\") = \"%s\". | This library only implemented getenv(\"CONTENT_TYPE\") = "
			"\"application/x-www-form-urlencoded\" or \"multipart/form-data\"\n", content_type.c_str());
		}
	} else {
		if(request_method.empty()) { throw err("getenv(\"REQUEST_METHOD\") is empty."); }
		throw err("CWEB::IN - REQUEST_METHOD do not recognize. | REQUEST_METHOD = \"%s\"", request_method.c_str());
	}
	
	// w::in.init(input_data); // inicializa a variável que guarda o http input
	w::in = w::fill_map<std::unordered_map<std::string, std::string>>(input_data);
	
 } catch(std::exception const& e) { throw err(e.what()); }
}

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////









