// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-683581.js");
	}
}
var v = 0;
function foo() {
  for (var i = 0; i < 70000; i++) {
    v += i;
  }
  eval();
}
foo()
assertEquals(2449965000, v);
