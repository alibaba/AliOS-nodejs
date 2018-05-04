"use strict"
// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-2858.js");
	}
}
"use strict";

function f() {
    var y = 1;
    var q1;
    var q;
    var z = new Error();
    try {
        throw z;
    } catch (y) {
      assertTrue(z === y);
      q1 = function() { return y; }
      var y = 15;
      q = function() { return y; }
      assertSame(15, y);
    }
    assertSame(1, y);
    assertSame(15, q1());
    assertSame(15, q());
}

f();
