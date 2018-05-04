// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/destructuring-assignment-lazy.js");
	}
}
function f() {
  var a, b;
  [ a, b ] = [1, 2];
  assertEquals(1, a);
  assertEquals(2, b);
}

f();
