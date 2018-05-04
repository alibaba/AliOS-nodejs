// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --turbo-escape --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-5598.js");
	}
}
function fn(a) {
  var [b] = a;
  return b;
}

fn('a');
fn('a');
%OptimizeFunctionOnNextCall(fn);

fn('a');
