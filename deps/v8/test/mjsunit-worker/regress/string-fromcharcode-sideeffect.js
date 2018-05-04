// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/string-fromcharcode-sideeffect.js");
	}
}
var counter = 0;
var a = { valueOf: function() { counter++; return 0x100; } };
assertEquals("A\u0100\u0100\u0100", String.fromCharCode(65, a, a, a));
assertEquals(3, counter);
