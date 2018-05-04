"use strict"
// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --es-staging --no-stress-fullcodegen

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-639270.js");
	}
}
"use strict";

var g = (async () => { return JSON.stringify() });

g();
g();
%OptimizeFunctionOnNextCall(g);
g();
