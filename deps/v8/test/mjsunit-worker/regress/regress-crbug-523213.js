// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-523213.js");
	}
}
var v1 = [];
var v2 = [];
v1.__proto__ = v2;

function f(){
  var a = [];
  for(var i=0; i<2; i++){
    a.push([]);
    a = v2;
  }
}

f();
%OptimizeFunctionOnNextCall(f);
f();
