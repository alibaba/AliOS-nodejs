// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//string-oom-array-join.js");
	}
}
var a = "a";
for (var i = 0; i < 23; i++) a += a;
var b = [];
for (var i = 0; i < (1<<5); i++) b.push(a);

function join() {
  b.join();
}

assertThrows(join, RangeError);
