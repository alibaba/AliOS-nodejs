// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/regexp-tolength.js");
	}
}
'use strict';

let regexp = /x/g;

regexp.lastIndex = -1;

assertTrue(regexp.test("axb"));
assertEquals(2, regexp.lastIndex);

regexp.lastIndex = -1;

assertEquals("x", regexp.exec("axb")[0]);
assertEquals(2, regexp.lastIndex);
