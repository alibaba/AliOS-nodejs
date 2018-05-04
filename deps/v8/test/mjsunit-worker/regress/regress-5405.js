// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-5405.js");
	}
}
let log = [];

(async function() {
  with ({get ['.promise']() { log.push('async') }}) {
    return 10;
  }
})();
%RunMicrotasks();

(function() {
  with ({get ['.new.target']() { log.push('new.target') }}) {
    return new.target;
  }
})();

(function() {
  with ({get ['this']() { log.push('this') }}) {
    return this;
  }
})();

assertArrayEquals([], log);
