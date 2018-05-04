// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-642409.js");
	}
}
class SuperClass {
}

class SubClass extends SuperClass {
  constructor() {
    super();
    this.doSomething();
  }
  doSomething() {
  }
}

new SubClass();
new SubClass();
%OptimizeFunctionOnNextCall(SubClass);
new SubClass();
