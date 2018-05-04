// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-489151.js");
	}
}
this.__proto__ = Array.prototype;
Object.freeze(this);
function __f_0() {
  for (var __v_0 = 0; __v_0 < 10; __v_0++) {
    this.length = 1;
  }
}
 __f_0();
