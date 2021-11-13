/**
 */
////////////////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////////////////
//#include "session_fileMap.h"
#include "../../cweb.hpp"

////////////////////////////////////////////////////////////////////////////////
// Useful SQL
////////////////////////////////////////////////////////////////////////////////
/**
// seleciona todos as linhas da tabela head em que o tempo delas (date) é maior que 5 minutos
SELECT * from head where (EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - date)/60))::NUMERIC > 5.0;

// deleta todas as linhas da tabela head e body em que o tempo da sessão (head.date) é maior que 5 minutos
delete from head where (EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - date)/60))::NUMERIC > 5.0;
 */

////////////////////////////////////////////////////////////////////////////////
// Public Function
////////////////////////////////////////////////////////////////////////////////
/**
 * esta função preenche a sessão com todos os valores dela no banco de dados.
std::unordered_map<std::string, std::string> global_main_get_all_session_values_in_db(const std::string& sid)
{ try {
	////////////////////////////////////////////////////////////////////////////////
	// verifica o valor de sid & abre conexão e uma transação com o banco de dados da sessão
	////////////////////////////////////////////////////////////////////////////////
	if(u::isdigit(sid, false) == false) return {}; // verifica se contém somente números a string - verifica  também se é vazio
	
	pqxx::connection C(DATABASE_CONNECTION_SESSION);
	try { // necessário for run C.disconnect() in catch()
    	if(!C.is_open()) throw err("Can't open session database.\nconnection_arg: \"%s\"", DATABASE_CONNECTION_SESSION);
	    pqxx::nontransaction N(C); // inicia uma transação com o BD - Create a notransactional object.
    	
    	////////////////////////////////////////////////////////////////////////////////
		// preenche todo o map da sessão
		////////////////////////////////////////////////////////////////////////////////
    	std::string sql = u::sprintf("select name, value from body where head_id = %s;", N.quote(sid).c_str());
    	pqxx::result R{ N.exec(sql) }; // executa o sql 
    	
    	std::unordered_map<std::string, std::string> session;
    	for(const auto& r : R) {
    		std::string name  = r["name"].is_null() ? "" : r["name"].as<string>();
    		std::string value = r["value"].is_null() ? "" : r["value"].as<string>();
    		session[name] = value;
    	}
    	
    	C.disconnect ();
    	return session;
 	} catch (pqxx::sql_error const &e) {
 		C.disconnect(); throw err("SQL error: %s\nQuery was: \"%s\"", e.what(), e.query().c_str());    
	} catch(std::exception const& e) { C.disconnect(); throw err(e.what()); }
 } catch(std::exception const& e) { throw err(e.what()); }
} */

void
w::session_postgres_t::config(
	const std::string& connection_arg, const size_t max_time_session,
	const size_t max_size_session_key, const size_t max_size_session_val,
	const size_t max_size_session_id)
{ try {
	if(connection_arg.empty()) throw err("connection_arg.empty() = true");
	if(max_time_session < 1) throw err("max_size_session_key < 1");
	if(max_size_session_key < 1) throw err("max_time_session < 1");
	if(max_size_session_val < 1) throw err("max_size_session_val < 1");
	if(max_size_session_id < 1) throw err("max_size_session_id < 1");
	this->connection_arg = connection_arg;
	this->max_time_session = max_time_session;
	this->max_size_session_key = max_size_session_key;
	this->max_size_session_val = max_size_session_val;
	this->max_size_session_id = max_size_session_id;
 } catch(std::exception const& e) { throw err(e.what()); }
}

std::string 
w::session_postgres_t::create()
{ try {
	sid = create_sid(); // gera um valor aleatório para o sid
	
	////////////////////////////////////////////////////////////////////////////////
	// insere no banco de dados da sessão o novo sid
	////////////////////////////////////////////////////////////////////////////////
	pqxx::connection C(connection_arg);
	std::unique_ptr<pqxx::work> W; // necessário para não precisar de outro try-catch to do roolback - W->abort() 
   	try{ // necessário for run C.disconnect() in catch()
   	
    	if(!C.is_open()) throw err("Can't open session database.\nconnection_arg: \"%s\"", connection_arg.c_str());
    	W = std::make_unique<pqxx::work>(C); // inicia uma transação com o banco de dados - Create a transactional object
    
    	std::string sql = u::sprintf("INSERT INTO head (id, date) VALUES (%s,CURRENT_TIMESTAMP);", W->quote(sid).c_str());
    	W->exec0(sql); // salva no banco de dados da sessão
		W->commit(); // somente faz o commit se NÃO ocorreu erro algum no salvamento
		C.disconnect(); // fecha a conexão
		return sid;
  	} catch (pqxx::sql_error const &e) {
 		W->abort(); C.disconnect();
 		throw err("Rollback the transaction. SQL error: %s\nQuery was: \"%s\"", e.what(), e.query().c_str());    
  	} catch(std::exception const& e) { C.disconnect(); throw err("NOT rollback the transaction. | %s", e.what()); }
  
 } catch(std::exception const& e) { throw err(e.what()); }
}

bool
w::session_postgres_t::load(const std::string& sid)
{ try {
	////////////////////////////////////////////////////////////////////////////////
	// verifica o valor de sid & abre conexão e uma transação com o banco de dados da sessão
	////////////////////////////////////////////////////////////////////////////////
	if(!check_sid(sid, false)) return false; // verifica se contém somente números a string - verifica também se é vazio
	if(sid.size() > max_size_session_id) {
		err("sid.size() > max_size_session_id\nsid: \"%s\" | "
		"sid.size(): %d | max_size_session_id: %d", sid, sid.size(), max_size_session_id);
		return false;
	}
	
	pqxx::connection C(connection_arg);
	try { // necessário for run C.disconnect() in catch()
    	if(!C.is_open()) throw err("Can't open session database.\nconnection_arg: \"%s\"", connection_arg.c_str());
	    pqxx::nontransaction N(C); // inicia uma transação com o BD /* Create a notransactional object. */
    
    	////////////////////////////////////////////////////////////////////////////////
		// monta o sql e executa
		////////////////////////////////////////////////////////////////////////////////
    	std::string sql = u::sprintf(
    		"SELECT EXTRACT(EPOCH FROM (CURRENT_TIMESTAMP - (select date from head where id =%s)))/60;",
    		N.quote(sid).c_str());
    
    	pqxx::row R{ N.exec1(sql) }; // executa o sql - espera somente 1 única linha
    
    	////////////////////////////////////////////////////////////////////////////////
		// trata a saída do banco de dados & verifica a validade da sessão & atualiza o sid
		////////////////////////////////////////////////////////////////////////////////
		this->sid = ""; // reseta o valor do sid - para o caso do login falhar
    	if(R[0].is_null()) return false; // não existe o sid no banco de dados
    	const int diff_time = static_cast<int>(R[0].as<double>()); // recebe a diferança de tempo
    	if(diff_time > max_time_session) return false; // o tempo da sessão já expirou
    	this->sid = sid; // atualiza o novo valor do sid
    
    	C.disconnect ();
    	return true;
 	} catch (pqxx::sql_error const &e) {
 		C.disconnect(); throw err("SQL error: %s\nQuery was: \"%s\"", e.what(), e.query().c_str());    
	} catch(std::exception const& e) { C.disconnect(); throw err(e.what()); }
 } catch(std::exception const& e) { throw err(e.what()); }
}

bool
w::session_postgres_t::load_all(const std::string& sid)
{ try {
	////////////////////////////////////////////////////////////////////////////////
	// carrega a sessão - inicialmente com o map vazio
	////////////////////////////////////////////////////////////////////////////////
	if(!load(sid)) return false; // caso não seja possível carregar a sessão

	////////////////////////////////////////////////////////////////////////////////
	// carrega todos os valores do sid - para o map.
	////////////////////////////////////////////////////////////////////////////////
	std::string sql = "SELECT name, value FROM body WHERE head_id=" + database::quote(sid) + ";";
	const auto R = database::selectr(sql, connection_arg);

	for(const auto& r : R) { // insere os key/value no this->var que ainda não estão no this->var
    	const std::string key_name  = r["name"].is_null() ? "" : r["name"].as<std::string>();
    	const std::string value = r["value"].is_null() ? "" : r["value"].as<std::string>();
    	const val_t session_value { value };
		var.emplace(key_name, session_value);
	}
	
	return true;
 } catch(std::exception const& e) { throw err(e.what()); }
}


void
w::session_postgres_t::save()
{ try {
	pqxx::connection C(connection_arg);
	std::unique_ptr<pqxx::work> W; // necessário para não precisar de outro try-catch to do roolback - W->abort()
	try { // necessário for run C.disconnect() in catch()
    	if(!C.is_open()) throw err("Can't open session database.\nconnection_arg: \"%s\"", connection_arg.c_str());
	    W = std::make_unique<pqxx::work>(C); // inicia uma transação com o banco de dados /* Create a transactional object. */
    
    	for(const auto& v : var) { // salva as variáveis de sessão no banco de dados
    	
    		if(v.second.val == v.second.val_db) continue;//não faz nada, não houve alteração no o valor que está no banco de dados
    		////////////////////////////////////////////////////////////////////////////////
			// verifica os valores
			////////////////////////////////////////////////////////////////////////////////
			const std::string& key = v.first;
			const std::string& val = v.second.val;
		
			if(key.size() > max_size_session_key) {
				throw err("key.size() > max_size_session_key\nkey: \"%s\" | "
				"key.size(): %d | max_size_session_key: %d", key, key.size(), max_size_session_key);
			}
			if(val.size() > max_size_session_val) {
				throw err("val.size() > max_size_session_key\nval: \"%s\" | "
				"val.size(): %d | max_size_session_val: %d", val, val.size(), max_size_session_val);
			}
			////////////////////////////////////////////////////////////////////////////////
    		// salva os valores e suas respectivas chaves no banco de dados da sessão
    		////////////////////////////////////////////////////////////////////////////////
    		if(v.second.insert == true) {
    			std::string sql = u::sprintf("INSERT INTO body (head_id, name, value) VALUES (%s, %s, %s);", 
					W->quote(sid).c_str(), W->quote(key).c_str(), W->quote(val).c_str());
    			W->exec0(sql);
    		} else {
    			std::string sql = u::sprintf("UPDATE body SET value = %s WHERE head_id = %s AND name = %s;", 
					W->quote(val).c_str(), W->quote(sid).c_str(), W->quote(key).c_str());
    			W->exec0(sql);
    		}
    	}
    
    	////////////////////////////////////////////////////////////////////////////////
		// atualiza o tempo de vida da sessão
		////////////////////////////////////////////////////////////////////////////////
		std::string sql = "UPDATE head SET date = CURRENT_TIMESTAMP WHERE id =" + W->quote(sid) + ";";
		W->exec0(sql); // salva no banco de dados da sessão
    
		W->commit(); // somente faz o commit se NÃO ocorreu erro algum no salvamento
		C.disconnect(); // fecha a conexão
	} catch (pqxx::sql_error const &e) {
 		W->abort(); C.disconnect();
 		throw err("Rollback the transaction. SQL error: %s\nQuery was: \"%s\"", e.what(), e.query().c_str());    
  	} catch(std::exception const& e) { C.disconnect(); throw err("NOT rollback the transaction. | %s", e.what()); }
 } catch(std::exception const& e) { throw err(e.what()); }
}


void
w::session_postgres_t::del()
{ try {
	////////////////////////////////////////////////////////////////////////////////
	// deleta a sessão no banco de dados
	////////////////////////////////////////////////////////////////////////////////
	pqxx::connection C(connection_arg);
	std::unique_ptr<pqxx::work> W; // necessário para não precisar de outro try-catch to do roolback - W->abort()
	try { // necessário for run C.disconnect() in catch()
    	if(!C.is_open()) throw err("Can't open session database.\nconnection_arg: \"%s\"", connection_arg.c_str());
	    W = std::make_unique<pqxx::work>(C); // inicia uma transação com o banco de dados /* Create a transactional object. */
	    
		std::string sql = "DELETE FROM head WHERE id =" + W->quote(sid) + ";";
		W->exec0(sql); // executa no banco de dados da sessão
		W->commit(); // somente faz o commit se NÃO ocorreu erro algum no salvamento
		C.disconnect(); // fecha a conexão
	} catch (pqxx::sql_error const &e) {
 		W->abort(); C.disconnect();
 		throw err("Rollback the transaction. SQL error: %s\nQuery was: \"%s\"", e.what(), e.query().c_str());    
  	} catch(std::exception const& e) { C.disconnect(); throw err("NOT rollback the transaction. | %s", e.what()); }
	
	////////////////////////////////////////////////////////////////////////////////
	// deleta o map & reseta o valor de sid
	////////////////////////////////////////////////////////////////////////////////
	var.clear(); // deleta todos os elementos do map
	sid = ""; // reseta o valor de sid
 } catch(std::exception const& e) { throw err(e.what()); }
}


std::string&
w::session_postgres_t::operator[](const std::string& key)
{ try {
	auto svar = var.find(key);
	if(svar == var.end()) {// não existe a chave no map
		if(run_sql_select(key)) { // a chave existe no banco de dados e é inserida dentro do map
			return var[key].val;
		} else { // a chave não existe no banco de dados
			return var[key].val; // retorna a chave com o valor "" - é criado automaticamente pelo c++ - safe
		}
	} else 	{ // existe a chave no map
		return svar->second.val;
	}
 } catch(std::exception const& e) { throw err(e.what()); }
}

std::string&
w::session_postgres_t::at(const std::string& key)
{ try {
	auto svar = var.find(key);
	if(svar == var.end()) {// não existe a chave no map
		if(run_sql_select(key)) { // a chave existe no banco de dados e é inserida dentro do map
			return var[key].val;
		} else { // a chave não existe no banco de dados
			const std::string error_msg = "Out of Range: key does not exists in SESSION. Key: \'" + key + "\'";
			err("%s", error_msg.c_str()); // emite uma mensagem de erro, para fazer o trace.
			throw std::out_of_range(error_msg);
		}
	} else 	{ // existe a chave no map
		return svar->second.val;
	}
 } catch(std::exception const& e) { throw err(e.what()); }
}

////////////////////////////////////////////////////////////////////////////////
// Private functions
////////////////////////////////////////////////////////////////////////////////
bool
w::session_postgres_t::run_sql_select(const std::string& key)
{ try {
	if(key.size() > max_size_session_key) { // verifica o  tamanho da chave
		throw err("key.size() > max_size_session_key\nkey: \"%s\" | "
		"key.size(): %d | max_size_session_key: %d", key, key.size(), max_size_session_key);
	}
	////////////////////////////////////////////////////////////////////////////////
	// abre conexão & busca chave no BD & verifica o resultado
	////////////////////////////////////////////////////////////////////////////////
	pqxx::connection C(connection_arg);
	try { // necessário for run C.disconnect() in catch()
    	if(!C.is_open()) throw err("Can't open session database.\nconnection_arg: \"%s\"", connection_arg.c_str());
    	pqxx::nontransaction N(C); // inicia uma transação com o banco de dados /* Create a nontransactional object. */
    
    	std::string sql = u::sprintf("SELECT value FROM body WHERE head_id=%s and name =%s;",
    		N.quote(sid).c_str(), N.quote(key).c_str());
    	pqxx::result R { N.exec(sql) };
    	if(R.size() < 1) return false; // a chave não existe no banco de dados
    	if(R.size() > 1) {
    		throw err("Exists more than one key for this session. Numbers of rows returned from database: %d\n"
    		"Sql to get key: \"%s\"", R.size(), sql.c_str());
    	}
    
    	////////////////////////////////////////////////////////////////////////////////
		// insere o resultado dentro do map & fecha conexão do BD
		////////////////////////////////////////////////////////////////////////////////
		var[key].val = R[0][0].is_null() ? "" : R[0][0].as<std::string>();
		var[key].val_db = var[key].val; // guarda o antigo valor do banco de dados para a função save();
		var[key].insert = false; // seta a operação de save() para ser update nesta chave
		C.disconnect(); // fecha a conexão
		return true;
	} catch (pqxx::sql_error const &e) {
		C.disconnect(); throw err("SQL error: %s\nQuery was: \"%s\"", e.what(), e.query().c_str());
	} catch(std::exception const& e) { C.disconnect(); throw err(e.what()); }
 } catch(std::exception const& e) { throw err(e.what()); }
}

std::string 
w::session_postgres_t::create_sid()
{ try {
	std::string nsec = ""; // nano seconds
	bool swap  = false;
	// aumentar o nível de aleatóriedade - funciona para Unix (Linux, *BSD, Mac OS X)
	#if defined(unix) || defined(__unix) || defined(__unix__) || (defined (__APPLE__) && defined (__MACH__))
	#ifdef _SC_MONOTONIC_CLOCK
	struct timespec ts;
	if (sysconf (_SC_MONOTONIC_CLOCK) > 0) {
		if (clock_gettime (CLOCK_MONOTONIC, &ts) == 0) {
	   		nsec = std::to_string(ts.tv_nsec);
	   		swap = ((ts.tv_nsec%2) == 0) ? false : true;
		}
	}
	#endif
	#endif// for windows and olther system

	srand(time(nullptr));
	int al = rand();
	std::string current = std::to_string(time(nullptr));

	// insere outra camada de aleatoriedade na criação do sid
	std::string sid = "";
	int _pid = getpid();
	bool spid = ((_pid%2) == 0) ? false : true;
	if(swap && spid) {
		sid = current + nsec + std::to_string(al) + std::to_string(_pid) + std::to_string(getppid());
	} else if(swap && !spid) {
		sid = current + std::to_string(al) + nsec + std::to_string(getppid()) + std::to_string(_pid);
	} else if(!swap && spid) {
		sid = current + std::to_string(al) + nsec + std::to_string(_pid) + std::to_string(getppid());
	} else {
		sid = current + nsec + std::to_string(al) + std::to_string(getppid()) + std::to_string(_pid);
	}
	
	return sid;
 } catch(std::exception const& e) { throw err(e.what()); }
}

bool 
w::session_postgres_t::check_sid(const std::string& sid, const bool throw_exception)
{ try {
	if(sid.empty()) {
		if(throw_exception) {
			throw err("SESSION::sid cannot be an empty string.");
		} else {
			 return false;
		}
	}
	bool b = u::isdigit(sid, throw_exception);
	return b;
 } catch(std::exception const& e) { throw err("%s\nsid: \'%s\'.", e.what(), sid.c_str()); }
}

////////////////////////////////////////////////////////////////////////////////
// variável global da sessão
////////////////////////////////////////////////////////////////////////////////
//static Session _session;
w::session_postgres_t w::session;

////////////////////////////////////////////////////////////////////////////////
// Interface
////////////////////////////////////////////////////////////////////////////////









