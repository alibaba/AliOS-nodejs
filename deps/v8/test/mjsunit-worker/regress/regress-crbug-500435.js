// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-500435.js");
	}
}
function bar(a) {
  delete a[1];
}

function foo(a) {
  var d;
  for (d in a) {
    assertFalse(d === undefined);
    bar(a);
  }
}

foo([1,2]);
foo([2,3]);
%OptimizeFunctionOnNextCall(foo);
foo([1,2]);
