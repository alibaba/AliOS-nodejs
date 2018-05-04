// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax


if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/regress-generators-resume.js");
	}
}
function* foo() {
  for (let i = 0; i < 10; i++) {
    yield 1;
  }
  return 0;
}

g = foo();
%OptimizeFunctionOnNextCall(foo);
g.next();
g.next();
