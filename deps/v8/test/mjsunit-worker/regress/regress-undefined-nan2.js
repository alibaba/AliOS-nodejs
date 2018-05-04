// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-undefined-nan2.js");
	}
}
function foo(a, i) {
  var o = [0.5,,1];
  a[i] = o[i];
}
var a1 = [0.1,0.1];
foo(a1, 0);
foo(a1, 1);
assertEquals(undefined, a1[1]);
