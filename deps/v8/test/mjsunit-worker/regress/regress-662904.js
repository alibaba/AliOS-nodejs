// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-662904.js");
	}
}
function foo(a) {
  var sum = 0;
  var a = [0, "a"];
  for (var i in a) {
    sum += a[i];
  }
  return sum;
}

assertEquals("0a", foo());
assertEquals("0a", foo());
%OptimizeFunctionOnNextCall(foo);
assertEquals("0a", foo());
