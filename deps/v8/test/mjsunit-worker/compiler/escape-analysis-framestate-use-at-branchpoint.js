// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --turbo-escape

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/escape-analysis-framestate-use-at-branchpoint.js");
	}
}
function foo() {
  var o = {x:0};
  for (var i = 0; o.x < 1;) {
    o.x++;
    i+= 1;
  }
 function bar() {i};
 return o.x;
}
foo();
foo();
%OptimizeFunctionOnNextCall(foo);
foo();
