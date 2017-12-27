// RdJson
// Rob Dobson 2017

// Many of the methods here support a dataPath parameter. This uses a syntax like a much simplified XPath:
// [0] returns the 0th element of an array
// / is a separator of nodes

#pragma once
#include "Particle.h"
#include "jsmnParticleR.h"

// Define this to enable reformatting of JSON
//#define RDJSON_RECREATE_JSON 1

class RdJson {
public:
    // Get location of element in JSON string
    static bool getElement(const char* dataPath,
        int& startPos, int& strLen,
        jsmnrtype_t& objType, int& objSize,
        const char* pSourceStr)
    {
        // Check for null
        if (!pSourceStr)
            return false;

        // Parse json into tokens
        int numTokens = 0;
        jsmnrtok_t* pTokens = parseJson(pSourceStr, numTokens);
        if (pTokens == NULL) {
            return false;
        }

        // Find token
        int startTokenIdx, endTokenIdx;
        bool isValid = getTokenByDataPath(pSourceStr, dataPath,
            pTokens, numTokens, startTokenIdx, endTokenIdx);
        if (!isValid) {
            delete[] pTokens;
            return false;
        }

        // Extract information on element
        objType = pTokens[startTokenIdx].type;
        objSize = pTokens[startTokenIdx].size;
        startPos = pTokens[startTokenIdx].start;
        strLen = pTokens[startTokenIdx].end-startPos;
        delete [] pTokens;
        return true;
    }

    // Get a string from the JSON
    static String getString(const char* dataPath,
        const char* defaultValue, bool& isValid,
        jsmnrtype_t& objType, int& objSize,
        const char* pSourceStr)
    {
        // Find the element in the JSON
        int startPos = 0, strLen = 0;
        isValid = getElement(dataPath, startPos, strLen, objType, objSize, pSourceStr);
        if (!isValid)
            return defaultValue;

        // Extract string
        String outStr;
        char* pStr = safeStringDup(pSourceStr + startPos, strLen,
                    !(objType == JSMNR_STRING || objType == JSMNR_PRIMITIVE));
        outStr = pStr;
        delete[] pStr;

        // If the underlying object is a string or primitive value return size as length of string
        if (objType == JSMNR_STRING || objType == JSMNR_PRIMITIVE)
            objSize = outStr.length();
        return outStr;
    }

    // Alternate form of getString with fewer parameters
    static String getString(const char* dataPath, const char* defaultValue,
        const char* pSourceStr, bool& isValid)
    {
        jsmnrtype_t objType = JSMNR_UNDEFINED;
        int objSize = 0;
        return getString(dataPath, defaultValue, isValid, objType, objSize,
            pSourceStr);
    }

    // Alternate form of getString with fewer parameters
    static String getString(const char* dataPath, const char* defaultValue,
        const char* pSourceStr)
    {
        bool isValid = false;
        jsmnrtype_t objType = JSMNR_UNDEFINED;
        int objSize = 0;
        return getString(dataPath, defaultValue, isValid, objType, objSize,
            pSourceStr);
    }

    static double getDouble(const char* dataPath,
        double defaultValue, bool& isValid,
        const char* pSourceStr)
    {
        // Find the element in the JSON
        int startPos = 0, strLen = 0;
        jsmnrtype_t objType = JSMNR_UNDEFINED;
        int objSize = 0;
        isValid = getElement(dataPath, startPos, strLen, objType, objSize, pSourceStr);
        if (!isValid)
            return defaultValue;
        return strtod(pSourceStr + startPos, NULL);
    }

    static double getDouble(const char* dataPath, double defaultValue,
        const char* pSourceStr)
    {
        bool isValid = false;
        return getDouble(dataPath, defaultValue, isValid, pSourceStr);
    }

    static long getLong(const char* dataPath,
        long defaultValue, bool& isValid,
        const char* pSourceStr)
    {
        // Find the element in the JSON
        int startPos = 0, strLen = 0;
        jsmnrtype_t objType = JSMNR_UNDEFINED;
        int objSize = 0;
        isValid = getElement(dataPath, startPos, strLen, objType, objSize, pSourceStr);
        if (!isValid)
            return defaultValue;
        return strtol(pSourceStr + startPos, NULL, 10);
    }

    static long getLong(const char* dataPath, long defaultValue, const char* pSourceStr)
    {
        bool isValid = false;
        return getLong(dataPath, defaultValue, isValid, pSourceStr);
    }

    static const char* getObjTypeStr(jsmnrtype_t type)
    {
        switch (type) {
        case JSMNR_PRIMITIVE:
            return "PRIMITIVE";
        case JSMNR_STRING:
            return "STRING";
        case JSMNR_OBJECT:
            return "OBJECT";
        case JSMNR_ARRAY:
            return "ARRAY";
        case JSMNR_UNDEFINED:
            return "UNDEFINED";
        }
        return "UNKNOWN";
    }

    static const jsmnrtype_t getType(int& arrayLen, const char* pSourceStr)
    {
        // Check for null
        if (!pSourceStr)
            return JSMNR_UNDEFINED;

        // Parse json into tokens
        int numTokens = 0;
        jsmnrtok_t* pTokens = parseJson(pSourceStr, numTokens);
        if (pTokens == NULL)
            return JSMNR_UNDEFINED;

        // Get the type of the first token
        arrayLen = pTokens->size;
        return pTokens->type;
    }

public:
    static jsmnrtok_t* parseJson(const char* jsonStr, int& numTokens,
        int maxTokens = 10000)
    {
        // Check for null source string
        if (jsonStr == NULL) {
            Log.error("RdJson: Source JSON is NULL");
            return NULL;
        }

        // Find how many tokens in the string
        JSMNR_parser parser;
        JSMNR_init(&parser);
        int tokenCountRslt = JSMNR_parse(&parser, jsonStr, strlen(jsonStr),
            NULL, maxTokens);
        if (tokenCountRslt < 0) {
            Log.trace("RdJson: parseJson result %d maxTokens %d jsonLen %d", tokenCountRslt, maxTokens, strlen(jsonStr));
            JSMNR_logLongStr("RdJson: jsonStr", jsonStr);
            return NULL;
        }

        // Allocate space for tokens
        if (tokenCountRslt > maxTokens)
            tokenCountRslt = maxTokens;
        jsmnrtok_t* pTokens = new jsmnrtok_t[tokenCountRslt];

        // Parse again
        JSMNR_init(&parser);
        tokenCountRslt = JSMNR_parse(&parser, jsonStr, strlen(jsonStr),
            pTokens, tokenCountRslt);
        if (tokenCountRslt < 0) {
            Log.info("parseJson result: %d", tokenCountRslt);
            Log.trace("jsonStr %s numTok %d maxTok %d", jsonStr, numTokens, maxTokens);
            delete[] pTokens;
            return NULL;
        }
        numTokens = tokenCountRslt;
        return pTokens;
    }

    static void escapeString(String& strToEsc)
    {
        // Replace characters which are invalid in JSON
        strToEsc.replace("\\", "\\\\");
        strToEsc.replace("\"", "\\\"");
        strToEsc.replace("\n", "\\n");
    }

    static void unescapeString(String& strToUnEsc)
    {
        // Replace characters which are invalid in JSON
        strToUnEsc.replace("\\\"", "\"");
        strToUnEsc.replace("\\\\", "\\");
        strToUnEsc.replace("\\n", "\n");
    }

private:
    static bool getTokenByDataPath(const char* jsonStr, const char* dataPath,
        jsmnrtok_t* pTokens, int numTokens,
        int& startTokenIdx, int& endTokenIdx)
    {
        // Get required token
        int keyIdx = findKeyInJson(jsonStr, pTokens, numTokens,
            dataPath, endTokenIdx);
        if (keyIdx < 0) {
            //Log.info("getTokenByDataPath not found %s", dataPath);
            return false;
        }

        // Copy out token value
        startTokenIdx = keyIdx;
        return true;
    }

    static int findObjectEnd(const char* jsonOriginal, jsmnrtok_t tokens[],
        unsigned int numTokens, int curTokenIdx,
        int count, bool atObjectKey = true)
    {
        // Log.trace("findObjectEnd idx %d, count %d, start %s", curTokenIdx, count,
        //                 jsonOriginal + tokens[curTokenIdx].start);
        // Primitives have a size of 0 but we still need to skip over them ...
        unsigned int tokIdx = curTokenIdx;
        if (count == 0) {
            jsmnrtok_t* pTok = tokens + tokIdx;
            if (pTok->type == JSMNR_ARRAY)
                return tokIdx + 1;
        }
        for (int objIdx = 0; objIdx < count; objIdx++) {
            jsmnrtok_t* pTok = tokens + tokIdx;
            if (pTok->type == JSMNR_PRIMITIVE) {
                // Log.trace("findObjectEnd PRIMITIVE");
                tokIdx += 1;
            }
            else if (pTok->type == JSMNR_STRING) {
                // Log.trace("findObjectEnd STRING");
                if (atObjectKey) {
                    tokIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx + 1, 1, false);
                }
                else {
                    tokIdx += 1;
                }
            }
            else if (pTok->type == JSMNR_OBJECT) {
                // Log.trace("findObjectEnd OBJECT");
                tokIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx + 1, pTok->size, true);
            }
            else if (pTok->type == JSMNR_ARRAY) {
                // Log.trace("findObjectEnd ARRAY");
                tokIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx + 1, pTok->size, false);
            }
            else {
                Log.trace("findObjectEnd UNKNOWN!!!!!!! %d", pTok->type);
                tokIdx += 1;
            }
            if (tokIdx >= numTokens) {
                break;
            }
        }
        // Log.trace("findObjectEnd returning %d, start %s, end %s", tokIdx,
        //                 jsonOriginal + tokens[tokIdx].start,
        //             jsonOriginal + tokens[tokIdx].end);
        return tokIdx;
    }

    static int findKeyInJson(const char* jsonOriginal, jsmnrtok_t tokens[],
        unsigned int numTokens, const char* dataPath,
        int& endTokenIdx,
        jsmnrtype_t keyType = JSMNR_UNDEFINED)
    {
        const int MAX_SRCH_KEY_LEN = 100;
        char srchKey[MAX_SRCH_KEY_LEN + 1];
        const char* pDataPathPos = dataPath;
        // Note that the root object has index 0
        int curTokenIdx = 0;
        int maxTokenIdx = numTokens - 1;
        bool atNodeLevel = false;

        // Go through the parts of the path to find the whole
        while (pDataPathPos <= dataPath + strlen(dataPath)) {
            // Get the next part of the path
            const char* slashPos = strstr(pDataPathPos, "/");
            if (slashPos == NULL) {
                safeStringCopy(srchKey, pDataPathPos, MAX_SRCH_KEY_LEN);
                pDataPathPos += strlen(pDataPathPos) + 1;
                atNodeLevel = true;
            }
            else if (slashPos - pDataPathPos >= MAX_SRCH_KEY_LEN) {
                safeStringCopy(srchKey, pDataPathPos, MAX_SRCH_KEY_LEN);
                pDataPathPos = slashPos + 1;
            }
            else {
                safeStringCopy(srchKey, pDataPathPos, slashPos - pDataPathPos);
                pDataPathPos = slashPos + 1;
            }
            // Log.trace("SlashPos %ld, %ld, srchKey <%s>", slashPos, slashPos-pDataPathPos, srchKey);

            // See if search key contains an array reference
            bool arrayElementReqd = false;
            int reqdArrayIdx = 0;
            char* sqBracketPos = strstr(srchKey, "[");
            if (sqBracketPos != NULL) {
                // Extract array index
                long arrayIdx = strtol(sqBracketPos + 1, NULL, 10);
                if (arrayIdx >= 0) {
                    arrayElementReqd = true;
                    reqdArrayIdx = (int)arrayIdx;
                }
                // Truncate the string at the square bracket
                *sqBracketPos = 0;
            }

            // Log.trace("findKeyInJson srchKey %s arrayIdx %ld", srchKey, reqdArrayIdx);

            // Iterate over tokens to find key of the right type
            // If we are already looking at the node level then search for requested type
            // Otherwise search for and object that will contain the next level key
            jsmnrtype_t keyTypeToFind = atNodeLevel ? keyType : JSMNR_STRING;
            for (int tokIdx = curTokenIdx; tokIdx <= maxTokenIdx;) {
                // See if the key matches - this can either be a string match on an object key or
                // just an array element match (with an empty key)
                jsmnrtok_t* pTok = tokens + tokIdx;
                bool keyMatchFound = false;
                // Log.trace("Token type %d", pTok->type);
                if ((pTok->type == JSMNR_STRING) && ((int)strlen(srchKey) == pTok->end - pTok->start) && (strncmp(jsonOriginal + pTok->start, srchKey, pTok->end - pTok->start) == 0)) {
                    keyMatchFound = true;
                    tokIdx += 1;
                }
                else if (((pTok->type == JSMNR_ARRAY) || (pTok->type == JSMNR_OBJECT)) && ((int)strlen(srchKey) == 0)) {
                    keyMatchFound = true;
                }

                if (keyMatchFound) {
                    // We have found the matching key so now for the contents ...

                    // Check if we were looking for an array element
                    if (arrayElementReqd) {
                        if (tokens[tokIdx].type == JSMNR_ARRAY) {
                            int newTokIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx + 1, reqdArrayIdx, false);
//                            Log.trace("TokIdxArray inIdx %d, reqdArrayIdx %d, outTokIdx %d", tokIdx, reqdArrayIdx, newTokIdx);
                            tokIdx = newTokIdx;
                        }
                        else {
                            // This isn't an array element
                            return -1;
                        }
                    }

                    // atNodeLevel indicates that we are now at the level of the JSON tree that the user requested
                    // - so we should be extracting the value referenced now
                    if (atNodeLevel) {
                        // Log.trace("findObjectEnd we have got it %d", tokIdx);
                        if ((keyTypeToFind == JSMNR_UNDEFINED) || (tokens[tokIdx].type == keyTypeToFind)) {
                            endTokenIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx, 1, false);
                            //int testTokenIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx+1, 1);
                            //Log.trace("TokIdxDiff max %d, test %d, diff %d", endTokenIdx, testTokenIdx, testTokenIdx-endTokenIdx);
                            return tokIdx;
                        }
                        return -1;
                    }
                    else {
                        // Check for an object
                        // Log.trace("findObjectEnd inside");
                        if (tokens[tokIdx].type == JSMNR_OBJECT) {
                            // Continue next level of search in this object
                            maxTokenIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx, 1);
                            //int testTokenIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx+1, 1);
                            //Log.trace("TokIdxDiff2 max %d, test %d, diff %d", maxTokenIdx, testTokenIdx, testTokenIdx- maxTokenIdx);
                            curTokenIdx = tokIdx + 1;
                            break;
                        }
                        else {
                            // Found a key in the path but it didn't point to an object so we can't continue
                            return -1;
                        }
                    }
                }
                else if (pTok->type == JSMNR_STRING) {
                    // We're at a key string but it isn't the one we want so skip its contents
                    tokIdx = findObjectEnd(jsonOriginal, tokens, numTokens, tokIdx, 1);
                }
                else if (pTok->type == JSMNR_OBJECT) {
                    // Move to the first key of the object
                    tokIdx++;
                }
                else if (pTok->type == JSMNR_ARRAY) {
                    // Root level array which doesn't match the dataPath
                    return -1;
                }
                else {
                    // Shouldn't really get here as all keys are strings
                    tokIdx++;
                }
            }
        }
        return -1;
    }

public:
    static size_t safeStringLen(const char* pSrc,
        bool skipJSONWhitespace = false, size_t maxx = LONG_MAX)
    {
        if (maxx == 0)
            return 0;
        const char* pS = pSrc;
        int stringLen = 0;
        bool insideDoubleQuotes = false;
        bool insideSingleQuotes = false;
        size_t charsTakenFromSrc = 0;
        while (*pS) {
            char ch = *pS++;
            charsTakenFromSrc++;
            if ((ch == '\'') && !insideDoubleQuotes)
                insideSingleQuotes = !insideSingleQuotes;
            else if ((ch == '\"') && !insideSingleQuotes)
                insideDoubleQuotes = !insideDoubleQuotes;
            else if (!insideDoubleQuotes && !insideSingleQuotes && skipJSONWhitespace && isspace(ch))
                continue;
            stringLen++;
            if (maxx != LONG_MAX && charsTakenFromSrc >= maxx)
                return stringLen;
        }
        return stringLen;
    }

    static void safeStringCopy(char* pDest, const char* pSrc,
        size_t maxx, bool skipJSONWhitespace = false)
    {
        char* pD = pDest;
        const char* pS = pSrc;
        size_t srcStrlen = strlen(pS);
        size_t stringLen = 0;
        bool insideDoubleQuotes = false;
        bool insideSingleQuotes = false;
        for (size_t i = 0; i < srcStrlen + 1; i++) {
            char ch = *pS++;
            if ((ch == '\'') && !insideDoubleQuotes)
                insideSingleQuotes = !insideSingleQuotes;
            else if ((ch == '\"') && !insideSingleQuotes)
                insideDoubleQuotes = !insideDoubleQuotes;
            else if (!insideDoubleQuotes && !insideSingleQuotes && skipJSONWhitespace && (isspace(ch) || (ch > 0 && ch < 32) || (ch >= 127))) {
                continue;
            }
            *pD++ = ch;
            stringLen++;
            if (stringLen >= maxx) {
                *pD = 0;
                break;
            }
        }
    }

    static char* safeStringDup(const char* pSrc, size_t maxx,
        bool skipJSONWhitespace = false)
    {
        size_t toAlloc = safeStringLen(pSrc, skipJSONWhitespace, maxx);
        char* pDest = new char[toAlloc + 1];
        pDest[toAlloc] = 0;
        if (toAlloc == 0)
            return pDest;
        char* pD = pDest;
        const char* pS = pSrc;
        size_t srcStrlen = strlen(pS);
        size_t stringLen = 0;
        bool insideDoubleQuotes = false;
        bool insideSingleQuotes = false;
        for (size_t i = 0; i < srcStrlen + 1; i++) {
            char ch = *pS++;
            if ((ch == '\'') && !insideDoubleQuotes)
                insideSingleQuotes = !insideSingleQuotes;
            else if ((ch == '\"') && !insideSingleQuotes)
                insideDoubleQuotes = !insideDoubleQuotes;
            else if (!insideDoubleQuotes && !insideSingleQuotes && skipJSONWhitespace && isspace(ch))
                continue;
            *pD++ = ch;
            stringLen++;
            if (stringLen >= maxx || stringLen >= toAlloc) {
                *pD = 0;
                break;
            }
        }
        //        Log.trace("safeStringDup <%s> %d %d %d %d %d %d %d <%s>", pSrc, maxx, toAlloc, srcStrlen, stringLen, insideDoubleQuotes, insideSingleQuotes, skipJSONWhitespace, pDest);
        return pDest;
    }

#ifdef RDJSON_RECREATE_JSON
    static int recreateJson(const char* js, jsmnrtok_t* t,
        size_t count, int indent, String& outStr);
    static bool doPrint(const char* jsonStr);
#endif // RDJSON_RECREATE_JSON
};
