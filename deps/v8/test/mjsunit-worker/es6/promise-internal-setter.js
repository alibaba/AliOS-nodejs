// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/promise-internal-setter.js");
	}
}
'use strict';

Object.defineProperties(Object.prototype, {
  promise: {set: assertUnreachable},
  reject: {set: assertUnreachable},
  resolve: {set: assertUnreachable},
});

class P extends Promise {}

P.all([Promise.resolve('ok')]);
P.race([Promise.resolve('ok')]);
