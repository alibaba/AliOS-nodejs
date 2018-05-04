"use strict"
// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --turbo-filter=f

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-variable-liveness-let.js");
	}
}
"use strict";

function f() {
  %DeoptimizeNow();
  let x = 23;
}

%OptimizeFunctionOnNextCall(f);
f();
