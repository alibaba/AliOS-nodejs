// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-445859.js");
	}
}
var foo = (function Module(global, env, buffer) {
  "use asm";
  var i8 = new global.Int8Array(buffer);
  function foo() { i8[0] += 4294967295; }
  return { foo: foo };
})(this, {}, new ArrayBuffer(64 * 1024)).foo;
foo();
