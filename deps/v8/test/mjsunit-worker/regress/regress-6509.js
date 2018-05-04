// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-6509.js");
	}
}
(function testSloppy() {
  var arrow = (sth = (function f() {
    {
      function f2() { }
    }
  })()) => 0;

  assertEquals(0, arrow());
})();

(function testStrict() {
  "use strict";
  var arrow = (sth = (function f() {
    {
      function f2() { }
    }
  })()) => 0;

  assertEquals(0, arrow());
})();
