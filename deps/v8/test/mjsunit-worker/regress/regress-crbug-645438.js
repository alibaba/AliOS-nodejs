// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-645438.js");
	}
}
function n(x,y){
  y = (y-(0x80000000|0)|0);
  return (x/y)|0;
};
var x = -0x80000000;
var y = 0x7fffffff;
n(x,y);
n(x,y);
%OptimizeFunctionOnNextCall(n);
assertEquals(x, n(x,y));
