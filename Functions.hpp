#ifndef FUNCTIONS_HPP
#define FUNCTIONS_HPP

# include <string>
# include <map>
# include <unordered_map>
# include <vector>
# include <utility>
# include <functional>
using API_PATH = std::string;
using API_METHOD = std::string;
using API_PARAM = std::string;
extern std::unordered_map<API_PATH, std::unordered_map<API_METHOD, std::vector<API_PARAM>>> MainMap;
extern std::unordered_map<API_PATH, std::unordered_map<API_METHOD, std::vector<API_PARAM>>> AnswerMap;
extern std::map<std::pair<API_PATH, API_METHOD>, std::function<void
                                                               (std::map<std::string, std::string>&, std::vector<std::string>&)>> FunctionMap;

void NewDifferentiate(std::string& Path, std::map<std::string, std::string>& Params, std::string& Method,
                      std::string& ReturnMessage);

void newAuthorizeUser(std::map<std::string, std::string>& Params,
                      std::vector<std::string>& ReturnMessage);

void newAddUser(std::map<std::string, std::string>& Params,
                std::vector<std::string>& ReturnMessage);

void newDeleteUser(std::map<std::string, std::string>& Params,
                   std::vector<std::string>& ReturnMessage);


void CreateReturn(std::vector<std::string>& Answers, std::string& Path,
                  std::string& Method, std::string& ReturnMessage);

void FalseReturn(std::string& ReturnMessage, signed int State);

void newPendingRequest(std::map<std::string, std::string>& Params,
                       std::vector<std::string>& ReturnMessage);


#endif // FUNCTIONS_H
