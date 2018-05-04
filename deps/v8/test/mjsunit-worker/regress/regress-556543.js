// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-556543.js");
	}
}
function f() {
  for (var __v_2 = 0; __v_2 < __v_5; ++__v_2) {
    for (var __v_5 = 0; __v_3 < 1; ++__v_8) {
      if (true || 0 > -6) continue;
      for (var __v_3 = 0; __v_3 < 1; ++__v_3) {
      }
    }
  }
}
%OptimizeFunctionOnNextCall(f);
f();
