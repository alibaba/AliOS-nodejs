// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc --always-opt

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-633585.js");
	}
}
function f() { this.x = this.x.x; }
gc();
f.prototype.x = { x:1 }
new f();
new f();

function g() {
  function h() {};
  h.prototype = { set x(value) { } };
  new f();
}
g();
