// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --es-staging

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-682242.js");
	}
}
class BaseClass {
  method() {
    return 1;
  }
}
class SubClass extends BaseClass {
  method(...args) {
    return super.method(...args);
  }
}
var a = new SubClass();
a.method();
