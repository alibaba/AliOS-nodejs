// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-551287.js");
	}
}
function f() { do { } while (true); }

function boom(x) {
  switch(x) {
    case 1:
    case f(): return;
  }
}

%OptimizeFunctionOnNextCall(boom)
boom(1);
