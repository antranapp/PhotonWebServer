/*
Copyright (c) 2017 Ethan Davis

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

// "use strict"; Let's not use this in production sites...

var ajax = function(definedOptions, cb) {
	try {
		// Validate inputs
		if (!definedOptions.hasOwnProperty("url")) throw "Missing required option: url";

		var options = {
			"method": (definedOptions.method || "GET"),
			"url": definedOptions.url,
			"data": (definedOptions.data || null),
			"headers": (definedOptions.headers || {})
		};

		var request = new(XMLHttpRequest || ActiveXObject)("Microsoft.XMLHTTP");
		request.open(options.method, options.url, true);

		// Set request headers
		var headerNames = Object.keys(options.headers);
		for (var i = 0; i < headerNames.length; i++) {
			request.setRequestHeader(headerNames[i], options.headers[headerNames[i]]);
		}

		request.onreadystatechange = function() {
			if (request.readyState === 4) {
				if (request.status === 0) {
					cb("Error sending request. Status 0.", null, request);
				}
				else {
					cb(null, request.responseText, request);
				}
			}
		}
		request.send((options.data ? options.data.toString() : null));
	}
	catch (err) {
		cb(err, null, null);
	}
};
