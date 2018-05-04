// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5772.js");
	}
}
(function sloppyPackedArguments() {
  function f(a) {
    for (var i = 0; i < 2; i++) {
      a[i] = 0;
    }
  }
  var boom;
  function g() {
    var a = arguments;
    f(a);
    boom = a[5];
    assertEquals(undefined, boom);
  }

  f([]);
  g(1);
})();

(function strictPackedArguments() {
  "use strict";
  function f(a) {
    for (var i = 0; i < 2; i++) {
      a[i] = 0;
    }
  }
  var boom;
  function g() {
    var a = arguments;
    f(a);
    boom = a[5];
    assertEquals(undefined, boom);
  }

  f([]);
  g(1);
})();
