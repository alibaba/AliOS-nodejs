// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/spread-call-super-property.js");
	}
}
(function testCallSuperPropertyStrict() {
  "use strict";
  class BaseClass {
    method(...args) { return [this].concat(args); }
  }
  class SubClass extends BaseClass {
    method(...args) { return super.method(...args); }
  }

  var c = new SubClass();
  assertEquals([c, 1, 2, 3, 4, 5], c.method(1, 2, 3, 4, 5));
})();


(function testCallSuperPropertySloppy() {
  class BaseClass {
    method(...args) { return [this].concat(args); }
  }
  class SubClass extends BaseClass {
    method(...args) { return super.method(...args); }
  }

  var c = new SubClass();
  assertEquals([c, 1, 2, 3, 4, 5], c.method(1, 2, 3, 4, 5));
})();
