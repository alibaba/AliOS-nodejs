// Copyright 2015 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

if (!isworker()) {
	for (var i = 0; i < ThreadWorkerCount; i++) {
	var worker = new ThreadWorker("test/mjsunit-worker/mjsunit.js","test/mjsunit-worker/regress/regress-447561.js");
	}
}
__proto__ = /foo/gi;
assertThrows(function() { source });
assertThrows(function() { global });
assertThrows(function() { ignoreCase });
assertThrows(function() { multiline });
assertEquals(0, lastIndex);
