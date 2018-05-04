// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-635798.js");
	}
}
function foo() {
  var x = [];
  var y = [];
  x.__proto__ = y;
  for (var i = 0; i < 10000; ++i) {
    y[i] = 1;
  }
}
foo();
