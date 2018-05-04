// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-644633.js");
	}
}
var g = -1073741824;

function f() {
  var x = g*g*g*g*g*g*g;
  for (var i = g; i < 1; ) {
    i += i * x;
  }
}

f();
