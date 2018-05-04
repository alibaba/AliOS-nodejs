// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-548580.js");
	}
}
function store(v) {
  var re = /(?=[d#.])/;
  re.a = v;
  return re;
}

var re1 = store(undefined);
var re2 = store(42);

assertEquals(re1.a, undefined);
assertEquals(re2.a, 42);
