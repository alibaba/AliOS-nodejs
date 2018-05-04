// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --no-stress-fullcodegen

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-618657.js");
	}
}
function* foo() { yield 42 }
function* goo() { yield 42 }
var f = foo();
var g = goo();
assertEquals(42, f.next().value);
assertEquals(42, g.next().value);
assertEquals(true, f.next().done);
assertEquals(true, g.next().done);
