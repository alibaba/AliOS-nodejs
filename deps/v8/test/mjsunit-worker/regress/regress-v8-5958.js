// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-v8-5958.js");
	}
}
class A {}
class B {}

A.__proto__ = new Proxy(A.__proto__, {
  get: function (target, property, receiver) {
    if (property === Symbol.hasInstance) throw new B;
  }
});

assertThrows(() => (new A) instanceof A, B);
