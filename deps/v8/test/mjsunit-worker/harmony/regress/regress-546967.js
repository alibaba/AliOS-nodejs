// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Flags: --harmony-do-expressions --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/harmony/regress/regress-546967.js");
	}
}
function func1() {
  for (var i = 0; i < 64; ++i) func2();
}

%OptimizeFunctionOnNextCall(func1);
func1();

function func2() {
  var v = do {};
}
