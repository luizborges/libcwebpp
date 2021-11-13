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
	// std::cerr << "INSIDE COOKIES\n";
	std::string http_cookie = u::to_str(getenv("HTTP_COOKIE"));
	if(http_cookie.empty()) return;
	
	if(max_size > 0) { // check the size of cookie
		if(http_cookie.size() > static_cast<size_t>(max_size)) {
			throw err("Cookie max size violation. Cookie size: %ld - Max size allowed: %d",
				http_cookie.size(), max_size);
		}
	}
	
	cookie = fill_map_cookie(http_cookie);
	// std::cerr << "cookies in map:\n";
	// for(auto [key, val] : cookie) {
	// 	std::cerr << "key: \'" << key << "\': \'" << val << "\'\n";
	// }
	// std::cerr << "\n";
 } catch(std::exception const& e) { throw err(e.what()); }
}	 	

std::unordered_map<std::string, std::string>
w::cookie_simple_t::fill_map_cookie(const std::string& str)
{ try {
	if(str.empty()) throw err("String input to fill cookie map is empty.");

	std::unordered_map<std::string, std::string> map;
	char end_key = '=';
	char lim = ';';
	for(int i=0; static_cast<size_t>(i) < str.size(); ++i)
	{
		///////////////////////////////////////////////////////////////////
		// busca, decodifica chave do ENCODE - um par de key e conteúdo
		///////////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////
		// busca, check and get the key of cookie
		///////////////////////////////////////////////////////////////////
		if(isspace(str.at(i))) continue; // get the init of key

		// check for erros in cookie key
		if(iscntrl(str.at(i))) throw err("HTTP_COOKIE malformed. Control character found where is expected key of cookie. Position: %d", i);
		const std::string check = "()<>@,;:\\\"/[]?={}";
		for(const auto& c : check)
			if(c == str.at(i))
				throw err("HTTP_COOKIE malformed. Forbidden character found where is expected key of cookie. Position: %d, character forbidden: \'%c\'", i, c);

		std::string key = w::decode(str, i, end_key);

		++i; // ir para o próximo character que iniciára a nova codificação

		///////////////////////////////////////////////////////////////////
		// busca, check and get the value of cookie
		///////////////////////////////////////////////////////////////////
		bool is_value_with_double_quote_wrapper = false;
		if(str.at(i) == '\"') {
			is_value_with_double_quote_wrapper = true;
			++i;
		}

		// check for erros
		for(int j=i; static_cast<size_t>(j) < str.size(); ++j) {
			if(str.at(j) == lim) break;

			if(iscntrl(str.at(j)))
				throw err("HTTP_COOKIE malformed. Control character found where is expected value of cookie. Position: %d, key of value: \'%s\'", j, key.c_str());
			if(isspace(str.at(j)))
				throw err("HTTP_COOKIE malformed. Whitespace character found where is expected value of cookie. Position: %d, key of value: \'%s\'", j, key.c_str());
			
			const std::string check = ",;\\";
			for(const auto& c : check)
				if(str.at(j) == c)
					throw err("HTTP_COOKIE malformed. Forbidden character found where is expected value of cookie. Position: %d, character forbidden: \'%c\', key of value: \'%s\'", j, c, key.c_str());
			
			if(str.at(j) == '\"') {
				if(is_value_with_double_quote_wrapper) {
					int n = j+1;
					if(static_cast<size_t>(n) < str.size() -1) { // tem que haver pelo menos mais um character depois do quote para a verificação
						if(str.at(n) != lim)
							throw err("HTTP_COOKIE malformed. Forbidden character found where is expected value of cookie. After a wrapper \" expected the limit character \'%c\' or end of cookie. Position: %d, key of value: \'%s\'", lim, j, key.c_str());
					}
				} else {
					throw err("HTTP_COOKIE malformed. Forbidden character found where is expected value of cookie. Position: %d, character forbidden: \'%c\', key of value: \'%s\'", j, '\"', key.c_str());
				}
			}
		}
		
		std::string value;
		if(is_value_with_double_quote_wrapper) {
			value = w::decode(str, i, '\"');
			++i; // vai para a posição do character lim
		} else {
			value = w::decode(str, i, lim);
		}

		map[key] = value;
 	}
	return map;
 } catch(std::exception const& e) { throw err("%s\nCookie string: \'%s\'", e.what(), str.c_str()); }
}

void w::cookie_simple_t::print(const std::string& name, const std::string& value, const std::string& args)
{ try {
	// std::printf("Set-Cookie: %s=%s; %s \r\n", name.c_str(), encode(value).c_str(), args.c_str());
	std::cout << "Set-Cookie: " << name << "=" << encode(value) << "; " << args << "\r\n";
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









