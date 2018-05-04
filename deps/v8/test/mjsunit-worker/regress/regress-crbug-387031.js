// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-387031.js");
	}
}
a = [1];
b = [];
a.__defineGetter__(0, function () {
  b.length = 0xffffffff;
});
c = a.concat(b);
for (var i = 0; i < 20; i++) {
  assertEquals(undefined, (c[i]));
}
