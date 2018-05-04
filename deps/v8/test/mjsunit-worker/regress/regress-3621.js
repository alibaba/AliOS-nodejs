// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-3621.js");
	}
}
var a = [];
var endIndex = 0xffff;
a[endIndex] = 3;
Object.defineProperty(a, 0, { get: function() { this[1] = 2; return 1; } });
assertEquals('123', a.join(''));
delete a[1];  // reset the array
assertEquals('1,2,', a.join().slice(0, 4));
