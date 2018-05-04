// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4376-3.js");
	}
}
function Foo() {}
var x = new Foo();
function foo() { return x instanceof Foo; }
assertTrue(foo());
Foo.prototype = 1;
assertThrows(foo, TypeError);
