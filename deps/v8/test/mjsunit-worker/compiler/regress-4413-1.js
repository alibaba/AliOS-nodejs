// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-4413-1.js");
	}
}
var foo = (function(stdlib) {
  "use asm";
  var bar = stdlib.Symbol;
  function foo() { return bar("lala"); }
  return foo;
})(this);

%OptimizeFunctionOnNextCall(foo);
foo();
