#include <string>
#include <vector>
#include <regex>

#include "helpers.h"
#include "errors.h"
#include "api/events/events.h"
#include "server/router.h"
#include "api/custom/custom.h"

#include "lib/json/json.hpp"

#ifdef _WIN32
#include <windows.h>
#include <thread>
#endif

using namespace std;
using json = nlohmann::json;

#define NEU_CMETHOD_REGEX "(^custom\\.)(.*)"

namespace custom {
vector<string> getMethods() {
    auto methodMap = router::getMethodMap();
    vector<string> customMethods = {};
    for(const auto &[methodName, _]: methodMap) {
        if(methodName == "custom.getMethods") {
            continue;
        }

        if(regex_match(methodName, regex(NEU_CMETHOD_REGEX))) {
            string cMethodName = regex_replace(methodName, regex(NEU_CMETHOD_REGEX), "$2");
            customMethods.push_back(cMethodName);
        }
    }
    return customMethods;
}

namespace controllers {

json getMethods(const json &input) {
    json output;
    output["returnValue"] = custom::getMethods();
    output["success"] = true;
    return output;
}

HHOOK keyboardHook = NULL;
std::thread keyboardThread;
DWORD keyboardThreadId = 0;

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        if (wParam == WM_KEYUP) {
            int vkCode = kb->vkCode;
            std::string key = std::to_string(vkCode);
            json data;
            data["vkCode"] = key;
            events::dispatch("keyboardhook_keydown", data);
        }
    }
    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

json startKeyboardHook(const json &input) {   

    keyboardThread = std::thread([]() {
        keyboardThreadId = GetCurrentThreadId();

        HINSTANCE hInstance = GetModuleHandle(NULL);
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, 0);

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0)) {
            if (msg.message == WM_QUIT) break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (keyboardHook != NULL) {
            UnhookWindowsHookEx(keyboardHook);
            keyboardHook = NULL;
        }
    });
    
    keyboardThread.detach();

    json data;
    data["success"] = true;
    events::dispatch("keyboardhook_started", data);
    return data;
}


json stopKeyboardHook(const json &input) {
    json data;

    if (keyboardThreadId != 0) {
        PostThreadMessage(keyboardThreadId, WM_QUIT, 0, 0);  // Tell thread to exit loop
        keyboardThreadId = 0;
        data["success"] = true;
    } else {
        data["success"] = false;
        data["error"] = "Keyboard hook thread not running.";
    }

    events::dispatch("keyboardhook_stopped", data);
    return data;
}

} // namespace controllers
} // namespace custom
