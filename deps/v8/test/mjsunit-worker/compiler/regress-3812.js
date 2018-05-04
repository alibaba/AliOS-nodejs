// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/compiler/regress-3812.js");
	}
}
var stdlib = this;
var buffer = new ArrayBuffer(64 * 1024);
var foreign = {}

var foo = (function Module(stdlib, foreign, heap) {
  "use asm";
  function foo(i) {
    var x = i ? (i&1) : true;
    if (x) return x;
    return false;
  }
  return {foo:foo};
})(stdlib, foreign, buffer).foo;

assertEquals(1, foo(1));
