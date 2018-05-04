// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5476.js");
	}
}
'use strict'

class LeakyPromise extends Promise {
  constructor(executor) {
    super((resolve, reject) => { resolve();});
    this.resolve = function() {assertEquals(this, undefined); };
    this.reject = function() {assertEquals(this, undefined); };
    executor(this.resolve, this.reject);
  }
}

const p1 = new LeakyPromise((r) => r());
const p2 = new LeakyPromise((_, r) => r());
