// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-664469.js");
	}
}
function f(a, i) {
  a[i] = "object";
}

f("make it generic", 0);

// Nearly kMaxRegularHeapObjectSize's worth of doubles.
var kLength = 500000 / 8;
var kValue = 0.1;
var a = new Array(kLength);
for (var i = 0; i < kLength; i++) {
  a[i] = kValue;
}
f(a, 0);
for (var i = 1; i < kLength; i++) {
  assertEquals(kValue, a[i]);
}
