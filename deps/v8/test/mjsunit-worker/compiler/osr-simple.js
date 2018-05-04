// Copyright 2010 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --use-osr

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/osr-simple.js");
	}
}
function f() {
  var sum = 0;
  for (var i = 0; i < 1000; i++) {
    var x = i + 2;
    var y = x + 5;
    var z = y + 3;
    sum += z;
    if (i == 11) %OptimizeOsr();
  }
  return sum;
}


for (var i = 0; i < 2; i++) {
  assertEquals(509500, f());
}
