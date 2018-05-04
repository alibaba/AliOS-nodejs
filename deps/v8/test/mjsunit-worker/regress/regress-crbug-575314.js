// Copyright 2014 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Flags: --allow-natives-syntax

// https://code.google.com/p/chromium/issues/detail?id=575314

// Overwriting the constructor of a Promise with something that doesn't have
// @@species shouldn't result in a rejection, even if that constructor
// is somewhat bogus.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-crbug-575314.js");
	}
}
var test = new Promise(function(){});
test.constructor = function(){};
Promise.resolve(test).catch(e => %AbortJS(e + " FAILED!"));
