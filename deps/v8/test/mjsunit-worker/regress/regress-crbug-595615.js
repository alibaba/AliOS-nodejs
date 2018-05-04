"use strict"
// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-595615.js");
	}
}
"use strict";

function f(o) {
  return o.x();
}
try { f({ x: 1 }); } catch(e) {}
try { f({ x: 1 }); } catch(e) {}
%OptimizeFunctionOnNextCall(f);
try { f({ x: 1 }); } catch(e) {}
