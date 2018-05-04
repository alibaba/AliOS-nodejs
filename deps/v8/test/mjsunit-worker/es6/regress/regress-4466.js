"use strict"
// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-4466.js");
	}
}
"use strict";

class Parent {
  parentMethod(x, y) {
    assertEquals(42, x);
    assertEquals('hello world', y);
  }
}

class Child extends Parent {
  method(x) {
    let outer = (y) => {
      let inner = () => {
        super.parentMethod(x, y);
      };
      inner();
    };
    outer('hello world');
  }
}

new Child().method(42);
