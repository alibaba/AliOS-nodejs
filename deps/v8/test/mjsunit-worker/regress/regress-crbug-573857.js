// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc --verify-heap

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-573857.js");
	}
}
function f() {}
f = f.bind();
f.x = f.name;
f.__defineGetter__('name', function() { return f.x; });
function g() {}
g.prototype = f;
gc();
