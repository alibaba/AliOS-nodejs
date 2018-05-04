// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --opt --allow-natives-syntax --no-always-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/ignition/throw-super-not-called.js");
	}
}
class A {
  constructor() { }
}
class B extends A {
  constructor(call_super) {
    if (call_super) {
      super();
    }
  }
}

test = new B(1);
test = new B(1);
%OptimizeFunctionOnNextCall(B);
test = new B(1);
assertOptimized(B);
// Check that hole checks are handled correctly in optimized code.
assertThrowsEquals(() => {new B(0)}, ReferenceError());
assertOptimized(B);
