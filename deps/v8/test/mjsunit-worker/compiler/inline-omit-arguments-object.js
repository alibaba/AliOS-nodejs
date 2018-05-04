// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/inline-omit-arguments-object.js");
	}
}
function bar(s, t, u, v) { return arguments; }
function foo(s, t) {
  var args = bar(s);
  return args.length == 1 && args[0] == 11;
}

%OptimizeFunctionOnNextCall(foo);
assertEquals(true, foo(11));
