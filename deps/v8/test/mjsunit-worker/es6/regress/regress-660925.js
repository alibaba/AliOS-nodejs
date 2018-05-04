// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regress/regress-660925.js");
	}
}
let array = new Array(0xFFFFFFFF);
let it = array.keys();
assertEquals({ value: 0, done: false }, it.next());

it = array.entries();
assertEquals({ value: [0, undefined], done: false }, it.next());

it = array[Symbol.iterator]();
assertEquals({ value: undefined, done: false }, it.next());
