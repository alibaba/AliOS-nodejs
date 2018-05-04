// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/inlined-array-pop-getter2.js");
	}
}
var pop = Array.prototype.pop;

function foo(a) {
  a.length;
  return pop.call(a);
}

var a = new Array(4);
var o = {}
o.__defineGetter__(0, function() { return 1; });

assertEquals(undefined, foo(a));
assertEquals(undefined, foo(a));
%OptimizeFunctionOnNextCall(foo);
assertEquals(undefined, foo(a));
Array.prototype.__proto__ = o;
assertEquals(1, foo(a));
