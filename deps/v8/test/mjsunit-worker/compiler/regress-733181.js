// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-733181.js");
	}
}
function l(s) {
  return ("xxxxxxxxxxxxxxxxxxxxxxx" + s).toLowerCase();
}

l("abcd");
l("abcd");
%OptimizeFunctionOnNextCall(l);
l("abcd");

function u(s) {
  return ("xxxxxxxxxxxxxxxxxxxxxxx" + s).toUpperCase();
}

u("abcd");
u("abcd");
%OptimizeFunctionOnNextCall(u);
u("abcd");
