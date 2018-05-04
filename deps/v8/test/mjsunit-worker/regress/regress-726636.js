// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-726636.js");
	}
}
Object.defineProperty(Promise, Symbol.species, { value: 0 });
var p = new Promise(function() {});
try {
  p.then();
  assertUnreachable();
} catch(e) {
  assertTrue(e instanceof TypeError);
}
