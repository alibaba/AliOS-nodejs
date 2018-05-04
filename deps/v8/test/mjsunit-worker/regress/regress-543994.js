// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --always-opt --gc-interval=163 --stress-compaction

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-543994.js");
	}
}
try { a = f();
} catch(e) {
}
var i = 0;
function f() {
   try {
     f();
   } catch(e) {
     i++;
     [];
   }
}
f();
