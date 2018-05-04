// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-361608.js");
	}
}
function f() {};
int_array = [1];

function foo() {
  var x;
  for (var i = -1; i < 0; i++) {
    x = int_array[i + 1];
    f(function() { x = i; });
  }
}

foo();
%OptimizeFunctionOnNextCall(foo);
foo();
