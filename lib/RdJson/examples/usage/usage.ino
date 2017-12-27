// Example usage for RdJson library
// Rob Dobson 2017

#include "RdJson.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

SerialLogHandler logHandler(LOG_LEVEL_TRACE);

void setup() {

    Serial.begin(115200);
    delay(5000);
    Serial.println("Example usage of RdJson library");

    // A JSON string that is an object
    String jsonStr = "{\"maxSpeed\":12.2, \"message\":\"This is a test\", \"a\":[0,1,2,3], \"b\":{\"c\":42} }";

    // Double
    Serial.printlnf("Max speed %0.3f", RdJson::getDouble("maxSpeed", 0.0, jsonStr.c_str()));

    // String
    String messageStr = RdJson::getString("message", "", jsonStr.c_str());
    Serial.printlnf("Message %s", messageStr.c_str());

    // Array - by extracting string from JSON
    bool isValid = false;
    jsmnrtype_t objType = JSMNR_UNDEFINED;
    int objSize = 0;
    String arrayJson = RdJson::getString("a", "", isValid, objType, objSize, jsonStr.c_str());
    Serial.printlnf("Array %s valid %d, type %s, size %d", arrayJson.c_str(), isValid, RdJson::getObjTypeStr(objType), objSize);

    // Array elements - using a simplified form of XPATH - to get element 2 of the object a use a[2]
    long arrayElem = RdJson::getLong("a[2]", 0, jsonStr);
    Serial.printlnf("Array element [2] = %ld", arrayElem);

    // Extracting an element within a sub-object - use / separator like XPATH - also not that string will get any kind of object
    String subObjStr = RdJson::getString("b/c", "", jsonStr.c_str());
    Serial.printlnf("Sub-object string %s", subObjStr.c_str());

    // A JSON string that is an array
    String jsonStr2 = "[0,1,2,{\"a\":1},4]";

    // Entire array
    String arrayJson2 = RdJson::getString("", "", isValid, objType, objSize, jsonStr2.c_str());
    Serial.printlnf("Array %s valid %d, type %s, size %d", arrayJson2.c_str(), isValid, RdJson::getObjTypeStr(objType), objSize);

    // Element of array - that is an object
    String objJson = RdJson::getString("[3]", "", isValid, objType, objSize, jsonStr2.c_str());
    Serial.printlnf("Object %s valid %d, type %s, size %d", objJson.c_str(), isValid, RdJson::getObjTypeStr(objType), objSize);

    // Get element information
    int startPos = 0, strLen = 0;
    isValid = RdJson::getElement("[3]", startPos, strLen, objType, objSize, jsonStr2.c_str());
    Serial.printlnf("Element info valid %d, type %s, size %d, startPos %d, strLen %d", isValid, RdJson::getObjTypeStr(objType), objSize, startPos, strLen);
}

void loop() {
}
