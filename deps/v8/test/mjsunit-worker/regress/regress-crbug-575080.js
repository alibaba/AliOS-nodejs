// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --es-staging

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-575080.js");
	}
}
class A extends Function {
  constructor(...args) {
    super(...args);
    this.a = 42;
    this.d = 4.2;
    this.o = 0;
  }
}
var obj = new A("'use strict';");
obj.o = 0.1;
