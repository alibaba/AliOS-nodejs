// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-500824.js");
	}
}
function get_thrower() {
  "use strict";
  return Object.getOwnPropertyDescriptor(arguments, "callee").get;
}

var f = (function(v) {
  "use asm";
  function fun() {
    switch (v) {}
  }
  return {
    fun: fun
  };
})(get_thrower()).fun;

%OptimizeFunctionOnNextCall(f);
f();
