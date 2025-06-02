#ifndef NEU_CUSTOM_H
#define NEU_CUSTOM_H

#include <string>
#include <vector>

#include "lib/json/json.hpp"

using json = nlohmann::json;
using namespace std;

namespace custom {

vector<string> getMethods();

namespace controllers {

json getMethods(const json &input);

json startKeyboardHook(const json &input);
json stopKeyboardHook(const json &input);

} // namespace controllers

} // namespace custom

#endif // #define NEU_CUSTOM_H
