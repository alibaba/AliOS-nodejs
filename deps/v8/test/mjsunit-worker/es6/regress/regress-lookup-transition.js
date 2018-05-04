// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --expose-gc

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-lookup-transition.js");
	}
}
var proxy = new Proxy({}, { getOwnPropertyDescriptor:function() {
  gc();
}});

function f() { this.x = 23; }
f.prototype = proxy;
new f();
new f();
