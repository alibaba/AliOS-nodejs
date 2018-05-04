// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//string-oom-concat.js");
	}
}
function concat() {
  var a = " ";
  for (var i = 0; i < 100; i++) {
    a += a;
  }
  return a;
}

assertThrows(concat, RangeError);
