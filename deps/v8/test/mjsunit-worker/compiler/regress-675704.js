// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-675704.js");
	}
}
function foo(a) {
  this.a = a;
  // Note that any call would do, it doesn't need to be %MaxSmi()
  this.x = this.a + %MaxSmi();
}

function g(x) {
  new foo(2);

  if (x) {
    for (var i = 0.1; i < 1.1; i++) {
      new foo(i);
    }
  }
}

g(false);
g(false);
%OptimizeFunctionOnNextCall(g);
g(true);
