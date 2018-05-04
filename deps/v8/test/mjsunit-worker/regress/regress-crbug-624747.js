"use strict"
// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --es-staging

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-624747.js");
	}
}
"use strict";

function bar() {
  try {
    unref;
  } catch (e) {
    return (1 instanceof TypeError) && unref();  // Call in tail position!
  }
}

function foo() {
  return bar();  // Call in tail position!
}

%OptimizeFunctionOnNextCall(foo);
foo();
