// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4495.js");
	}
}
function foo(a, s) { a[s] = 35; }
var x = { bilbo: 3 };
var y = { frodo: 3, bilbo: 'hi' };
foo(x, "bilbo");
foo(x, "bilbo");
// Without the fix for 4495, this will crash on ia32:
foo(y, "bilbo");
