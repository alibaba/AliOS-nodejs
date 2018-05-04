// Copyright 2016 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/es6/array-species-neg-zero.js");
	}
}
/**
 * 9.4.2.3 ArraySpeciesCreate(originalArray, length)
 *
 * 1. Assert: length is an integer Number ≥ 0.
 * 2. If length is −0, let length be +0.
 * [...]
 */

var x = [];
var deleteCount;

x.constructor = function() {};
x.constructor[Symbol.species] = function(param) {
  deleteCount = param;
};

x.splice(0, -0);

assertEquals(0, deleteCount);
