/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
#include "../../cweb.hpp"

////////////////////////////////////////////////////////////////////////////////
// Public Function
////////////////////////////////////////////////////////////////////////////////
void w::cookie_simple_t::init(const int max_size)
{ try {
	std::string http_cookie = u::to_str(getenv("HTTP_COOKIE"));
	if(http_cookie.empty()) return;
	
	if(max_size != -1) { // check the size of cookie
		if(http_cookie.size() > static_cast<size_t>(max_size)) {
			throw err("Cookie max size violation. Cookie size: %ld - Max size allowed: %d",
				http_cookie.size(), max_size);
		}
	}
	
	cookie = w::fill_map<std::unordered_map<std::string, std::string>>(http_cookie, ';');
	
 } catch(std::exception const& e) { throw err(e.what()); }
}	 	

void w::cookie_simple_t::print(const std::string& name, const std::string& value, const std::string& args)
{ try {
	std::printf("Set-Cookie: %s=%s; %s \r\n", name.c_str(), encode(value).c_str(), args.c_str());
 } catch(std::exception const& e) { throw err(e.what()); }
}


std::string& w::cookie_simple_t::operator[](const std::string& key)
{ try {
	if(key.empty()) {
		throw err("cookie key is empty.");
	}
	
	auto svar = cookie.find(key);
	if(svar == cookie.end()) {// não existe a chave no map
		cookie[key] = "";
		return cookie[key];
	} else 	{ // existe a chave no map
		return svar->second;
	}
 } catch(std::exception const& e) { throw err(e.what()); }
}

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
// variável global da sessão
////////////////////////////////////////////////////////////////////////////////
//static Session _session;
w::cookie_simple_t w::cookie;

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////









