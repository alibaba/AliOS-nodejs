// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-615776.js");
	}
}
Object.defineProperty(Int32Array.prototype.__proto__, 'length', {
  get: function() { throw new Error('Custom length property'); }
});

var a = Math.random();

// This tests MathRandomRaw.
var v0 = new Set();
var v1 = new Object();
v0.add(v1);
