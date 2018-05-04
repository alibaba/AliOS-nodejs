// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-603463.js");
	}
}
function load(a, i) {
  return a[i];
}

function f() {
  return load(new Proxy({}, {}), undefined);
}

f();
f();
load([11, 22, 33], 0);
f();
