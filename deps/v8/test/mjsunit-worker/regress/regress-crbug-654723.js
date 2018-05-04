// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-654723.js");
	}
}
var k = "0101010101010101" + "01010101";

function foo(s) {
  return k + s;
}

foo("a");
foo("a");
%OptimizeFunctionOnNextCall(foo);
var x = foo("");
