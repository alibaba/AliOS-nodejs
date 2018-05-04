// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This used to trigger a segfault because of NULL being accessed.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-crbug-465671-null.js");
	}
}
function f() {
  var a = [10];
  try {
    f();
  } catch(e) {
    a.map((v) => v + 1);
  }
}
f();
