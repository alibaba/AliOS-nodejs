// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --turbo-escape

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-658185.js");
	}
}
var t = 0;
function foo() {
  var o = {x:1};
  var p = {y:2.5, x:0};
  o = [];
  for (var i = 0; i < 2; ++i) {
    t = o.x;
    o = p;
  }
}
foo();
foo();
%OptimizeFunctionOnNextCall(foo);
foo();
