#pragma once

#include "RdWebServer.h"
#include "GenResources.h"

class LocalServer {

    public:
        LocalServer();

        void setup();
        void service();

    private:
        RdWebServer* _webServer;
        RestAPIEndpoints _restAPIEndpoints;
        String _token;
        unsigned long _tokenTime;

        String _generateToken();
        bool _isTokenValid(String token);
        bool _isTokenExpired();
        String _getTokenFromHeader(char* headerString);
        bool _isTokenExistedAndValid(char* headerString);

        void restAPI_PostLogin(RestAPIEndpointMsg& apiMsg, String& retStr);
        void restAPI_PostChangePassword(RestAPIEndpointMsg& apiMsg, String& retStr);
        void restAPI_PostSettings(RestAPIEndpointMsg& apiMsg, String& retStr);
        void restAPI_GetSettings(RestAPIEndpointMsg& apiMsg, String& retStr);

        struct Password {
            uint8_t version;
            char value[11]; // null terminated 10 byte string
            unsigned int length;
        };

        String _getPassword();
        void _savePassword(String passwordString);
};
