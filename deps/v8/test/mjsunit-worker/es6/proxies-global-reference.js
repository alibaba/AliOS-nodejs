// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/proxies-global-reference.js");
	}
}
var failing_proxy = new Proxy({}, new Proxy({}, {
  get() { throw "No trap should fire" }}));

assertThrows(() => Object.setPrototypeOf(Object.prototype, failing_proxy), TypeError);
assertThrows(()=>a, ReferenceError);
