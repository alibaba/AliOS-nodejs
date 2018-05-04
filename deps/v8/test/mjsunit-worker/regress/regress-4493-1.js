// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4493-1.js");
	}
}
function baz(x, f) { return x.length; };

function bar(x, y) {
  if (y) {}
  baz(x, function() { return x; });
};

function foo(x) { bar(x, ''); }
%OptimizeFunctionOnNextCall(foo);
foo(['a']);
