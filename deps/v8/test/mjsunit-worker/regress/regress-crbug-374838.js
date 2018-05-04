// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-374838.js");
	}
}
function foo() {
  var a = [0];
  result = 0;
  for (var i = 0; i < 4; i++) {
    result += a.length;
    a.shift();
  }
  return result;
}

assertEquals(1, foo());
assertEquals(1, foo());
%OptimizeFunctionOnNextCall(foo);
assertEquals(1, foo());
