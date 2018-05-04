// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-4577.js");
	}
}
function f(...arguments) {
  return Array.isArray(arguments);
}
assertTrue(f());

function g({arguments}) {
  return arguments === 42;
}
assertTrue(g({arguments: 42}));

function foo() {
  let arguments = 2;
  return arguments;
}
assertEquals(2, foo());

assertThrows(function(x = arguments, arguments) {}, ReferenceError);
