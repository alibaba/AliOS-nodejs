// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-707580.js");
	}
}
var thrower = { [Symbol.toPrimitive] : function() { throw "I was called!" } };
var heap_number = 4.2;
var smi_number = 23;

assertThrows(() => heap_number.hasOwnProperty(thrower));
assertThrows(() => smi_number.hasOwnProperty(thrower));
