// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-dictionary-to-fast-arguments.js");
	}
}
function f(a, b) {
  for (var i = 10000; i > 0; i--) {
    arguments[i] = 0;
  }
}

f(1.5, 2.5);
