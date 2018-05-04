// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es8/async-await-no-constructor.js");
	}
}
'use strict';

var resolved = Promise.resolve();
var count = 0;

Object.defineProperty(Promise.prototype, 'constructor',
                      { get() { count++; return Promise; } })

async function foo() {
  await resolved;
  return resolved;
}

async function bar() {
  throw 1;
}

foo();
bar();
%RunMicrotasks();
assertEquals(2, count);
