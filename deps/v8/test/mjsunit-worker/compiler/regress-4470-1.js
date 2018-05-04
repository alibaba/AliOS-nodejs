// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-4470-1.js");
	}
}
function Foo() {}
Foo.prototype.x = 0;
function foo(f) {
  f.x = 1;
}
foo(new Foo);
foo(new Foo);
%OptimizeFunctionOnNextCall(foo);
foo(new Foo);
assertEquals(Foo.prototype.x, 0);
