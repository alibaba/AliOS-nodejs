// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-674469.js");
	}
}
global = -1073741824;
global = 2;
function foo() {
  global = "a";
  global = global;
  var o = global;
  while (o < 2) {
  }
}
foo();
