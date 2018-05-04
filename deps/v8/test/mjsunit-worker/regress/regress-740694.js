// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --harmony --allow-natives-syntax --stack-size=100

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-740694.js");
	}
}
function __f_0() {
  try {
    return __f_0();
  } catch(e) {
    return import('no-such-file');
  }
}

var done = false;
var error;
var promise = __f_0();
promise.then(assertUnreachable,
             err => { done = true; error = err });
%RunMicrotasks();
assertTrue(error.startsWith('Error reading'));
assertTrue(done);
