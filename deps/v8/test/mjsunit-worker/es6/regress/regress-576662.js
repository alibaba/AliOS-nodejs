// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// https://code.google.com/p/chromium/issues/detail?id=576662 (simplified)

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-576662.js");
	}
}
Realm.create();
this.__proto__ = new Proxy({},{});
assertThrows(() => Realm.eval(1, "Realm.global(0).bla = 1"));
