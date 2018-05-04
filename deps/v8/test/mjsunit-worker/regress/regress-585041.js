// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-585041.js");
	}
}
function f(arr, i) {
  arr[i] = 50;
}

function boom(dummy) {
  var arr = new Array(10);
  f(arr, 10);
  if (dummy) {
    f(arr, -2147483648);
  }
}

boom(false);
%OptimizeFunctionOnNextCall(boom);
boom(false);
