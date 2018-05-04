// Copyright 2013 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-382143.js");
	}
}
function A() {
  Object.defineProperty(this, "x", { set: function () {}, get: function () {}});
  this.a = function () { return 1; }
}

function B() {
  A.apply( this );
  this.a = function () { return 2; }
}

var b = new B();
assertTrue(Object.getOwnPropertyDescriptor(b, "a").enumerable);
