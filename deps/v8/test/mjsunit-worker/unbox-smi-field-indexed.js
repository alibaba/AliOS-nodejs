// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//unbox-smi-field-indexed.js");
	}
}
function Foo(x) {
  this.x = x;
}

var f = new Foo(1);
var g = new Foo(2);

function add(a, b) {
  var name = "x";
  return a[name] + b[name];
}

assertEquals(3, add(f, g));
assertEquals(3, add(g, f));
%OptimizeFunctionOnNextCall(add);
assertEquals(3, add(f, g));
assertEquals(3, add(g, f));
