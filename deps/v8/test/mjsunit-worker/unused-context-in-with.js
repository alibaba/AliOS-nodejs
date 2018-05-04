// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker//unused-context-in-with.js");
	}
}
var x = 1;
function foo(object) {
  with(object) {
    x;
  }
  return 100;
}

assertEquals(100,foo("str"));
