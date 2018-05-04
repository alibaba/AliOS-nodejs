// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6121.js");
	}
}
function foo(o) {
  try {
    for (var x in o) {}
    return false;
  } catch (e) {
    return true;
  }
}

var o = new Proxy({a:1},{
  getOwnPropertyDescriptor(target, property) { throw target; }
});

assertTrue(foo(o));
assertTrue(foo(o));
%OptimizeFunctionOnNextCall(foo);
assertTrue(foo(o));
