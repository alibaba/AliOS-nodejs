// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-723132.js");
	}
}
function outer() {
  function* generator() {
    let arrow = () => {
      assertSame(expectedReceiver, this);
      assertEquals(42, arguments[0]);
    };
    arrow();
  }
  generator.call(this, 42).next();
}
let expectedReceiver = {};
outer.call(expectedReceiver);
