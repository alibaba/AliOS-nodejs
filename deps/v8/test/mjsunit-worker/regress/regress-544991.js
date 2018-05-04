// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-544991.js");
	}
}
'use strict';

var typedArray = new Int8Array(1);
var saved;
var called;
class TypedArraySubclass extends Int8Array {
  constructor(x) {
    super(x);
    called = true;
    saved = x;
  }
}
typedArray.constructor = TypedArraySubclass
typedArray.map(function(){});

assertTrue(called);
assertEquals(saved, 1);
