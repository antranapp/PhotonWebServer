#include "Particle.h"
#include "LocalServer.h"
#include "RestAPIEndpoints.h"
#include "RdJson.h"
#include <functional>

LocalServer::LocalServer() {
    _token = "";
    _tokenTime = 0;
}

void LocalServer::setup() {
    using namespace std::placeholders;

    while (1) {
        if (WiFi.ready()) {
            break;
        }
        Particle.process();
    }

    // Add endpoint
    _restAPIEndpoints.addEndpoint("postLogin", RestAPIEndpointDef::ENDPOINT_CALLBACK, std::bind(&LocalServer::restAPI_PostLogin, this, _1, _2), "");
    _restAPIEndpoints.addEndpoint("postChangePassword", RestAPIEndpointDef::ENDPOINT_CALLBACK, std::bind(&LocalServer::restAPI_PostChangePassword, this, _1, _2), "");
    _restAPIEndpoints.addEndpoint("postSettings", RestAPIEndpointDef::ENDPOINT_CALLBACK, std::bind(&LocalServer::restAPI_PostSettings, this, _1, _2), "");
    _restAPIEndpoints.addEndpoint("getSettings", RestAPIEndpointDef::ENDPOINT_CALLBACK, std::bind(&LocalServer::restAPI_GetSettings, this, _1, _2), "");

    // Construct server
    _webServer = new RdWebServer();

    // Configure web server
    if (_webServer) {
        // Add resources to web server
        _webServer->addStaticResources(genResources, genResourcesCount);

        // Endpoints
        _webServer->addRestAPIEndpoints(&_restAPIEndpoints);

        // Start the web server
        _webServer->start(80);
    }
}

void LocalServer::service() {
    if (_webServer) {
        _webServer->service();
    }
}

void LocalServer::restAPI_PostLogin(RestAPIEndpointMsg& apiMsg, String& retStr) {
    String postStr = (const char *)apiMsg._pMsgContent;
    String passwordString = RdJson::getString("password", "", postStr.c_str());
    String localPassword = _getPassword();
    Serial.print("passwordString: ");Serial.println(passwordString);
    Serial.print("localPassword: ");Serial.println(localPassword);
    if (passwordString == localPassword) {
        // authorized
        _token = _generateToken();
        _tokenTime = millis();

        retStr = "{\"status\":\"ok\", \"token\":\"" + _token + "\"}";
    } else {
        retStr = "{\"status\":\"unauthorized\"}";
    }
}

void LocalServer::restAPI_PostChangePassword(RestAPIEndpointMsg& apiMsg, String& retStr) {
    if (!_isTokenExistedAndValid((char*)apiMsg._pMsgHeader)) {
        retStr = "{\"status\":\"unauthorized\"}";
        return;
    }

    String postStr = (const char *)apiMsg._pMsgContent;

    String oldPasswordString = RdJson::getString("oldPassword", "", postStr.c_str());
    String newPasswordString = RdJson::getString("newPassword", "", postStr.c_str());
    String localPassword = _getPassword();
    Serial.print("oldPasswordString: ");Serial.println(oldPasswordString);
    Serial.print("newPasswordString: ");Serial.println(newPasswordString);
    Serial.print("localPassword: ");Serial.println(localPassword);
    if (oldPasswordString == localPassword) {
        _savePassword(newPasswordString);
        _tokenTime = millis();

        retStr = "{\"status\":\"ok\"}";
    } else {
        retStr = "{\"status\":\"unauthorized\"}";
    }
}

void LocalServer::restAPI_PostSettings(RestAPIEndpointMsg& apiMsg, String& retStr) {
    Serial.println(apiMsg._pMsgHeader);
    if (!_isTokenExistedAndValid((char*)apiMsg._pMsgHeader)) {
        retStr = "{\"status\":\"unauthorized\"}";
        return;
    }

    _tokenTime = millis();
    retStr = "{\"status\":\"ok\"}";
}

// Get settings information via API
void LocalServer::restAPI_GetSettings(RestAPIEndpointMsg& apiMsg, String& retStr) {
    Log.trace("RestAPI GetSettings method %d contentLen %d", apiMsg._method, apiMsg._msgContentLen);
    if (!_isTokenExistedAndValid((char*)apiMsg._pMsgHeader)) {
        retStr = "{\"status\":\"unauthorized\"}";
        return;
    }

    _tokenTime = millis();
    String configStr = "{\"openTime\":\"7:15\", \"reminderTime\":\"20:15\"}";
    retStr = configStr;
}

String LocalServer::_generateToken() {
    String token = "";
    for (unsigned int i=0; i<32; i++) {
        token += String(random(0, 10));
    }

    return token;
}

bool LocalServer::_isTokenValid(String token) {
    return (token == _token) && !_isTokenExpired();
}

bool LocalServer::_isTokenExpired() {
    return millis() - _tokenTime > 1000 * 60 * 5; // 5 minutes
}

String LocalServer::_getPassword() {
    Password password;
    EEPROM.get(0, password);
    Serial.print("length: ");Serial.println(password.length);
    if (password.version != 0) {
        Serial.println("no password found");
        return "password"; // default password
    }

    password.value[password.length] = 0;

    String passwordString(password.value);

    Serial.print("found password: ");Serial.println(passwordString);

    return passwordString;
}

void LocalServer::_savePassword(String passwordString) {
    Password password;
    passwordString.getBytes((unsigned char *)password.value, sizeof(password.value));
    password.length = passwordString.length();
    password.version = 0;

    EEPROM.put(0, password);
}

String LocalServer::_getTokenFromHeader(char* headerString) {
    char* line = strtok(headerString, "\n");
    while (line != 0) {
        char* separator = strchr(line, ':');
        if (separator != 0) {
            *separator = 0;

            if (strcmp(line, "X-Token") == 0) {
                ++separator;
                return String(separator).trim();
            }
        }
        line = strtok(0, "\n");
    }

    return "";
}

bool LocalServer::_isTokenExistedAndValid(char* headerString) {
    String token = _getTokenFromHeader(headerString);
    return _isTokenValid(token);
}
