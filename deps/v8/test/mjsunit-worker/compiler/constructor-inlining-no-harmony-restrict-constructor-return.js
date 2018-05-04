// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax --no-harmony-restrict-constructor-return --max-deopt-count 200

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/constructor-inlining-no-harmony-restrict-constructor-return.js");
	}
}
this.FLAG_harmony_restrict_constructor_return = false;
try {
  load('mjsunit/compiler/constructor-inlining.js');
} catch(e) {
  load('test/mjsunit/compiler/constructor-inlining.js');
}
