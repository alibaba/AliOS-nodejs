// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-4759.js");
	}
}
function iterable(done) {
  return {
    [Symbol.iterator]: function() {
      return {
        next: function() {
          if (done) return { done: true };
          done = true;
          return { value: 42, done: false };
        }
      }
    }
  }
}

var [...result] = iterable(true);
assertEquals([], result);

var [...result] = iterable(false);
assertEquals([42], result);
