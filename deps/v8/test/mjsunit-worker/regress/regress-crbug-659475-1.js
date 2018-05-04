// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-659475-1.js");
	}
}
var n;

function Ctor() {
  n = new Set();
}

function Check() {
  n.xyz = 0x826852f4;
}

Ctor();
Ctor();
%OptimizeFunctionOnNextCall(Ctor);
Ctor();

Check();
Check();
%OptimizeFunctionOnNextCall(Check);
Check();

Ctor();
Check();

parseInt('AAAAAAAA');
