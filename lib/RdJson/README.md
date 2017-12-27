# RdJson

A library to parse and access JSON data

## Usage

Examples of usage

```
// Example usage for RdJson library
// Rob Dobson 2017

#include "RdJson.h"

SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

void setup() {

    Serial.begin(115200);
    delay(5000);
    Serial.println("Example usage of RdJson library");

    // Test JSON
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

    // A JSON string that is just an array
    String jsonStr2 = "[0,1,2,{\"a\":1},4]";

    // Entire array
    String arrayJson2 = RdJson::getString("", "", isValid, objType, objSize, jsonStr2.c_str());
    Serial.printlnf("Array %s valid %d, type %s, size %d", arrayJson2.c_str(), isValid, RdJson::getObjTypeStr(objType), objSize);

    // Element of array - that is an object
    String arrayJson3 = RdJson::getString("[3]", "", isValid, objType, objSize, jsonStr2.c_str());
    Serial.printlnf("Array %s valid %d, type %s, size %d", arrayJson3.c_str(), isValid, RdJson::getObjTypeStr(objType), objSize);
}

void loop() {
}

```

See the [examples](examples) folder for more details.

## Documentation

Three basic methods are provided to access elements of a JSON string:

getString()
getDouble()
getLong()

getString() can be used on any kind of element - the parser simply isolates the string in the JSON and returns it.

The method setJsonStr() can be used to set a member variable to avoid passing in the base string repeatedly.  Each method is available in static form and, whether or not the static form is used, the JSON is re-parsed for each call to getString, getDouble or getLong. This may be inefficient in some cases but often memory is a more valuable resource than processor cycles.

A very much simplified form of XPATH is available to access members:
[] can be used to isolate an element of an array
/ can be used to specify sub-objects

## LICENSE
Based on JSMN https://github.com/zserge/jsmn
Modified from https://github.com/pkourany/JSMNSpark
Portions Copyright 2017 Rob Dobson
Licensed under the MIT license
