// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --turbo-filter=test --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4266.js");
	}
}
function test() {
  try {
    [].foo();
  } catch (e) {
    return e.message;
  }
}

assertEquals("[].foo is not a function", test());
%OptimizeFunctionOnNextCall(test);
assertEquals("[].foo is not a function", test());
