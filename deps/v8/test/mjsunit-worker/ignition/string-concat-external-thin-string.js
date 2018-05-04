// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --expose-externalize-string

// Calculate string so that it isn't internalized.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/ignition/string-concat-external-thin-string.js");
	}
}
var string = ((a,b) => { return a + b; })('foo', 'bar');

// Now externalize it.
externalizeString(string, false);

// Then internalize it by using it as a keyed property name
// to turn it a thin string.
var obj = {};
obj[string];

// Perform a string concatination.
assertEquals('"' + string + '"', '"foobar"');
