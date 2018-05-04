// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/classes-lazy-parsing.js");
	}
}
'use strict';

class Base {
  m() {
    return 42;
  }
}

class Derived extends Base {
  m() {
    return super.m();
  }
  evalM() {
    return eval('super.m()');
  }
}

assertEquals(42, new Derived().m());
assertEquals(42, new Derived().evalM());


class LazyDerived extends Base {
  constructor() {
    eval('super()');
  }
}
assertInstanceof(new LazyDerived(), LazyDerived);
assertInstanceof(new LazyDerived(), Base);
