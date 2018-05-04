// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-386034.js");
	}
}
function f(x) {
  var v = x;
  for (i = 0; i < 1; i++) {
    v.apply(this, arguments);
  }
}

function g() {}

f(g);
f(g);
%OptimizeFunctionOnNextCall(f);
assertThrows(function() { f('----'); }, TypeError);
