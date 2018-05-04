// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --stack-size=100 --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-527364.js");
	}
}
function module() {
  "use asm";
  var abs = Math.abs;
  function f() {
    return +abs();
  }
  return { f:f };
}

function run_close_to_stack_limit(f) {
  try {
    run_close_to_stack_limit(f);
    f();
  } catch(e) {
  }
}

var boom = module().f;
%OptimizeFunctionOnNextCall(boom)
run_close_to_stack_limit(boom);
